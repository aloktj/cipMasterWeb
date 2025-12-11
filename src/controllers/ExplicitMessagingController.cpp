#include "ExplicitMessagingController.h"
#include "models/ExplicitMessage.h"
#include "repositories/RepositoryProvider.h"
#include "services/ExplicitMessageServiceProvider.h"

#include <EIPScanner/cip/GeneralStatusCodes.h>
#include <drogon/HttpResponse.h>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <json/json.h>
#include <limits>
#include <sstream>

using namespace drogon;

namespace
{
std::string normalizeInput(const std::string &value)
{
    auto trimmed = value;
    trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), [](unsigned char c) { return std::isspace(c) != 0; }), trimmed.end());
    return trimmed;
}

PayloadType parsePayloadType(const std::string &value)
{
    auto lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "uint8")
    {
        return PayloadType::UInt8;
    }
    if (lower == "uint16")
    {
        return PayloadType::UInt16;
    }
    if (lower == "uint32")
    {
        return PayloadType::UInt32;
    }
    if (lower == "int8")
    {
        return PayloadType::Int8;
    }
    if (lower == "real32")
    {
        return PayloadType::Real32;
    }
    if (lower == "hex")
    {
        return PayloadType::Hex;
    }
    if (lower == "none")
    {
        return PayloadType::None;
    }
    return PayloadType::None;
}

std::string payloadTypeToString(PayloadType type)
{
    switch (type)
    {
    case PayloadType::UInt8:
        return "uint8";
    case PayloadType::UInt16:
        return "uint16";
    case PayloadType::UInt32:
        return "uint32";
    case PayloadType::Int8:
        return "int8";
    case PayloadType::Real32:
        return "real32";
    case PayloadType::Hex:
        return "hex";
    default:
        return "none";
    }
}

std::string generalStatusDescription(uint8_t status)
{
    switch (static_cast<eipScanner::cip::GeneralStatusCodes>(status))
    {
    case eipScanner::cip::GeneralStatusCodes::SUCCESS:
        return "Success";
    case eipScanner::cip::GeneralStatusCodes::CONNECTION_FAILURE:
        return "Connection failure";
    case eipScanner::cip::GeneralStatusCodes::RESOURCE_UNAVAILABLE:
        return "Resource unavailable";
    case eipScanner::cip::GeneralStatusCodes::INVALID_PARAMETER_VALUE:
        return "Invalid parameter value";
    case eipScanner::cip::GeneralStatusCodes::PATH_SEGMENT_ERROR:
        return "Path segment error";
    case eipScanner::cip::GeneralStatusCodes::PATH_DESTINATION_UNKNOWN:
        return "Path destination unknown";
    case eipScanner::cip::GeneralStatusCodes::PARTIAL_TRANSFER:
        return "Partial transfer";
    case eipScanner::cip::GeneralStatusCodes::CONNECTION_LOST:
        return "Connection lost";
    case eipScanner::cip::GeneralStatusCodes::SERVICE_NOT_SUPPORTED:
        return "Service not supported";
    case eipScanner::cip::GeneralStatusCodes::INVALID_ATTRIBUTE_VALUE:
        return "Invalid attribute value";
    case eipScanner::cip::GeneralStatusCodes::ATTRIBUTE_LIST_ERROR:
        return "Attribute list error";
    case eipScanner::cip::GeneralStatusCodes::ALREADY_IN_REQUESTED_MODE_OR_STATE:
        return "Already in requested mode/state";
    case eipScanner::cip::GeneralStatusCodes::OBJECT_STATE_CONFLICT:
        return "Object state conflict";
    case eipScanner::cip::GeneralStatusCodes::OBJECT_ALREADY_EXISTS:
        return "Object already exists";
    case eipScanner::cip::GeneralStatusCodes::ATTRIBUTE_NOT_SETTABLE:
        return "Attribute not settable";
    case eipScanner::cip::GeneralStatusCodes::PRIVILEGE_VIOLATION:
        return "Privilege violation";
    case eipScanner::cip::GeneralStatusCodes::DEVICE_STATE_CONFLICT:
        return "Device state conflict";
    case eipScanner::cip::GeneralStatusCodes::REPLY_DATA_TOO_LARGE:
        return "Reply data too large";
    case eipScanner::cip::GeneralStatusCodes::NOT_ENOUGH_DATA:
        return "Not enough data";
    case eipScanner::cip::GeneralStatusCodes::ATTRIBUTE_NOT_SUPPORTED:
        return "Attribute not supported";
    case eipScanner::cip::GeneralStatusCodes::TOO_MUCH_DATA:
        return "Too much data";
    case eipScanner::cip::GeneralStatusCodes::OBJECT_DOES_NOT_EXIST:
        return "Object does not exist";
    case eipScanner::cip::GeneralStatusCodes::VENDOR_SPECIFIC:
        return "Vendor specific";
    case eipScanner::cip::GeneralStatusCodes::INVALID_PARAMETER:
        return "Invalid parameter";
    default:
        return "Unknown";
    }
}

std::string toHexString(const std::vector<uint8_t> &data)
{
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < data.size(); ++i)
    {
        ss << std::setw(2) << static_cast<int>(data[i]);
        if (i + 1 < data.size())
        {
            ss << " ";
        }
    }
    return ss.str();
}

bool parseUnsignedField(const std::string &input, uint64_t min, uint64_t max, const std::string &fieldName, uint64_t &output, std::string &error)
{
    if (input.empty())
    {
        error = fieldName + " is required";
        return false;
    }

    try
    {
        auto value = std::stoull(input, nullptr, 0);
        if (value < min || value > max)
        {
            error = fieldName + " must be between " + std::to_string(min) + " and " + std::to_string(max);
            return false;
        }
        output = value;
        return true;
    }
    catch (const std::exception &)
    {
        error = "Invalid number for " + fieldName;
    }
    return false;
}

bool parseSignedField(const std::string &input, int64_t min, int64_t max, const std::string &fieldName, int64_t &output, std::string &error)
{
    if (input.empty())
    {
        error = fieldName + " is required";
        return false;
    }

    try
    {
        auto value = std::stoll(input, nullptr, 0);
        if (value < min || value > max)
        {
            error = fieldName + " must be between " + std::to_string(min) + " and " + std::to_string(max);
            return false;
        }
        output = value;
        return true;
    }
    catch (const std::exception &)
    {
        error = "Invalid number for " + fieldName;
    }
    return false;
}

bool parseOptionalUnsigned(const std::string &input, uint64_t min, uint64_t max, const std::string &fieldName, std::optional<uint16_t> &output, std::string &error)
{
    if (input.empty())
    {
        output.reset();
        return true;
    }

    uint64_t value{0};
    if (!parseUnsignedField(input, min, max, fieldName, value, error))
    {
        return false;
    }
    output = static_cast<uint16_t>(value);
    return true;
}

bool parseHexPayload(const std::string &input, std::vector<uint8_t> &payload, std::string &error)
{
    auto normalized = normalizeInput(input);
    if (normalized.rfind("0x", 0) == 0 || normalized.rfind("0X", 0) == 0)
    {
        normalized = normalized.substr(2);
    }

    if (normalized.empty())
    {
        payload.clear();
        return true;
    }

    if (normalized.size() % 2 != 0)
    {
        error = "Hex payload must have an even number of characters";
        return false;
    }

    payload.clear();
    payload.reserve(normalized.size() / 2);

    for (size_t i = 0; i < normalized.size(); i += 2)
    {
        auto byteString = normalized.substr(i, 2);
        try
        {
            auto value = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
            payload.push_back(value);
        }
        catch (const std::exception &)
        {
            error = "Invalid hex byte: " + byteString;
            return false;
        }
    }

    return true;
}

bool encodePayload(const std::string &input, PayloadType type, std::vector<uint8_t> &payload, std::string &error)
{
    switch (type)
    {
    case PayloadType::None:
        if (!input.empty())
        {
            error = "Payload type 'none' does not accept data";
            return false;
        }
        payload.clear();
        return true;
    case PayloadType::Hex:
        return parseHexPayload(input, payload, error);
    case PayloadType::UInt8:
    {
        uint64_t value{0};
        if (!parseUnsignedField(input, 0, std::numeric_limits<uint8_t>::max(), "Payload", value, error))
        {
            return false;
        }
        payload = {static_cast<uint8_t>(value)};
        return true;
    }
    case PayloadType::UInt16:
    {
        uint64_t value{0};
        if (!parseUnsignedField(input, 0, std::numeric_limits<uint16_t>::max(), "Payload", value, error))
        {
            return false;
        }
        payload = {static_cast<uint8_t>(value & 0xFF), static_cast<uint8_t>((value >> 8) & 0xFF)};
        return true;
    }
    case PayloadType::UInt32:
    {
        uint64_t value{0};
        if (!parseUnsignedField(input, 0, std::numeric_limits<uint32_t>::max(), "Payload", value, error))
        {
            return false;
        }
        payload = {static_cast<uint8_t>(value & 0xFF),
                   static_cast<uint8_t>((value >> 8) & 0xFF),
                   static_cast<uint8_t>((value >> 16) & 0xFF),
                   static_cast<uint8_t>((value >> 24) & 0xFF)};
        return true;
    }
    case PayloadType::Int8:
    {
        int64_t value{0};
        if (!parseSignedField(input, std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max(), "Payload", value, error))
        {
            return false;
        }
        payload = {static_cast<uint8_t>(static_cast<int8_t>(value))};
        return true;
    }
    case PayloadType::Real32:
    {
        if (input.empty())
        {
            error = "Payload is required";
            return false;
        }
        try
        {
            float floatValue = std::stof(input);
            static_assert(sizeof(float) == 4, "Unexpected float size");
            uint32_t raw{0};
            std::memcpy(&raw, &floatValue, sizeof(float));
            payload = {static_cast<uint8_t>(raw & 0xFF),
                       static_cast<uint8_t>((raw >> 8) & 0xFF),
                       static_cast<uint8_t>((raw >> 16) & 0xFF),
                       static_cast<uint8_t>((raw >> 24) & 0xFF)};
            return true;
        }
        catch (const std::exception &)
        {
            error = "Invalid floating point payload";
            return false;
        }
    }
    }
    return false;
}

std::string decodeValue(const std::vector<uint8_t> &data, PayloadType type, std::string &decodeError)
{
    decodeError.clear();
    if (data.empty())
    {
        return "";
    }

    switch (type)
    {
    case PayloadType::UInt8:
        return std::to_string(data.front());
    case PayloadType::UInt16:
        if (data.size() < 2)
        {
            decodeError = "Response too short for uint16";
            return "";
        }
        return std::to_string(static_cast<uint16_t>(data[0] | (data[1] << 8)));
    case PayloadType::UInt32:
        if (data.size() < 4)
        {
            decodeError = "Response too short for uint32";
            return "";
        }
        return std::to_string(static_cast<uint32_t>(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24)));
    case PayloadType::Int8:
        return std::to_string(static_cast<int8_t>(data.front()));
    case PayloadType::Real32:
        if (data.size() < 4)
        {
            decodeError = "Response too short for real32";
            return "";
        }
        {
            uint32_t raw = static_cast<uint32_t>(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
            float value{0.0F};
            std::memcpy(&value, &raw, sizeof(float));
            std::ostringstream ss;
            ss << value;
            return ss.str();
        }
    case PayloadType::Hex:
        return toHexString(data);
    case PayloadType::None:
    default:
        return toHexString(data);
    }
}

Json::Value presets()
{
    Json::Value list(Json::arrayValue);
    auto appendPreset = [&list](const std::string &code, const std::string &label) {
        Json::Value preset(Json::objectValue);
        preset["code"] = code;
        preset["label"] = label;
        list.append(preset);
    };

    appendPreset("0x0E", "Get Attribute Single (0x0E)");
    appendPreset("0x10", "Set Attribute Single (0x10)");
    appendPreset("0x01", "Get Attribute All (0x01)");
    return list;
}

HttpResponsePtr makeErrorResponse(HttpStatusCode code, const std::string &message)
{
    Json::Value payload;
    payload["error"] = message;
    auto response = HttpResponse::newHttpJsonResponse(payload);
    response->setStatusCode(code);
    return response;
}

bool buildRequest(const ExplicitMessageForm &form, ExplicitMessageRequest &request, PayloadType &payloadType, std::string &error)
{
    payloadType = parsePayloadType(form.payloadType);
    auto normalizedType = form.payloadType;
    std::transform(normalizedType.begin(), normalizedType.end(), normalizedType.begin(), ::tolower);
    if (payloadType == PayloadType::None && !normalizedType.empty() && normalizedType != "none")
    {
        error = "Unsupported payload type: " + form.payloadType;
        return false;
    }
    uint64_t service{0};
    if (!parseUnsignedField(form.serviceCode, 0, std::numeric_limits<uint8_t>::max(), "Service code", service, error))
    {
        return false;
    }
    request.serviceCode = static_cast<uint8_t>(service);

    uint64_t classId{0};
    if (!parseUnsignedField(form.classId, 1, std::numeric_limits<uint16_t>::max(), "Class ID", classId, error))
    {
        return false;
    }
    request.classId = static_cast<uint16_t>(classId);

    if (!parseOptionalUnsigned(form.instanceId, 1, std::numeric_limits<uint16_t>::max(), "Instance ID", request.instanceId, error))
    {
        return false;
    }

    if (!parseOptionalUnsigned(form.attributeId, 1, std::numeric_limits<uint16_t>::max(), "Attribute ID", request.attributeId, error))
    {
        return false;
    }

    if (request.attributeId.has_value() && !request.instanceId.has_value())
    {
        error = "Attribute ID requires an Instance ID";
        return false;
    }

    if (!encodePayload(form.payload, payloadType, request.payload, error))
    {
        return false;
    }

    return true;
}

bool buildRequestFromJson(const Json::Value &json, ExplicitMessageRequest &request, PayloadType &payloadType, std::string &error)
{
    ExplicitMessageForm form;
    form.serviceCode = json.get("serviceCode", "").asString();
    form.classId = json.get("classId", "").asString();
    form.instanceId = json.get("instanceId", "").asString();
    form.attributeId = json.get("attributeId", "").asString();
    form.payload = json.get("payload", "").asString();
    form.payloadType = json.get("payloadType", "hex").asString();
    return buildRequest(form, request, payloadType, error);
}
}

void ExplicitMessagingController::sendExplicit(const HttpRequestPtr &request,
                                               std::function<void(const HttpResponsePtr &)> &&callback,
                                               const std::string &deviceName) const
{
    auto repository = RepositoryProvider::instance();
    auto device = repository->find(deviceName);
    if (!device)
    {
        callback(makeErrorResponse(k404NotFound, "Device not found"));
        return;
    }

    auto json = request->getJsonObject();
    if (!json)
    {
        callback(makeErrorResponse(k400BadRequest, "JSON body is required"));
        return;
    }

    ExplicitMessageRequest messageRequest;
    PayloadType payloadType{PayloadType::None};
    std::string error;
    if (!buildRequestFromJson(*json, messageRequest, payloadType, error))
    {
        callback(makeErrorResponse(k400BadRequest, error));
        return;
    }

    auto service = ExplicitMessageServiceProvider::instance();
    auto result = service->sendExplicit(*device, messageRequest, error);
    if (!result)
    {
        callback(makeErrorResponse(k502BadGateway, error));
        return;
    }

    std::string decodeError;
    auto decoded = decodeValue(result->responseData, payloadType, decodeError);
    auto generalName = generalStatusDescription(result->generalStatus);
    auto responseJson = result->toJson(generalName, decoded, decodeError);
    responseJson["responseHex"] = toHexString(result->responseData);
    responseJson["request"] = (*json);

    auto response = HttpResponse::newHttpJsonResponse(responseJson);
    response->setStatusCode(k200OK);
    callback(response);
}

void ExplicitMessagingController::showForm(const HttpRequestPtr &request,
                                           std::function<void(const HttpResponsePtr &)> &&callback,
                                           const std::string &deviceName) const
{
    auto repository = RepositoryProvider::instance();
    auto device = repository->find(deviceName);
    if (!device)
    {
        callback(HttpResponse::newNotFoundResponse());
        return;
    }

    ExplicitMessageForm form;
    auto params = request->getParameters();
    if (params.find("classId") != params.end())
    {
        form.classId = params.at("classId");
    }
    if (params.find("instanceId") != params.end())
    {
        form.instanceId = params.at("instanceId");
    }
    if (params.find("attributeId") != params.end())
    {
        form.attributeId = params.at("attributeId");
    }

    HttpViewData data;
    data.insert("device", *device);
    data.insert("form", form);
    data.insert("error", std::string());
    data.insert("hasResult", false);
    data.insert("generalStatusName", std::string());
    data.insert("decodedValue", std::string());
    data.insert("decodeError", std::string());
    data.insert("responseHex", std::string());
    data.insert("result", ExplicitMessageResult());
    data.insert("payloadType", payloadTypeToString(PayloadType::Hex));
    data.insert("presets", presets());

    auto response = HttpResponse::newHttpViewResponse("devices/explicit_message.csp", data);
    response->setStatusCode(k200OK);
    callback(response);
}

void ExplicitMessagingController::submitForm(const HttpRequestPtr &request,
                                             std::function<void(const HttpResponsePtr &)> &&callback,
                                             const std::string &deviceName) const
{
    auto repository = RepositoryProvider::instance();
    auto device = repository->find(deviceName);
    if (!device)
    {
        callback(HttpResponse::newNotFoundResponse());
        return;
    }

    ExplicitMessageForm form;
    auto params = request->getParameters();
    if (params.find("serviceCode") != params.end())
    {
        form.serviceCode = params.at("serviceCode");
    }
    if (params.find("classId") != params.end())
    {
        form.classId = params.at("classId");
    }
    if (params.find("instanceId") != params.end())
    {
        form.instanceId = params.at("instanceId");
    }
    if (params.find("attributeId") != params.end())
    {
        form.attributeId = params.at("attributeId");
    }
    if (params.find("payload") != params.end())
    {
        form.payload = params.at("payload");
    }
    if (params.find("payloadType") != params.end())
    {
        form.payloadType = params.at("payloadType");
    }

    ExplicitMessageRequest messageRequest;
    PayloadType payloadType{PayloadType::None};
    std::string error;
    auto service = ExplicitMessageServiceProvider::instance();
    auto ok = buildRequest(form, messageRequest, payloadType, error);

    std::string decodeError;
    std::string decodedValue;
    ExplicitMessageResult result;
    bool hasResult = false;
    std::string responseHex;
    std::string generalName;

    if (ok)
    {
        auto response = service->sendExplicit(*device, messageRequest, error);
        if (response)
        {
            result = *response;
            hasResult = true;
            responseHex = toHexString(result.responseData);
            decodedValue = decodeValue(result.responseData, payloadType, decodeError);
            generalName = generalStatusDescription(result.generalStatus);
        }
    }

    HttpViewData data;
    data.insert("device", *device);
    data.insert("form", form);
    data.insert("error", error);
    data.insert("hasResult", hasResult);
    data.insert("result", result);
    data.insert("responseHex", responseHex);
    data.insert("decodedValue", decodedValue);
    data.insert("decodeError", decodeError);
    data.insert("generalStatusName", generalName);
    data.insert("payloadType", payloadTypeToString(payloadType));
    data.insert("presets", presets());

    auto response = HttpResponse::newHttpViewResponse("devices/explicit_message.csp", data);
    response->setStatusCode(ok && hasResult ? k200OK : k400BadRequest);
    callback(response);
}
