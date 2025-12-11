#pragma once

#include <cstdint>
#include <json/json.h>
#include <optional>
#include <string>
#include <vector>

enum class PayloadType
{
    None,
    Hex,
    UInt8,
    UInt16,
    UInt32,
    Int8,
    Real32
};

struct ExplicitMessageRequest
{
    uint8_t serviceCode{0};
    uint16_t classId{0};
    std::optional<uint16_t> instanceId;
    std::optional<uint16_t> attributeId;
    std::vector<uint8_t> payload;
};

struct ExplicitMessageResult
{
    uint8_t generalStatus{0};
    std::vector<uint16_t> additionalStatus;
    std::vector<uint8_t> responseData;

    Json::Value toJson(const std::string &generalStatusName,
                       const std::string &decodedValue,
                       const std::string &decodeError) const
    {
        Json::Value value;
        value["generalStatus"] = generalStatus;
        value["generalStatusName"] = generalStatusName;
        Json::Value additional(Json::arrayValue);
        for (auto status : additionalStatus)
        {
            additional.append(status);
        }
        value["additionalStatus"] = additional;

        Json::Value data(Json::arrayValue);
        for (auto byte : responseData)
        {
            data.append(byte);
        }
        value["responseData"] = data;
        if (!decodedValue.empty())
        {
            value["decodedValue"] = decodedValue;
        }
        if (!decodeError.empty())
        {
            value["decodeError"] = decodeError;
        }
        return value;
    }
};

struct ExplicitMessageForm
{
    std::string serviceCode{"0x0E"};
    std::string classId;
    std::string instanceId;
    std::string attributeId;
    std::string payload;
    std::string payloadType{"hex"};
    bool useEightBitSegments{false};
};
