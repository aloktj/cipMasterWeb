#pragma once

#include "DeviceRepository.h"
#include <mutex>
#include <unordered_map>

class InMemoryDeviceRepository : public DeviceRepository
{
public:
    bool create(const Device &device, std::string &error) override;
    std::optional<Device> find(const std::string &name) const override;
    std::vector<Device> list() const override;
    bool update(const std::string &name, const Device &device, std::string &error) override;
    bool remove(const std::string &name, std::string &error) override;

protected:
    bool validateAndCheckName(const Device &device, const std::string &name, std::string &error) const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Device> devices_;
};

