#include "EIPExplicitMessageService.h"

#include <EIPScanner/MessageRouter.h>
#include <EIPScanner/SessionInfo.h>
#include <EIPScanner/cip/EPath.h>
#include <chrono>
#include <system_error>

std::optional<ExplicitMessageResult> EIPExplicitMessageService::sendExplicit(const Device &device,
                                                                             const ExplicitMessageRequest &request,
                                                                             std::string &error)
{
    try
    {
        auto timeout = std::chrono::milliseconds(device.timeoutMs);
        auto session = std::make_shared<eipScanner::SessionInfo>(device.ipAddress, device.port, timeout);
        eipScanner::MessageRouter router;

        eipScanner::cip::EPath path(request.classId);
        if (request.instanceId.has_value())
        {
            if (request.attributeId.has_value())
            {
                path = eipScanner::cip::EPath(request.classId, *request.instanceId, *request.attributeId);
            }
            else
            {
                path = eipScanner::cip::EPath(request.classId, *request.instanceId);
            }
        }

        auto response = router.sendRequest(session,
                                           static_cast<eipScanner::cip::CipUsint>(request.serviceCode),
                                           path,
                                           request.payload);

        ExplicitMessageResult result;
        result.generalStatus = static_cast<uint8_t>(response.getGeneralStatusCode());
        result.additionalStatus = response.getAdditionalStatus();
        result.responseData = response.getData();
        return result;
    }
    catch (const std::system_error &ex)
    {
        if (ex.code() == std::errc::timed_out)
        {
            error = "Request timed out after " + std::to_string(device.timeoutMs) + " ms";
        }
        else
        {
            error = ex.code().message();
        }
    }
    catch (const std::exception &ex)
    {
        error = ex.what();
    }
    catch (...)
    {
        error = "Unknown error while sending explicit message";
    }

    return std::nullopt;
}
