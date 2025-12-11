cmake_minimum_required(VERSION 3.16)
project(cipMasterWeb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow find_package to pick up custom modules shipped with this repo.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

find_package(Drogon CONFIG REQUIRED)
find_package(EIPScanner REQUIRED)

enable_testing()

add_executable(tcms_cip_sim
  src/main.cpp
  src/controllers/DeviceController.cpp
  src/controllers/HealthController.cpp
  src/repositories/InMemoryDeviceRepository.cpp
  src/repositories/JsonDeviceRepository.cpp
  src/repositories/RepositoryProvider.cpp
  src/services/EIPIdentityService.cpp
  src/services/IdentityServiceProvider.cpp
)

target_compile_features(tcms_cip_sim PRIVATE cxx_std_17)

target_include_directories(tcms_cip_sim PRIVATE src /usr/include/jsoncpp)

target_link_libraries(tcms_cip_sim PRIVATE
  Drogon::Drogon
  EIPScanner::EIPScanner
)

drogon_create_views(tcms_cip_sim ${CMAKE_CURRENT_SOURCE_DIR}/views ${CMAKE_CURRENT_BINARY_DIR}/generated/views)

add_subdirectory(tests)
