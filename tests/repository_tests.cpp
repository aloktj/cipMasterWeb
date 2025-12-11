#include "repositories/InMemoryDeviceRepository.h"
#include "repositories/JsonDeviceRepository.h"
#include <cassert>
#include <filesystem>
#include <iostream>

int main()
{
    {
        InMemoryDeviceRepository repository;
        Device device{"Test", "192.168.1.10", 44818, 1000};
        std::string error;
        assert(repository.create(device, error));
        auto loaded = repository.find("Test");
        assert(loaded.has_value());
        assert(loaded->ipAddress == device.ipAddress);

        device.ipAddress = "10.0.0.5";
        assert(repository.update("Test", device, error));
        auto updated = repository.find("Test");
        assert(updated->ipAddress == "10.0.0.5");

        assert(repository.remove("Test", error));
        assert(!repository.find("Test").has_value());
    }

    {
        auto tempPath = std::filesystem::temp_directory_path() / "device_repo_test.json";
        std::filesystem::remove(tempPath);
        JsonDeviceRepository repository(tempPath.string());
        std::string error;

        Device device{"Persisted", "127.0.0.1", 44818, 1500};
        assert(repository.create(device, error));

        JsonDeviceRepository reloaded(tempPath.string());
        auto restored = reloaded.find("Persisted");
        assert(restored.has_value());
        assert(restored->timeoutMs == 1500);
        std::filesystem::remove(tempPath);
    }

    std::cout << "Repository tests passed" << std::endl;
    return 0;
}
