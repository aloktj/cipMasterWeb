#pragma once

#include "models/Device.h"
#include <optional>
#include <vector>

class DeviceRepository
{
public:
    virtual ~DeviceRepository() = default;

    virtual bool create(const Device &device, std::string &error) = 0;
    virtual std::optional<Device> find(const std::string &name) const = 0;
    virtual std::vector<Device> list() const = 0;
    virtual bool update(const std::string &name, const Device &device, std::string &error) = 0;
    virtual bool remove(const std::string &name, std::string &error) = 0;
};

