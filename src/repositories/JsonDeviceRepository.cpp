#include "JsonDeviceRepository.h"

#include <fstream>
#include <json/json.h>
#include <stdexcept>

JsonDeviceRepository::JsonDeviceRepository(const std::string &path) : path_(path)
{
    load();
}

bool JsonDeviceRepository::create(const Device &device, std::string &error)
{
    if (!InMemoryDeviceRepository::create(device, error))
    {
        return false;
    }

    persist();
    return true;
}

bool JsonDeviceRepository::update(const std::string &name, const Device &device, std::string &error)
{
    if (!InMemoryDeviceRepository::update(name, device, error))
    {
        return false;
    }
    persist();
    return true;
}

bool JsonDeviceRepository::remove(const std::string &name, std::string &error)
{
    if (!InMemoryDeviceRepository::remove(name, error))
    {
        return false;
    }
    persist();
    return true;
}

void JsonDeviceRepository::load()
{
    std::ifstream file(path_, std::ios::in);
    if (!file.is_open())
    {
        return;
    }

    Json::Value root;
    file >> root;
    file.close();

    std::string error;
    if (root.isArray())
    {
        for (const auto &entry : root)
        {
            Device device = Device::fromJson(entry);
            InMemoryDeviceRepository::create(device, error);
        }
    }
}

void JsonDeviceRepository::persist()
{
    Json::Value root(Json::arrayValue);
    for (const auto &device : list())
    {
        root.append(device.toJson());
    }

    std::filesystem::create_directories(std::filesystem::path(path_).parent_path());

    std::ofstream file(path_, std::ios::out | std::ios::trunc);
    file << root;
    file.close();
}
