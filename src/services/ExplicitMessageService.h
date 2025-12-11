#pragma once

#include "models/Device.h"
#include "models/ExplicitMessage.h"
#include <optional>
#include <string>

class ExplicitMessageService
{
public:
    virtual ~ExplicitMessageService() = default;

    virtual std::optional<ExplicitMessageResult> sendExplicit(const Device &device,
                                                               const ExplicitMessageRequest &request,
                                                               std::string &error) = 0;
};
