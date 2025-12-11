#include "HealthController.h"

#include <drogon/HttpResponse.h>
#include <json/json.h>

using namespace drogon;

void HealthController::healthCheck(const HttpRequestPtr &request,
                                   std::function<void(const HttpResponsePtr &)> &&callback) const
{
    Json::Value payload;
    payload["status"] = "ok";

    auto response = HttpResponse::newHttpJsonResponse(payload);
    response->setStatusCode(k200OK);
    callback(response);
}
