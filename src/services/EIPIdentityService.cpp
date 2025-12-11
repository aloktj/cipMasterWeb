#include "EIPIdentityService.h"

#include <EIPScanner/IdentityObject.h>
#include <EIPScanner/SessionInfo.h>
#include <chrono>
#include <exception>

std::optional<IdentityResult> EIPIdentityService::readIdentity(const Device &device, std::string &error)
{
    try
    {
        auto timeout = std::chrono::milliseconds(device.timeoutMs);
        auto session = std::make_shared<eipScanner::SessionInfo>(device.ipAddress, device.port, timeout);
        eipScanner::IdentityObject identity(1, session);

        IdentityResult result;
        result.vendorId = identity.getVendorId();
        result.deviceType = identity.getDeviceType();
        result.productCode = identity.getProductCode();
        result.revisionMajor = identity.getRevision().getMajorRevision();
        result.revisionMinor = identity.getRevision().getMinorRevision();
        result.serialNumber = identity.getSerialNumber();
        result.productName = identity.getProductName();
        return result;
    }
    catch (const std::exception &ex)
    {
        error = ex.what();
    }
    catch (...)
    {
        error = "Unknown error while reading identity";
    }

    return std::nullopt;
}
