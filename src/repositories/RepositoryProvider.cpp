#include "RepositoryProvider.h"
#include "InMemoryDeviceRepository.h"
#include "JsonDeviceRepository.h"

#include <drogon/drogon.h>

std::shared_ptr<DeviceRepository> RepositoryProvider::repository_ = nullptr;

std::shared_ptr<DeviceRepository> RepositoryProvider::instance()
{
    if (!repository_)
    {
        std::string storageType = "json";
        std::string storagePath = "config/devices.json";

        auto config = drogon::app().getCustomConfig();
        if (config.isMember("repository"))
        {
            const auto &repoConfig = config["repository"];
            storageType = repoConfig.get("type", storageType).asString();
            storagePath = repoConfig.get("path", storagePath).asString();
        }

        if (storageType == "json")
        {
            repository_ = std::make_shared<JsonDeviceRepository>(storagePath);
        }
        else
        {
            repository_ = std::make_shared<InMemoryDeviceRepository>();
        }
    }
    return repository_;
}

void RepositoryProvider::use(const std::shared_ptr<DeviceRepository> &repository)
{
    repository_ = repository;
}
