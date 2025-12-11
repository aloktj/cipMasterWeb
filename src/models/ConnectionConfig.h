#pragma once

#include <json/json.h>
#include <optional>

struct AssemblyInstanceConfig
{
    uint16_t instance{0};
    uint16_t sizeBytes{0};

    Json::Value toJson() const
    {
        Json::Value value;
        value["instance"] = instance;
        value["sizeBytes"] = sizeBytes;
        return value;
    }

    static AssemblyInstanceConfig fromJson(const Json::Value &value)
    {
        AssemblyInstanceConfig config;
        config.instance = static_cast<uint16_t>(value.get("instance", 0).asUInt());
        config.sizeBytes = static_cast<uint16_t>(value.get("sizeBytes", 0).asUInt());
        return config;
    }

    bool isValid(std::string &error) const
    {
        if (instance == 0)
        {
            error = "Assembly instance must be greater than zero";
            return false;
        }
        if (sizeBytes == 0)
        {
            error = "Assembly size must be greater than zero";
            return false;
        }
        return true;
    }
};

struct ConnectionConfig
{
    AssemblyInstanceConfig outputAssembly;
    AssemblyInstanceConfig inputAssembly;
    std::optional<AssemblyInstanceConfig> configAssembly;
    uint32_t rpiUs{100000};
    bool multicast{false};
    bool useLargeForwardOpen{false};

    Json::Value toJson() const
    {
        Json::Value value;
        value["outputAssembly"] = outputAssembly.toJson();
        value["inputAssembly"] = inputAssembly.toJson();
        if (configAssembly.has_value())
        {
            value["configAssembly"] = configAssembly->toJson();
        }
        value["rpiUs"] = rpiUs;
        value["multicast"] = multicast;
        value["useLargeForwardOpen"] = useLargeForwardOpen;
        return value;
    }

    static ConnectionConfig fromJson(const Json::Value &value)
    {
        ConnectionConfig config;
        if (value.isMember("outputAssembly"))
        {
            config.outputAssembly = AssemblyInstanceConfig::fromJson(value["outputAssembly"]);
        }
        if (value.isMember("inputAssembly"))
        {
            config.inputAssembly = AssemblyInstanceConfig::fromJson(value["inputAssembly"]);
        }
        if (value.isMember("configAssembly"))
        {
            config.configAssembly = AssemblyInstanceConfig::fromJson(value["configAssembly"]);
        }
        config.rpiUs = value.get("rpiUs", 100000).asUInt();
        config.multicast = value.get("multicast", false).asBool();
        config.useLargeForwardOpen = value.get("useLargeForwardOpen", false).asBool();
        return config;
    }

    bool isValid(std::string &error) const
    {
        if (!outputAssembly.isValid(error))
        {
            error = "Output: " + error;
            return false;
        }
        if (!inputAssembly.isValid(error))
        {
            error = "Input: " + error;
            return false;
        }
        if (configAssembly.has_value())
        {
            if (!configAssembly->isValid(error))
            {
                error = "Config: " + error;
                return false;
            }
        }
        if (rpiUs == 0)
        {
            error = "RPI must be greater than zero";
            return false;
        }
        return true;
    }
};

