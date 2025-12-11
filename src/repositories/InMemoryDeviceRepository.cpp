#include "InMemoryDeviceRepository.h"

#include <algorithm>

bool InMemoryDeviceRepository::validateAndCheckName(const Device &device, const std::string &name, std::string &error) const
{
    if (!device.isValid(error))
    {
        return false;
    }

    if (device.name != name && devices_.count(device.name) > 0)
    {
        error = "Device name already exists";
        return false;
    }

    return true;
}

bool InMemoryDeviceRepository::create(const Device &device, std::string &error)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!device.isValid(error))
    {
        return false;
    }

    if (devices_.count(device.name) > 0)
    {
        error = "Device name already exists";
        return false;
    }

    devices_[device.name] = device;
    return true;
}

std::optional<Device> InMemoryDeviceRepository::find(const std::string &name) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devices_.find(name);
    if (it != devices_.end())
    {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Device> InMemoryDeviceRepository::list() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Device> result;
    result.reserve(devices_.size());
    for (const auto &entry : devices_)
    {
        result.push_back(entry.second);
    }
    std::sort(result.begin(), result.end(), [](const Device &lhs, const Device &rhs) {
        return lhs.name < rhs.name;
    });
    return result;
}

bool InMemoryDeviceRepository::update(const std::string &name, const Device &device, std::string &error)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!validateAndCheckName(device, name, error))
    {
        return false;
    }

    auto it = devices_.find(name);
    if (it == devices_.end())
    {
        error = "Device not found";
        return false;
    }

    if (device.name != name)
    {
        devices_.erase(it);
    }

    devices_[device.name] = device;
    return true;
}

bool InMemoryDeviceRepository::remove(const std::string &name, std::string &error)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devices_.find(name);
    if (it == devices_.end())
    {
        error = "Device not found";
        return false;
    }
    devices_.erase(it);
    return true;
}
