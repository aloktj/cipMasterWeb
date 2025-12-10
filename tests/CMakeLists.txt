find_package(Drogon CONFIG REQUIRED)
find_package(EIPScanner REQUIRED)

add_executable(library_detection
  library_detection.cpp
)

target_compile_features(library_detection PRIVATE cxx_std_17)

target_link_libraries(library_detection PRIVATE
  Drogon::Drogon
  EIPScanner::EIPScanner
)
