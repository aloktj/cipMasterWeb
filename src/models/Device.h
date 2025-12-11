#pragma once

#include <json/json.h>
#include <optional>
#include <string>

struct Device
{
    std::string name;
    std::string ipAddress;
    uint16_t port{44818};
    uint32_t timeoutMs{1000};
    std::optional<std::string> templateRef;
    std::optional<std::string> edsFile;

    Json::Value toJson() const
    {
        Json::Value value;
        value["name"] = name;
        value["ipAddress"] = ipAddress;
        value["port"] = port;
        value["timeoutMs"] = timeoutMs;
        if (templateRef.has_value())
        {
            value["templateRef"] = *templateRef;
        }
        if (edsFile.has_value())
        {
            value["edsFile"] = *edsFile;
        }
        return value;
    }

    static Device fromJson(const Json::Value &value)
    {
        Device device;
        device.name = value.get("name", "").asString();
        device.ipAddress = value.get("ipAddress", "").asString();
        device.port = static_cast<uint16_t>(value.get("port", 44818).asUInt());
        device.timeoutMs = value.get("timeoutMs", 1000).asUInt();
        if (value.isMember("templateRef"))
        {
            device.templateRef = value["templateRef"].asString();
        }
        if (value.isMember("edsFile"))
        {
            device.edsFile = value["edsFile"].asString();
        }
        return device;
    }

    bool isValid(std::string &error) const
    {
        if (name.empty())
        {
            error = "Device name is required";
            return false;
        }
        if (ipAddress.empty())
        {
            error = "IP address is required";
            return false;
        }
        if (port == 0)
        {
            error = "Port must be greater than zero";
            return false;
        }
        if (timeoutMs == 0)
        {
            error = "Timeout must be greater than zero";
            return false;
        }
        return true;
    }
};

inline bool operator==(const Device &lhs, const Device &rhs)
{
    return lhs.name == rhs.name && lhs.ipAddress == rhs.ipAddress && lhs.port == rhs.port &&
           lhs.timeoutMs == rhs.timeoutMs && lhs.templateRef == rhs.templateRef &&
           lhs.edsFile == rhs.edsFile;
}
