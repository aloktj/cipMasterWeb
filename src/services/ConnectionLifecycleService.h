#pragma once

#include "models/Device.h"

#include <EIPScanner/ConnectionManager.h>
#include <EIPScanner/SessionInfo.h>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <thread>

struct ConnectionStatus
{
    std::string deviceName;
    bool connected{false};
    bool opening{false};
    std::string lastError;
    uint64_t packetsSent{0};
    uint64_t packetsReceived{0};
    uint64_t lastSequence{0};
    std::chrono::system_clock::time_point lastUpdate;

    Json::Value toJson() const
    {
        Json::Value value;
        value["deviceName"] = deviceName;
        value["connected"] = connected;
        value["opening"] = opening;
        value["lastError"] = lastError;
        value["packetsSent"] = static_cast<Json::UInt64>(packetsSent);
        value["packetsReceived"] = static_cast<Json::UInt64>(packetsReceived);
        value["lastSequence"] = static_cast<Json::UInt64>(lastSequence);
        value["lastUpdateMs"] = static_cast<Json::UInt64>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                             lastUpdate.time_since_epoch())
                                                             .count());
        return value;
    }
};

class ConnectionLifecycleService
{
public:
    ConnectionLifecycleService();
    ~ConnectionLifecycleService();

    bool open(const Device &device, std::string &error);
    bool close(const std::string &deviceName, std::string &error);
    std::optional<ConnectionStatus> status(const std::string &deviceName);
    std::vector<ConnectionStatus> listStatuses();

private:
    struct ConnectionEntry
    {
        Device device;
        ConnectionStatus status;
        std::shared_ptr<eipScanner::SessionInfo> session;
        std::shared_ptr<eipScanner::ConnectionManager> manager;
        eipScanner::IOConnection::WPtr connection;
    };

    std::mutex mutex_;
    std::map<std::string, ConnectionEntry> connections_;
    bool running_{true};
    std::thread worker_;

    void loop();
    void markError(ConnectionEntry &entry, const std::string &message);
    void updateStatus(ConnectionEntry &entry, const std::function<void(ConnectionStatus &)> &fn);
};

