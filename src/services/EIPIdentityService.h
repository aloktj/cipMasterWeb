#pragma once

#include "IdentityService.h"
#include <memory>

class EIPIdentityService : public IdentityService
{
public:
    std::optional<IdentityResult> readIdentity(const Device &device, std::string &error) override;
};

