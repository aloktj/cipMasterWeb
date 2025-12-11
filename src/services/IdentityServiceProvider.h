#pragma once

#include "EIPIdentityService.h"
#include <memory>

class IdentityServiceProvider
{
public:
    static std::shared_ptr<IdentityService> instance();
    static void use(const std::shared_ptr<IdentityService> &service);

private:
    static std::shared_ptr<IdentityService> service_;
};

