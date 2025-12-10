#include <EIPScanner/cip/CipRevision.h>
#include <iostream>

int main() {
    // Touch a simple data type from EIPScanner to ensure headers and symbols resolve.
    eipScanner::cip::CipRevision revision{1, 0};

    std::cout << "Drogon and EIPScanner detected successfully" << std::endl;
    std::cout << "CIP Revision stub: " << revision.toString() << std::endl;
    return 0;
}
