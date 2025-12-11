#include "HomeController.h"
#include "repositories/RepositoryProvider.h"

#include <drogon/HttpResponse.h>

using namespace drogon;

void HomeController::index(const HttpRequestPtr &request,
                           std::function<void(const HttpResponsePtr &)> &&callback) const
{
    HttpViewData data;
    data.insert("devices", RepositoryProvider::instance()->list());

    auto response = HttpResponse::newHttpViewResponse("home.csp", data);
    response->setStatusCode(k200OK);
    callback(response);
}
