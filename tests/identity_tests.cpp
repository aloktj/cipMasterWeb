#include "services/IdentityService.h"
#include "services/IdentityServiceProvider.h"
#include <cassert>
#include <iostream>

class StubIdentityService : public IdentityService
{
public:
    std::optional<IdentityResult> readIdentity(const Device &device, std::string &error) override
    {
        IdentityResult result;
        result.vendorId = 123;
        result.deviceType = 77;
        result.productCode = 42;
        result.revisionMajor = 1;
        result.revisionMinor = 2;
        result.serialNumber = 5555;
        result.productName = "Stubbed Device: " + device.name;
        return result;
    }
};

int main()
{
    auto stub = std::make_shared<StubIdentityService>();
    IdentityServiceProvider::use(stub);

    Device device{"Demo", "192.168.1.20", 44818, 1000};
    std::string error;
    auto result = IdentityServiceProvider::instance()->readIdentity(device, error);
    assert(result.has_value());
    assert(result->vendorId == 123);
    assert(result->productName.find(device.name) != std::string::npos);

    std::cout << "Identity stub test passed" << std::endl;
    return 0;
}
