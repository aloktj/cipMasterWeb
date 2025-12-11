#include "ConnectionController.h"

#include "repositories/RepositoryProvider.h"
#include "services/ConnectionLifecycleServiceProvider.h"

#include <drogon/HttpResponse.h>
#include <json/json.h>

using namespace drogon;

namespace
{
HttpResponsePtr makeError(HttpStatusCode code, const std::string &message)
{
    Json::Value payload;
    payload["error"] = message;
    auto response = HttpResponse::newHttpJsonResponse(payload);
    response->setStatusCode(code);
    return response;
}
} // namespace

void ConnectionController::listStatuses(const HttpRequestPtr &request,
                                        std::function<void(const HttpResponsePtr &)> &&callback) const
{
    auto service = ConnectionLifecycleServiceProvider::instance();
    Json::Value payload(Json::arrayValue);
    for (const auto &status : service->listStatuses())
    {
        payload.append(status.toJson());
    }

    auto response = HttpResponse::newHttpJsonResponse(payload);
    response->setStatusCode(k200OK);
    callback(response);
}

void ConnectionController::openConnection(const HttpRequestPtr &request,
                                          std::function<void(const HttpResponsePtr &)> &&callback,
                                          const std::string &name) const
{
    auto repo = RepositoryProvider::instance();
    auto device = repo->find(name);
    if (!device)
    {
        callback(makeError(k404NotFound, "Device not found"));
        return;
    }

    if (!device->connection.has_value())
    {
        callback(makeError(k400BadRequest, "Device does not include a connection configuration"));
        return;
    }

    auto service = ConnectionLifecycleServiceProvider::instance();
    std::string error;
    if (!service->open(*device, error))
    {
        callback(makeError(k502BadGateway, error));
        return;
    }

    auto response = HttpResponse::newHttpJsonResponse(device->connection->toJson());
    response->setStatusCode(k202Accepted);
    callback(response);
}

void ConnectionController::closeConnection(const HttpRequestPtr &request,
                                           std::function<void(const HttpResponsePtr &)> &&callback,
                                           const std::string &name) const
{
    auto service = ConnectionLifecycleServiceProvider::instance();
    std::string error;
    if (!service->close(name, error))
    {
        callback(makeError(k404NotFound, error));
        return;
    }

    auto response = HttpResponse::newHttpJsonResponse(Json::Value());
    response->setStatusCode(k200OK);
    callback(response);
}

void ConnectionController::view(const HttpRequestPtr &request,
                                std::function<void(const HttpResponsePtr &)> &&callback) const
{
    HttpViewData data;
    data.insert("devices", RepositoryProvider::instance()->list());
    auto response = HttpResponse::newHttpViewResponse("connections/index.csp", data);
    response->setStatusCode(k200OK);
    callback(response);
}

