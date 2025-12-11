#pragma once

#include <drogon/HttpController.h>

class ConnectionController : public drogon::HttpController<ConnectionController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ConnectionController::listStatuses, "/api/connections", drogon::Get);
    ADD_METHOD_TO(ConnectionController::openConnection, "/api/connections/{1}/open", drogon::Post);
    ADD_METHOD_TO(ConnectionController::closeConnection, "/api/connections/{1}/close", drogon::Post);
    ADD_METHOD_TO(ConnectionController::view, "/connections", drogon::Get);
    METHOD_LIST_END

    void listStatuses(const drogon::HttpRequestPtr &request,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
    void openConnection(const drogon::HttpRequestPtr &request,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                        const std::string &name) const;
    void closeConnection(const drogon::HttpRequestPtr &request,
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                         const std::string &name) const;
    void view(const drogon::HttpRequestPtr &request,
              std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
};

