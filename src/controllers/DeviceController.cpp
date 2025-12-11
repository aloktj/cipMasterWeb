#include "DeviceController.h"
#include "models/Device.h"
#include "repositories/RepositoryProvider.h"
#include "services/IdentityServiceProvider.h"

#include <drogon/HttpResponse.h>
#include <json/json.h>

using namespace drogon;

namespace
{
Device deviceFromRequest(const HttpRequestPtr &request)
{
    Device device;
    auto json = request->getJsonObject();
    if (json)
    {
        device.name = (*json)["name"].asString();
        device.ipAddress = (*json)["ipAddress"].asString();
        device.port = static_cast<uint16_t>((*json).get("port", 44818).asUInt());
        device.timeoutMs = (*json).get("timeoutMs", 1000).asUInt();
        if ((*json).isMember("templateRef"))
        {
            device.templateRef = (*json)["templateRef"].asString();
        }
        if ((*json).isMember("edsFile"))
        {
            device.edsFile = (*json)["edsFile"].asString();
        }
    }
    else
    {
        auto params = request->getParameters();
        device.name = params.find("name") != params.end() ? params.at("name") : "";
        device.ipAddress = params.find("ipAddress") != params.end() ? params.at("ipAddress") : "";
        if (params.find("port") != params.end() && !params.at("port").empty())
        {
            device.port = static_cast<uint16_t>(std::stoi(params.at("port")));
        }
        if (params.find("timeoutMs") != params.end() && !params.at("timeoutMs").empty())
        {
            device.timeoutMs = static_cast<uint32_t>(std::stoul(params.at("timeoutMs")));
        }
        if (params.find("templateRef") != params.end() && !params.at("templateRef").empty())
        {
            device.templateRef = params.at("templateRef");
        }
        if (params.find("edsFile") != params.end() && !params.at("edsFile").empty())
        {
            device.edsFile = params.at("edsFile");
        }
    }
    return device;
}

HttpResponsePtr makeErrorResponse(HttpStatusCode code, const std::string &message)
{
    Json::Value payload;
    payload["error"] = message;
    auto response = HttpResponse::newHttpJsonResponse(payload);
    response->setStatusCode(code);
    return response;
}
}

void DeviceController::listDevices(const HttpRequestPtr &request,
                                   std::function<void(const HttpResponsePtr &)> &&callback) const
{
    Json::Value payload(Json::arrayValue);
    for (const auto &device : RepositoryProvider::instance()->list())
    {
        payload.append(device.toJson());
    }

    auto response = HttpResponse::newHttpJsonResponse(payload);
    response->setStatusCode(k200OK);
    callback(response);
}

void DeviceController::getDevice(const HttpRequestPtr &request,
                                 std::function<void(const HttpResponsePtr &)> &&callback,
                                 const std::string &name) const
{
    auto repository = RepositoryProvider::instance();
    auto device = repository->find(name);
    if (!device)
    {
        callback(makeErrorResponse(k404NotFound, "Device not found"));
        return;
    }

    auto response = HttpResponse::newHttpJsonResponse(device->toJson());
    response->setStatusCode(k200OK);
    callback(response);
}

void DeviceController::createDevice(const HttpRequestPtr &request,
                                    std::function<void(const HttpResponsePtr &)> &&callback) const
{
    auto repository = RepositoryProvider::instance();
    auto device = deviceFromRequest(request);
    std::string error;
    if (!repository->create(device, error))
    {
        callback(makeErrorResponse(k400BadRequest, error));
        return;
    }

    auto response = HttpResponse::newHttpJsonResponse(device.toJson());
    response->setStatusCode(k201Created);
    callback(response);
}

void DeviceController::updateDevice(const HttpRequestPtr &request,
                                    std::function<void(const HttpResponsePtr &)> &&callback,
                                    const std::string &name) const
{
    auto repository = RepositoryProvider::instance();
    auto device = deviceFromRequest(request);
    std::string error;
    if (!repository->update(name, device, error))
    {
        callback(makeErrorResponse(k400BadRequest, error));
        return;
    }

    auto response = HttpResponse::newHttpJsonResponse(device.toJson());
    response->setStatusCode(k200OK);
    callback(response);
}

void DeviceController::deleteDevice(const HttpRequestPtr &request,
                                    std::function<void(const HttpResponsePtr &)> &&callback,
                                    const std::string &name) const
{
    auto repository = RepositoryProvider::instance();
    std::string error;
    if (!repository->remove(name, error))
    {
        callback(makeErrorResponse(k404NotFound, error));
        return;
    }

    auto response = HttpResponse::newHttpJsonResponse(Json::Value());
    response->setStatusCode(k204NoContent);
    callback(response);
}

void DeviceController::identity(const HttpRequestPtr &request,
                                std::function<void(const HttpResponsePtr &)> &&callback,
                                const std::string &name) const
{
    auto repository = RepositoryProvider::instance();
    auto device = repository->find(name);
    if (!device)
    {
        callback(makeErrorResponse(k404NotFound, "Device not found"));
        return;
    }

    auto service = IdentityServiceProvider::instance();
    std::string error;
    auto result = service->readIdentity(*device, error);
    if (!result)
    {
        callback(makeErrorResponse(k502BadGateway, error));
        return;
    }

    auto response = HttpResponse::newHttpJsonResponse(result->toJson());
    response->setStatusCode(k200OK);
    callback(response);
}

void DeviceController::listDevicesView(const HttpRequestPtr &request,
                                       std::function<void(const HttpResponsePtr &)> &&callback) const
{
    HttpViewData data;
    data.insert("devices", RepositoryProvider::instance()->list());
    auto response = HttpResponse::newHttpViewResponse("devices/list.csp", data);
    response->setStatusCode(k200OK);
    callback(response);
}

void DeviceController::newDeviceForm(const HttpRequestPtr &request,
                                     std::function<void(const HttpResponsePtr &)> &&callback) const
{
    HttpViewData data;
    auto response = HttpResponse::newHttpViewResponse("devices/create.csp", data);
    response->setStatusCode(k200OK);
    callback(response);
}

void DeviceController::createDeviceFromForm(const HttpRequestPtr &request,
                                            std::function<void(const HttpResponsePtr &)> &&callback) const
{
    auto repository = RepositoryProvider::instance();
    auto device = deviceFromRequest(request);
    std::string error;
    if (!repository->create(device, error))
    {
        HttpViewData data;
        data.insert("error", error);
        data.insert("device", device);
        auto response = HttpResponse::newHttpViewResponse("devices/create.csp", data);
        response->setStatusCode(k400BadRequest);
        callback(response);
        return;
    }

    auto response = HttpResponse::newRedirectionResponse("/devices");
    callback(response);
}

void DeviceController::showDeviceView(const HttpRequestPtr &request,
                                      std::function<void(const HttpResponsePtr &)> &&callback,
                                      const std::string &name) const
{
    auto repository = RepositoryProvider::instance();
    auto device = repository->find(name);
    if (!device)
    {
        callback(HttpResponse::newNotFoundResponse());
        return;
    }

    HttpViewData data;
    data.insert("device", *device);
    auto response = HttpResponse::newHttpViewResponse("devices/show.csp", data);
    response->setStatusCode(k200OK);
    callback(response);
}

void DeviceController::editDeviceForm(const HttpRequestPtr &request,
                                      std::function<void(const HttpResponsePtr &)> &&callback,
                                      const std::string &name) const
{
    auto repository = RepositoryProvider::instance();
    auto device = repository->find(name);
    if (!device)
    {
        callback(HttpResponse::newNotFoundResponse());
        return;
    }

    HttpViewData data;
    data.insert("device", *device);
    auto response = HttpResponse::newHttpViewResponse("devices/edit.csp", data);
    response->setStatusCode(k200OK);
    callback(response);
}

void DeviceController::updateDeviceFromForm(const HttpRequestPtr &request,
                                            std::function<void(const HttpResponsePtr &)> &&callback,
                                            const std::string &name) const
{
    auto repository = RepositoryProvider::instance();
    auto existing = repository->find(name);
    if (!existing)
    {
        callback(HttpResponse::newNotFoundResponse());
        return;
    }

    auto updated = deviceFromRequest(request);
    std::string error;
    if (!repository->update(name, updated, error))
    {
        HttpViewData data;
        data.insert("error", error);
        data.insert("device", updated);
        auto response = HttpResponse::newHttpViewResponse("devices/edit.csp", data);
        response->setStatusCode(k400BadRequest);
        callback(response);
        return;
    }

    auto response = HttpResponse::newRedirectionResponse("/devices/" + updated.name);
    callback(response);
}
