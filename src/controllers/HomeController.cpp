#include "HomeController.h"

#include <drogon/HttpResponse.h>

using namespace drogon;

void HomeController::index(const HttpRequestPtr &request,
                           std::function<void(const HttpResponsePtr &)> &&callback) const
{
    auto response = HttpResponse::newRedirectionResponse("/devices");
    callback(response);
}
