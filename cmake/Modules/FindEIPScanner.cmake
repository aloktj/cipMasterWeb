# Custom CMake find module for EIPScanner
# The Debian package installs headers to /usr/include/EIPScanner
# and the shared library to /usr/lib/libEIPScanner.so

find_path(EIPSCANNER_INCLUDE_DIR
  NAMES SessionInfo.h
  PATH_SUFFIXES EIPScanner
)

find_library(EIPSCANNER_LIBRARY
  NAMES EIPScanner
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EIPScanner
  REQUIRED_VARS EIPSCANNER_LIBRARY EIPSCANNER_INCLUDE_DIR
)

if(EIPSCANNER_FOUND)
  add_library(EIPScanner::EIPScanner UNKNOWN IMPORTED)
  set_target_properties(EIPScanner::EIPScanner PROPERTIES
    IMPORTED_LOCATION "${EIPSCANNER_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${EIPSCANNER_INCLUDE_DIR}"
  )
endif()
