#pragma once

#include <drogon/HttpController.h>

class SignalController : public drogon::HttpController<SignalController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(SignalController::list, "/api/devices/{1}/signals", drogon::Get);
    ADD_METHOD_TO(SignalController::update, "/api/devices/{1}/signals", drogon::Put);
    ADD_METHOD_TO(SignalController::importMappings, "/api/devices/{1}/signals/import", drogon::Post);
    ADD_METHOD_TO(SignalController::exportMappings, "/api/devices/{1}/signals/export", drogon::Get);
    ADD_METHOD_TO(SignalController::setValue, "/api/devices/{1}/signals/{2}/value", drogon::Post);
    ADD_METHOD_TO(SignalController::view, "/devices/{1}/io", drogon::Get);
    METHOD_LIST_END

    void list(const drogon::HttpRequestPtr &request,
              std::function<void(const drogon::HttpResponsePtr &)> &&callback,
              const std::string &deviceName) const;
    void update(const drogon::HttpRequestPtr &request,
                std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                const std::string &deviceName) const;
    void importMappings(const drogon::HttpRequestPtr &request,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                        const std::string &deviceName) const;
    void exportMappings(const drogon::HttpRequestPtr &request,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                        const std::string &deviceName) const;
    void setValue(const drogon::HttpRequestPtr &request,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                  const std::string &deviceName,
                  const std::string &signalName) const;
    void view(const drogon::HttpRequestPtr &request,
              std::function<void(const drogon::HttpResponsePtr &)> &&callback,
              const std::string &deviceName) const;
};

