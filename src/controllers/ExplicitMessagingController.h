#pragma once

#include <drogon/HttpController.h>

class ExplicitMessagingController : public drogon::HttpController<ExplicitMessagingController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ExplicitMessagingController::sendExplicit, "/api/devices/{1}/explicit", drogon::Post);
    ADD_METHOD_TO(ExplicitMessagingController::showForm, "/devices/{1}/explicit", drogon::Get);
    ADD_METHOD_TO(ExplicitMessagingController::submitForm, "/devices/{1}/explicit", drogon::Post);
    METHOD_LIST_END

    void sendExplicit(const drogon::HttpRequestPtr &request,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                      const std::string &deviceName) const;

    void showForm(const drogon::HttpRequestPtr &request,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                  const std::string &deviceName) const;

    void submitForm(const drogon::HttpRequestPtr &request,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                    const std::string &deviceName) const;
};
