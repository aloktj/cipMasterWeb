#pragma once

#include "ExplicitMessageService.h"

class EIPExplicitMessageService : public ExplicitMessageService
{
public:
    std::optional<ExplicitMessageResult> sendExplicit(const Device &device,
                                                      const ExplicitMessageRequest &request,
                                                      std::string &error) override;
};
