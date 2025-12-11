#pragma once

#include "ExplicitMessageService.h"
#include <memory>

class ExplicitMessageServiceProvider
{
public:
    static std::shared_ptr<ExplicitMessageService> instance();
    static void use(const std::shared_ptr<ExplicitMessageService> &service);

private:
    static std::shared_ptr<ExplicitMessageService> service_;
};
