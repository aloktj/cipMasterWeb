#pragma once

#include <drogon/HttpController.h>

class DeviceController : public drogon::HttpController<DeviceController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(DeviceController::listDevices, "/api/devices", drogon::Get);
    ADD_METHOD_TO(DeviceController::getDevice, "/api/devices/{1}", drogon::Get);
    ADD_METHOD_TO(DeviceController::createDevice, "/api/devices", drogon::Post);
    ADD_METHOD_TO(DeviceController::updateDevice, "/api/devices/{1}", drogon::Put);
    ADD_METHOD_TO(DeviceController::deleteDevice, "/api/devices/{1}", drogon::Delete);
    ADD_METHOD_TO(DeviceController::identity, "/api/devices/{1}/identity", drogon::Get);

    ADD_METHOD_TO(DeviceController::listDevicesView, "/devices", drogon::Get);
    ADD_METHOD_TO(DeviceController::newDeviceForm, "/devices/new", drogon::Get);
    ADD_METHOD_TO(DeviceController::createDeviceFromForm, "/devices", drogon::Post);
    ADD_METHOD_TO(DeviceController::showDeviceView, "/devices/{1}", drogon::Get);
    ADD_METHOD_TO(DeviceController::editDeviceForm, "/devices/{1}/edit", drogon::Get);
    ADD_METHOD_TO(DeviceController::updateDeviceFromForm, "/devices/{1}/edit", drogon::Post);
    METHOD_LIST_END

    void listDevices(const drogon::HttpRequestPtr &request,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
    void getDevice(const drogon::HttpRequestPtr &request,
                   std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                   const std::string &name) const;
    void createDevice(const drogon::HttpRequestPtr &request,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
    void updateDevice(const drogon::HttpRequestPtr &request,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                      const std::string &name) const;
    void deleteDevice(const drogon::HttpRequestPtr &request,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                      const std::string &name) const;
    void identity(const drogon::HttpRequestPtr &request,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                  const std::string &name) const;

    void listDevicesView(const drogon::HttpRequestPtr &request,
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
    void newDeviceForm(const drogon::HttpRequestPtr &request,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
    void createDeviceFromForm(const drogon::HttpRequestPtr &request,
                              std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
    void showDeviceView(const drogon::HttpRequestPtr &request,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                        const std::string &name) const;
    void editDeviceForm(const drogon::HttpRequestPtr &request,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                        const std::string &name) const;
    void updateDeviceFromForm(const drogon::HttpRequestPtr &request,
                              std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                              const std::string &name) const;
};

