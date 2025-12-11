#include "SignalController.h"

#include "models/SignalMapping.h"
#include "repositories/RepositoryProvider.h"
#include "services/IOSignalService.h"

#include <drogon/HttpResponse.h>
#include <fstream>
#include <optional>
#include <sstream>

using namespace drogon;

namespace
{
HttpResponsePtr makeError(HttpStatusCode code, const std::string &message)
{
    Json::Value payload;
    payload["error"] = message;
    auto resp = HttpResponse::newHttpJsonResponse(payload);
    resp->setStatusCode(code);
    return resp;
}

std::optional<Device> fetchDevice(const std::string &name, std::string &error)
{
    auto repo = RepositoryProvider::instance();
    auto device = repo->find(name);
    if (!device)
    {
        error = "Device not found";
        return std::nullopt;
    }
    return device;
}

std::vector<SignalMapping> defaultMappingsFromEds(const Device &device)
{
    std::vector<SignalMapping> mappings;
    if (!device.connection.has_value())
    {
        return mappings;
    }

    uint16_t outputSize = device.connection->outputAssembly.sizeBytes;
    uint16_t inputSize = device.connection->inputAssembly.sizeBytes;
    if (device.edsFile.has_value())
    {
        std::ifstream file(*device.edsFile);
        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("Input_Size=") != std::string::npos)
            {
                inputSize = static_cast<uint16_t>(std::stoi(line.substr(line.find('=') + 1)));
            }
            if (line.find("Output_Size=") != std::string::npos)
            {
                outputSize = static_cast<uint16_t>(std::stoi(line.substr(line.find('=') + 1)));
            }
        }
    }

    for (uint16_t i = 0; i < outputSize; ++i)
    {
        SignalMapping mapping;
        mapping.name = "OUT_" + std::to_string(i);
        mapping.direction = SignalDirection::Output;
        mapping.type = SignalType::UInt8;
        mapping.byteOffset = i;
        mappings.push_back(mapping);
    }

    for (uint16_t i = 0; i < inputSize; ++i)
    {
        SignalMapping mapping;
        mapping.name = "IN_" + std::to_string(i);
        mapping.direction = SignalDirection::Input;
        mapping.type = SignalType::UInt8;
        mapping.byteOffset = i;
        mappings.push_back(mapping);
    }
    return mappings;
}
}

void SignalController::list(const HttpRequestPtr &request,
                            std::function<void(const HttpResponsePtr &)> &&callback,
                            const std::string &deviceName) const
{
    std::string error;
    auto device = fetchDevice(deviceName, error);
    if (!device)
    {
        callback(makeError(k404NotFound, error));
        return;
    }

    auto service = IOSignalServiceProvider::instance();
    if (service->mappings(deviceName).empty() && !device->signals.empty())
    {
        service->applyMappings(deviceName, device->signals);
    }

    Json::Value payload(Json::arrayValue);
    for (const auto &value : service->snapshot(deviceName))
    {
        payload.append(value.toJson());
    }
    auto resp = HttpResponse::newHttpJsonResponse(payload);
    resp->setStatusCode(k200OK);
    callback(resp);
}

void SignalController::update(const HttpRequestPtr &request,
                              std::function<void(const HttpResponsePtr &)> &&callback,
                              const std::string &deviceName) const
{
    std::string error;
    auto repo = RepositoryProvider::instance();
    auto device = fetchDevice(deviceName, error);
    if (!device)
    {
        callback(makeError(k404NotFound, error));
        return;
    }

    auto json = request->getJsonObject();
    if (!json || !json->isArray())
    {
        callback(makeError(k400BadRequest, "Expected an array of signals"));
        return;
    }

    std::vector<SignalMapping> newMappings;
    for (const auto &entry : *json)
    {
        newMappings.push_back(SignalMapping::fromJson(entry));
    }

    device->signals = newMappings;
    if (!repo->update(deviceName, *device, error))
    {
        callback(makeError(k400BadRequest, error));
        return;
    }

    IOSignalServiceProvider::instance()->applyMappings(deviceName, newMappings);

    Json::Value payload(Json::arrayValue);
    for (const auto &value : IOSignalServiceProvider::instance()->snapshot(deviceName))
    {
        payload.append(value.toJson());
    }
    auto resp = HttpResponse::newHttpJsonResponse(payload);
    resp->setStatusCode(k200OK);
    callback(resp);
}

void SignalController::importMappings(const HttpRequestPtr &request,
                                      std::function<void(const HttpResponsePtr &)> &&callback,
                                      const std::string &deviceName) const
{
    std::string error;
    auto repo = RepositoryProvider::instance();
    auto device = fetchDevice(deviceName, error);
    if (!device)
    {
        callback(makeError(k404NotFound, error));
        return;
    }

    const auto bodyView = request->getBody();
    const std::string body(bodyView.data(), bodyView.size());
    const auto format = request->getParameter("format");
    const bool yaml = format == "yaml";

    std::vector<SignalMapping> imported;
    try
    {
        imported = IOSignalServiceProvider::instance()->importMappings(body, yaml);
    }
    catch (const std::exception &ex)
    {
        callback(makeError(k400BadRequest, ex.what()));
        return;
    }

    if (imported.empty())
    {
        imported = defaultMappingsFromEds(*device);
    }

    device->signals = imported;
    if (!repo->update(deviceName, *device, error))
    {
        callback(makeError(k400BadRequest, error));
        return;
    }
    IOSignalServiceProvider::instance()->applyMappings(deviceName, imported);

    Json::Value payload(Json::arrayValue);
    for (const auto &value : IOSignalServiceProvider::instance()->snapshot(deviceName))
    {
        payload.append(value.toJson());
    }
    auto resp = HttpResponse::newHttpJsonResponse(payload);
    resp->setStatusCode(k200OK);
    callback(resp);
}

void SignalController::exportMappings(const HttpRequestPtr &request,
                                      std::function<void(const HttpResponsePtr &)> &&callback,
                                      const std::string &deviceName) const
{
    std::string error;
    auto device = fetchDevice(deviceName, error);
    if (!device)
    {
        callback(makeError(k404NotFound, error));
        return;
    }

    auto service = IOSignalServiceProvider::instance();
    service->applyMappings(deviceName, device->signals);
    auto format = request->getParameter("format");
    if (format == "yaml")
    {
        auto yaml = service->exportMappingsYaml(deviceName);
        auto resp = HttpResponse::newHttpResponse();
        resp->setContentTypeCode(CT_TEXT_PLAIN);
        resp->setBody(yaml);
        resp->setStatusCode(k200OK);
        callback(resp);
        return;
    }

    auto json = service->exportMappingsJson(deviceName);
    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(k200OK);
    callback(resp);
}

void SignalController::assemblies(const HttpRequestPtr &request,
                                  std::function<void(const HttpResponsePtr &)> &&callback,
                                  const std::string &deviceName) const
{
    std::string error;
    auto device = fetchDevice(deviceName, error);
    if (!device)
    {
        callback(makeError(k404NotFound, error));
        return;
    }

    auto service = IOSignalServiceProvider::instance();
    if (service->mappings(deviceName).empty() && !device->signals.empty())
    {
        service->applyMappings(deviceName, device->signals);
    }

    auto assembliesOpt = service->assemblies(deviceName);
    if (!assembliesOpt)
    {
        callback(makeError(k404NotFound, "Device has no assembly data yet"));
        return;
    }

    Json::Value payload;
    payload["inputBytes"] = Json::arrayValue;
    payload["outputBytes"] = Json::arrayValue;
    for (const auto byte : assembliesOpt->input)
    {
        payload["inputBytes"].append(byte);
    }
    for (const auto byte : assembliesOpt->output)
    {
        payload["outputBytes"].append(byte);
    }

    if (device->connection.has_value())
    {
        Json::Value conn;
        conn["inputInstance"] = device->connection->inputAssembly.instance;
        conn["inputSize"] = device->connection->inputAssembly.sizeBytes;
        conn["outputInstance"] = device->connection->outputAssembly.instance;
        conn["outputSize"] = device->connection->outputAssembly.sizeBytes;
        payload["connection"] = conn;
    }

    auto resp = HttpResponse::newHttpJsonResponse(payload);
    resp->setStatusCode(k200OK);
    callback(resp);
}

void SignalController::setOutputBytes(const HttpRequestPtr &request,
                                      std::function<void(const HttpResponsePtr &)> &&callback,
                                      const std::string &deviceName) const
{
    std::string error;
    auto device = fetchDevice(deviceName, error);
    if (!device)
    {
        callback(makeError(k404NotFound, error));
        return;
    }

    auto json = request->getJsonObject();
    if (!json || (!json->isArray() && !json->isMember("bytes")))
    {
        callback(makeError(k400BadRequest, "Expected an array of bytes or {\"bytes\": []}"));
        return;
    }

    const Json::Value &byteArray = json->isArray() ? *json : (*json)["bytes"];
    if (!byteArray.isArray())
    {
        callback(makeError(k400BadRequest, "bytes must be an array"));
        return;
    }

    std::vector<uint8_t> bytes;
    bytes.reserve(byteArray.size());
    for (const auto &item : byteArray)
    {
        if (!item.isUInt())
        {
            callback(makeError(k400BadRequest, "All bytes must be unsigned integers"));
            return;
        }
        const auto value = item.asUInt();
        if (value > 255)
        {
            callback(makeError(k400BadRequest, "Byte values must be between 0 and 255"));
            return;
        }
        bytes.push_back(static_cast<uint8_t>(value));
    }

    auto service = IOSignalServiceProvider::instance();
    if (service->mappings(deviceName).empty() && !device->signals.empty())
    {
        service->applyMappings(deviceName, device->signals);
    }

    if (!service->applyOutputBytes(deviceName, bytes))
    {
        callback(makeError(k404NotFound, "Device has no output mappings"));
        return;
    }

    auto resp = HttpResponse::newHttpJsonResponse(Json::Value());
    resp->setStatusCode(k202Accepted);
    callback(resp);
}

void SignalController::setValue(const HttpRequestPtr &request,
                                std::function<void(const HttpResponsePtr &)> &&callback,
                                const std::string &deviceName,
                                const std::string &signalName) const
{
    std::string error;
    auto device = fetchDevice(deviceName, error);
    if (!device)
    {
        callback(makeError(k404NotFound, error));
        return;
    }

    auto json = request->getJsonObject();
    if (!json || !json->isMember("value"))
    {
        callback(makeError(k400BadRequest, "Missing value"));
        return;
    }
    const double value = (*json)["value"].asDouble();

    auto service = IOSignalServiceProvider::instance();
    service->applyMappings(deviceName, device->signals);
    if (!service->setOutputValue(deviceName, signalName, value))
    {
        callback(makeError(k404NotFound, "Signal not found or not writable"));
        return;
    }

    Json::Value payload(Json::arrayValue);
    for (const auto &entry : service->snapshot(deviceName))
    {
        payload.append(entry.toJson());
    }
    auto resp = HttpResponse::newHttpJsonResponse(payload);
    resp->setStatusCode(k200OK);
    callback(resp);
}

void SignalController::view(const HttpRequestPtr &request,
                            std::function<void(const HttpResponsePtr &)> &&callback,
                            const std::string &deviceName) const
{
    std::string error;
    auto device = fetchDevice(deviceName, error);
    if (!device)
    {
        callback(HttpResponse::newNotFoundResponse());
        return;
    }

    HttpViewData data;
    data.insert("device", *device);
    auto resp = HttpResponse::newHttpViewResponse("devices/io.csp", data);
    resp->setStatusCode(k200OK);
    callback(resp);
}

void SignalController::assembliesView(const HttpRequestPtr &request,
                                      std::function<void(const HttpResponsePtr &)> &&callback,
                                      const std::string &deviceName) const
{
    std::string error;
    auto device = fetchDevice(deviceName, error);
    if (!device)
    {
        callback(makeError(k404NotFound, error));
        return;
    }

    HttpViewData data;
    data.insert("device", *device);
    auto resp = HttpResponse::newHttpViewResponse("devices/assemblies.csp", data);
    resp->setStatusCode(k200OK);
    callback(resp);
}

