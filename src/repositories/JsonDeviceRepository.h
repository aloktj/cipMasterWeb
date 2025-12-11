#pragma once

#include "InMemoryDeviceRepository.h"
#include <filesystem>

class JsonDeviceRepository : public InMemoryDeviceRepository
{
public:
    explicit JsonDeviceRepository(const std::string &path);

    bool create(const Device &device, std::string &error) override;
    bool update(const std::string &name, const Device &device, std::string &error) override;
    bool remove(const std::string &name, std::string &error) override;

private:
    void load();
    void persist();

    std::string path_;
};

