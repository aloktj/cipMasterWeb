#include "IdentityServiceProvider.h"

std::shared_ptr<IdentityService> IdentityServiceProvider::service_ = nullptr;

std::shared_ptr<IdentityService> IdentityServiceProvider::instance()
{
    if (!service_)
    {
        service_ = std::make_shared<EIPIdentityService>();
    }
    return service_;
}

void IdentityServiceProvider::use(const std::shared_ptr<IdentityService> &service)
{
    service_ = service;
}
