#pragma once

#include <drogon/HttpController.h>

class HomeController : public drogon::HttpController<HomeController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(HomeController::index, "/", drogon::Get);
    METHOD_LIST_END

    void index(const drogon::HttpRequestPtr &request,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
};
