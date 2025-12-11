#include "ExplicitMessageServiceProvider.h"
#include "EIPExplicitMessageService.h"

std::shared_ptr<ExplicitMessageService> ExplicitMessageServiceProvider::service_ = nullptr;

std::shared_ptr<ExplicitMessageService> ExplicitMessageServiceProvider::instance()
{
    if (!service_)
    {
        service_ = std::make_shared<EIPExplicitMessageService>();
    }
    return service_;
}

void ExplicitMessageServiceProvider::use(const std::shared_ptr<ExplicitMessageService> &service)
{
    service_ = service;
}
