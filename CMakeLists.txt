cmake_minimum_required(VERSION 3.16)
project(cipMasterWeb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow find_package to pick up custom modules shipped with this repo.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

option(BUILD_EIPSCANNER_FROM_SOURCE "Fetch and build EIPScanner from source instead of using a preinstalled package" OFF)
set(EIPSCANNER_VERSION_TAG "1.3.0")

# Drogon's MySQL finder misses Debian multi-arch library paths unless they are
# pre-populated. Help it locate the standard packages on Ubuntu/Debian.
if(NOT DEFINED MYSQL_INCLUDE_DIRS AND EXISTS "/usr/include/mysql")
  set(MYSQL_INCLUDE_DIRS "/usr/include/mysql" CACHE PATH "MySQL include dir" FORCE)
endif()

if(NOT DEFINED MYSQL_LIBRARIES)
  set(_mysql_lib_candidate "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/libmysqlclient.so")
  if(EXISTS "${_mysql_lib_candidate}")
    set(MYSQL_LIBRARIES "${_mysql_lib_candidate}" CACHE FILEPATH "MySQL client library" FORCE)
  endif()
endif()

find_package(Drogon CONFIG REQUIRED)

if(BUILD_EIPSCANNER_FROM_SOURCE)
  include(FetchContent)

  # EIPScanner 1.3.0 matches the bundled .deb in this repository. Building from
  # source keeps Raspberry Pi and arm64 builds working when the amd64 .deb is
  # not available.
  set(TEST_ENABLED OFF CACHE BOOL "Disable EIPScanner unit tests" FORCE)
  set(EXAMPLE_ENABLED OFF CACHE BOOL "Disable EIPScanner examples" FORCE)
  set(ENABLE_VENDOR_SRC ON CACHE BOOL "Enable vendor specific code" FORCE)

  FetchContent_Declare(
    EIPScannerSrc
    GIT_REPOSITORY https://github.com/nimbuscontrols/EIPScanner.git
    GIT_TAG ${EIPSCANNER_VERSION_TAG}
  )

  FetchContent_MakeAvailable(EIPScannerSrc)

  if(NOT TARGET EIPScanner::EIPScanner)
    add_library(EIPScanner::EIPScanner ALIAS EIPScanner)
  endif()

  target_include_directories(EIPScanner PUBLIC
    $<BUILD_INTERFACE:${EIPScannerSrc_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include/EIPScanner>
  )
else()
  find_package(EIPScanner REQUIRED)
endif()

enable_testing()

add_executable(tcms_cip_sim
  src/main.cpp
  src/controllers/ConnectionController.cpp
  src/controllers/DeviceController.cpp
  src/controllers/HomeController.cpp
  src/controllers/SignalController.cpp
  src/controllers/ExplicitMessagingController.cpp
  src/controllers/HealthController.cpp
  src/repositories/InMemoryDeviceRepository.cpp
  src/repositories/JsonDeviceRepository.cpp
  src/repositories/RepositoryProvider.cpp
  src/services/ConnectionLifecycleService.cpp
  src/services/EIPExplicitMessageService.cpp
  src/services/EIPIdentityService.cpp
  src/services/IOSignalService.cpp
  src/services/ExplicitMessageServiceProvider.cpp
  src/services/IdentityServiceProvider.cpp
)

target_compile_features(tcms_cip_sim PRIVATE cxx_std_17)

target_include_directories(tcms_cip_sim PRIVATE src /usr/include/jsoncpp)

target_link_libraries(tcms_cip_sim PRIVATE
  Drogon::Drogon
  EIPScanner::EIPScanner
  yaml-cpp
)

drogon_create_views(tcms_cip_sim ${CMAKE_CURRENT_SOURCE_DIR}/views ${CMAKE_CURRENT_BINARY_DIR}/generated/views)

add_subdirectory(tests)
