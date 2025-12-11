if(NOT TARGET Drogon::Drogon)
  find_package(Drogon CONFIG REQUIRED)
endif()

if(NOT TARGET EIPScanner::EIPScanner)
  find_package(EIPScanner REQUIRED)
endif()

add_executable(library_detection
  library_detection.cpp
)

target_compile_features(library_detection PRIVATE cxx_std_17)

target_link_libraries(library_detection PRIVATE
  Drogon::Drogon
  EIPScanner::EIPScanner
)

add_executable(repository_tests
  repository_tests.cpp
)

target_include_directories(repository_tests PRIVATE ${PROJECT_SOURCE_DIR}/src /usr/include/jsoncpp)
target_compile_features(repository_tests PRIVATE cxx_std_17)

target_link_libraries(repository_tests PRIVATE
  Drogon::Drogon
  EIPScanner::EIPScanner
)
target_sources(repository_tests PRIVATE
  ${PROJECT_SOURCE_DIR}/src/repositories/InMemoryDeviceRepository.cpp
  ${PROJECT_SOURCE_DIR}/src/repositories/JsonDeviceRepository.cpp
)

add_executable(identity_tests
  identity_tests.cpp
)

target_include_directories(identity_tests PRIVATE ${PROJECT_SOURCE_DIR}/src /usr/include/jsoncpp)
target_compile_features(identity_tests PRIVATE cxx_std_17)

target_link_libraries(identity_tests PRIVATE
  Drogon::Drogon
  EIPScanner::EIPScanner
)
target_sources(identity_tests PRIVATE
  ${PROJECT_SOURCE_DIR}/src/services/EIPIdentityService.cpp
  ${PROJECT_SOURCE_DIR}/src/services/IdentityServiceProvider.cpp
)

add_test(NAME repository_tests COMMAND repository_tests)
add_test(NAME identity_tests COMMAND identity_tests)
