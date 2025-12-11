#pragma once

#include "models/Device.h"
#include <json/json.h>
#include <optional>

struct IdentityResult
{
    uint16_t vendorId{0};
    uint16_t deviceType{0};
    uint16_t productCode{0};
    uint16_t revisionMajor{0};
    uint16_t revisionMinor{0};
    uint32_t serialNumber{0};
    std::string productName;

    Json::Value toJson() const
    {
        Json::Value value;
        value["vendorId"] = vendorId;
        value["deviceType"] = deviceType;
        value["productCode"] = productCode;
        value["revisionMajor"] = revisionMajor;
        value["revisionMinor"] = revisionMinor;
        value["serialNumber"] = serialNumber;
        value["productName"] = productName;
        return value;
    }
};

class IdentityService
{
public:
    virtual ~IdentityService() = default;

    virtual std::optional<IdentityResult> readIdentity(const Device &device, std::string &error) = 0;
};

