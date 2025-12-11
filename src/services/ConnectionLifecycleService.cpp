#include "ConnectionLifecycleService.h"

#include <EIPScanner/MessageRouter.h>
#include <EIPScanner/cip/EPath.h>
#include <EIPScanner/cip/connectionManager/NetworkConnectionParametersBuilder.h>
#include <random>
#include <limits>

namespace
{
uint16_t generateSerial()
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint16_t> dist(1, std::numeric_limits<uint16_t>::max());
    return dist(rng);
}

std::vector<uint8_t> buildConnectionPath(const ConnectionConfig &config)
{
    std::vector<uint8_t> path;
    auto appendAssembly = [&path](uint16_t instance) {
        eipScanner::cip::EPath assemblyPath(0x04, instance);
        auto packed = assemblyPath.packPaddedPath();
        path.insert(path.end(), packed.begin(), packed.end());
    };

    if (config.configAssembly.has_value())
    {
        appendAssembly(config.configAssembly->instance);
    }

    appendAssembly(config.outputAssembly.instance);
    appendAssembly(config.inputAssembly.instance);

    return path;
}
} // namespace

ConnectionLifecycleService::ConnectionLifecycleService()
{
    worker_ = std::thread([this]() { loop(); });
}

ConnectionLifecycleService::~ConnectionLifecycleService()
{
    running_ = false;
    if (worker_.joinable())
    {
        worker_.join();
    }
}

bool ConnectionLifecycleService::open(const Device &device, std::string &error)
{
    if (!device.connection.has_value())
    {
        error = "Device has no connection configuration";
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto &entry = connections_[device.name];
    entry.device = device;
    entry.status.deviceName = device.name;

    if (entry.status.connected || entry.status.opening)
    {
        return true;
    }

    entry.status.opening = true;
    entry.status.lastError.clear();

    try
    {
        auto timeout = std::chrono::milliseconds(device.timeoutMs);
        entry.session = std::make_shared<eipScanner::SessionInfo>(device.ipAddress, device.port, timeout);
        entry.manager = std::make_shared<eipScanner::ConnectionManager>(std::make_shared<eipScanner::MessageRouter>());

        eipScanner::cip::connectionManager::ConnectionParameters params;
        auto config = *device.connection;

        eipScanner::cip::connectionManager::NetworkConnectionParametersBuilder o2tBuilder(0, config.useLargeForwardOpen);
        o2tBuilder.setConnectionType(config.multicast
                                         ? eipScanner::cip::connectionManager::NetworkConnectionParametersBuilder::ConnectionType::MULTICAST
                                         : eipScanner::cip::connectionManager::NetworkConnectionParametersBuilder::ConnectionType::P2P)
            .setPriority(eipScanner::cip::connectionManager::NetworkConnectionParametersBuilder::Priority::HIGH_PRIORITY)
            .setType(eipScanner::cip::connectionManager::NetworkConnectionParametersBuilder::Type::FIXED)
            .setConnectionSize(config.outputAssembly.sizeBytes);

        eipScanner::cip::connectionManager::NetworkConnectionParametersBuilder t2oBuilder(0, config.useLargeForwardOpen);
        t2oBuilder.setConnectionType(eipScanner::cip::connectionManager::NetworkConnectionParametersBuilder::ConnectionType::MULTICAST)
            .setPriority(eipScanner::cip::connectionManager::NetworkConnectionParametersBuilder::Priority::SCHEDULED)
            .setType(eipScanner::cip::connectionManager::NetworkConnectionParametersBuilder::Type::FIXED)
            .setConnectionSize(config.inputAssembly.sizeBytes);

        params.priorityTimeTick = 0x07;
        params.timeoutTicks = 0x05;
        params.connectionSerialNumber = generateSerial();
        params.originatorVendorId = 0x0456;
        params.originatorSerialNumber = 0x00010001;
        params.o2tRPI = config.rpiUs;
        params.t2oRPI = config.rpiUs;
        params.o2tNetworkConnectionParams = o2tBuilder.build();
        params.t2oNetworkConnectionParams = t2oBuilder.build();
        params.transportTypeTrigger = 0xA3; // client, cyclic
        params.connectionPath = buildConnectionPath(config);
        params.connectionPathSize = static_cast<eipScanner::cip::CipUsint>(params.connectionPath.size() / 2);

        auto conn = entry.manager->forwardOpen(entry.session, params, config.useLargeForwardOpen);
        if (conn.expired())
        {
            markError(entry, "ForwardOpen failed");
            error = entry.status.lastError;
            return false;
        }

        auto sharedConn = conn.lock();
        entry.connection = sharedConn;
        sharedConn->setReceiveDataListener([this, name = device.name](auto, auto, const std::vector<uint8_t> &data) {
            std::lock_guard<std::mutex> guard(mutex_);
            auto it = connections_.find(name);
            if (it == connections_.end())
            {
                return;
            }
            updateStatus(it->second, [&, received = data.size()](ConnectionStatus &status) {
                status.packetsReceived++;
                status.lastSequence += received > 0 ? 1 : 0;
            });
        });

        sharedConn->setSendDataListener([this, name = device.name](std::vector<uint8_t> &buffer) {
            std::lock_guard<std::mutex> guard(mutex_);
            auto it = connections_.find(name);
            if (it == connections_.end())
            {
                return;
            }
            updateStatus(it->second, [](ConnectionStatus &status) { status.packetsSent++; });
            buffer.resize(it->second.device.connection->outputAssembly.sizeBytes, 0);
        });

        sharedConn->setCloseListener([this, name = device.name]() {
            std::lock_guard<std::mutex> guard(mutex_);
            auto it = connections_.find(name);
            if (it == connections_.end())
            {
                return;
            }
            it->second.status.connected = false;
            it->second.status.opening = false;
            it->second.status.lastError = "Connection closed by target";
        });

        updateStatus(entry, [](ConnectionStatus &status) {
            status.connected = true;
            status.opening = false;
            status.lastError.clear();
        });
        return true;
    }
    catch (const std::exception &ex)
    {
        markError(entry, ex.what());
        error = entry.status.lastError;
        return false;
    }
}

bool ConnectionLifecycleService::close(const std::string &deviceName, std::string &error)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = connections_.find(deviceName);
    if (it == connections_.end())
    {
        error = "No such connection";
        return false;
    }

    auto &entry = it->second;
    if (entry.manager && !entry.connection.expired())
    {
        entry.manager->forwardClose(entry.session, entry.connection);
    }

    entry.connection.reset();
    updateStatus(entry, [](ConnectionStatus &status) {
        status.connected = false;
        status.opening = false;
    });
    return true;
}

std::optional<ConnectionStatus> ConnectionLifecycleService::status(const std::string &deviceName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = connections_.find(deviceName);
    if (it == connections_.end())
    {
        return std::nullopt;
    }
    return it->second.status;
}

std::vector<ConnectionStatus> ConnectionLifecycleService::listStatuses()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ConnectionStatus> items;
    items.reserve(connections_.size());
    for (const auto &pair : connections_)
    {
        items.push_back(pair.second.status);
    }
    return items;
}

void ConnectionLifecycleService::loop()
{
    while (running_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::vector<std::shared_ptr<eipScanner::ConnectionManager>> managers;

        {
            std::lock_guard<std::mutex> guard(mutex_);
            managers.reserve(connections_.size());
            for (auto &pair : connections_)
            {
                managers.push_back(pair.second.manager);
                updateStatus(pair.second, [](ConnectionStatus &status) {
                    status.lastUpdate = std::chrono::system_clock::now();
                });
            }
        }

        for (auto &manager : managers)
        {
            if (manager)
            {
                manager->handleConnections(std::chrono::milliseconds(1));
            }
        }
    }
}

void ConnectionLifecycleService::markError(ConnectionEntry &entry, const std::string &message)
{
    entry.status.connected = false;
    entry.status.opening = false;
    entry.status.lastError = message;
    entry.status.lastUpdate = std::chrono::system_clock::now();
}

void ConnectionLifecycleService::updateStatus(ConnectionEntry &entry, const std::function<void(ConnectionStatus &)> &fn)
{
    fn(entry.status);
    entry.status.lastUpdate = std::chrono::system_clock::now();
}

