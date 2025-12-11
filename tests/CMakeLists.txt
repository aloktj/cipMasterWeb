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
