#pragma once

#include "DeviceRepository.h"
#include <memory>

class RepositoryProvider
{
public:
    static std::shared_ptr<DeviceRepository> instance();
    static void use(const std::shared_ptr<DeviceRepository> &repository);

private:
    static std::shared_ptr<DeviceRepository> repository_;
};

