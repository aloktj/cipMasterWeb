#pragma once

#include <drogon/HttpController.h>

class HealthController : public drogon::HttpController<HealthController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(HealthController::healthCheck, "/healthz", drogon::Get);
    ADD_METHOD_TO(HealthController::healthCheck, "/health", drogon::Get);
    ADD_METHOD_TO(HealthController::healthCheck, "/api/health", drogon::Get);
    METHOD_LIST_END

    void healthCheck(const drogon::HttpRequestPtr &request,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
};
