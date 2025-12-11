#pragma once

#include <cstdint>
#include <json/json.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

enum class SignalDirection
{
    Input,
    Output
};

enum class SignalType
{
    Bool,
    UInt8,
    UInt16,
    UInt32,
    SInt,
    Real32
};

struct SignalEnumOption
{
    int value{0};
    std::string label;

    Json::Value toJson() const
    {
        Json::Value v;
        v["value"] = value;
        v["label"] = label;
        return v;
    }

    static SignalEnumOption fromJson(const Json::Value &value)
    {
        SignalEnumOption option;
        option.value = value.get("value", 0).asInt();
        option.label = value.get("label", "").asString();
        return option;
    }
};

inline bool operator==(const SignalEnumOption &lhs, const SignalEnumOption &rhs)
{
    return lhs.value == rhs.value && lhs.label == rhs.label;
}

struct SignalMapping
{
    std::string name;
    SignalDirection direction{SignalDirection::Input};
    SignalType type{SignalType::UInt8};
    uint16_t byteOffset{0};
    std::optional<uint8_t> bitOffset;
    double scale{1.0};
    double engineeringOffset{0.0};
    std::string units;
    std::vector<SignalEnumOption> enums;

    size_t widthBytes() const
    {
        switch (type)
        {
        case SignalType::Bool:
        case SignalType::UInt8:
        case SignalType::SInt:
            return 1;
        case SignalType::UInt16:
            return 2;
        case SignalType::UInt32:
        case SignalType::Real32:
            return 4;
        default:
            throw std::runtime_error("Unsupported signal type");
        }
    }

    Json::Value toJson() const
    {
        Json::Value value;
        value["name"] = name;
        value["direction"] = direction == SignalDirection::Input ? "input" : "output";
        switch (type)
        {
        case SignalType::Bool:
            value["type"] = "bool";
            break;
        case SignalType::UInt8:
            value["type"] = "uint8";
            break;
        case SignalType::UInt16:
            value["type"] = "uint16";
            break;
        case SignalType::UInt32:
            value["type"] = "uint32";
            break;
        case SignalType::SInt:
            value["type"] = "sint";
            break;
        case SignalType::Real32:
            value["type"] = "real32";
            break;
        }
        value["byteOffset"] = byteOffset;
        if (bitOffset.has_value())
        {
            value["bitOffset"] = *bitOffset;
        }
        value["scale"] = scale;
        value["engineeringOffset"] = engineeringOffset;
        value["units"] = units;
        Json::Value enumArray(Json::arrayValue);
        for (const auto &opt : enums)
        {
            enumArray.append(opt.toJson());
        }
        value["enums"] = enumArray;
        return value;
    }

    static SignalMapping fromJson(const Json::Value &value)
    {
        SignalMapping mapping;
        mapping.name = value.get("name", "").asString();
        const auto dir = value.get("direction", "input").asString();
        mapping.direction = dir == "output" ? SignalDirection::Output : SignalDirection::Input;
        const auto typeStr = value.get("type", "uint8").asString();
        if (typeStr == "bool")
        {
            mapping.type = SignalType::Bool;
        }
        else if (typeStr == "uint16")
        {
            mapping.type = SignalType::UInt16;
        }
        else if (typeStr == "uint32")
        {
            mapping.type = SignalType::UInt32;
        }
        else if (typeStr == "sint")
        {
            mapping.type = SignalType::SInt;
        }
        else if (typeStr == "real32")
        {
            mapping.type = SignalType::Real32;
        }
        else
        {
            mapping.type = SignalType::UInt8;
        }
        mapping.byteOffset = static_cast<uint16_t>(value.get("byteOffset", 0).asUInt());
        if (value.isMember("bitOffset"))
        {
            mapping.bitOffset = static_cast<uint8_t>(value.get("bitOffset", 0).asUInt());
        }
        mapping.scale = value.get("scale", 1.0).asDouble();
        mapping.engineeringOffset = value.get("engineeringOffset", 0.0).asDouble();
        mapping.units = value.get("units", "").asString();
        if (value.isMember("enums") && value["enums"].isArray())
        {
            for (const auto &opt : value["enums"])
            {
                mapping.enums.push_back(SignalEnumOption::fromJson(opt));
            }
        }
        return mapping;
    }
};

struct SignalValue
{
    SignalMapping mapping;
    double engineeringValue{0.0};
    double rawValue{0.0};

    Json::Value toJson() const
    {
        Json::Value v = mapping.toJson();
        v["engineeringValue"] = engineeringValue;
        v["rawValue"] = rawValue;
        return v;
    }
};

inline bool operator==(const SignalMapping &lhs, const SignalMapping &rhs)
{
    return lhs.name == rhs.name && lhs.direction == rhs.direction && lhs.type == rhs.type &&
           lhs.byteOffset == rhs.byteOffset && lhs.bitOffset == rhs.bitOffset && lhs.scale == rhs.scale &&
           lhs.engineeringOffset == rhs.engineeringOffset && lhs.units == rhs.units && lhs.enums == rhs.enums;
}

inline bool operator!=(const SignalMapping &lhs, const SignalMapping &rhs)
{
    return !(lhs == rhs);
}

