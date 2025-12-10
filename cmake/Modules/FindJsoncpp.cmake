# Minimal FindJsoncpp module with a fallback header for environments where the
# jsoncpp development package is unavailable. This is primarily to satisfy the
# Drogon dependency chain when only the runtime library is present.

set(_jsoncpp_search_paths
  /usr/include
  /usr/include/jsoncpp
  "${CMAKE_CURRENT_LIST_DIR}/../../third_party/jsoncpp"
  "${CMAKE_CURRENT_LIST_DIR}/../third_party/jsoncpp"
  "${CMAKE_CURRENT_LIST_DIR}/third_party/jsoncpp"
)

find_path(JSONCPP_INCLUDE_DIRS
  NAMES json/json.h
  PATHS ${_jsoncpp_search_paths}
)

find_library(JSONCPP_LIBRARIES
  NAMES jsoncpp jsoncpp.so.25 jsoncpp.so.1.9.5
  PATHS /usr/lib /usr/lib/x86_64-linux-gnu /lib /lib/x86_64-linux-gnu
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Jsoncpp
  REQUIRED_VARS JSONCPP_INCLUDE_DIRS JSONCPP_LIBRARIES
)

if(Jsoncpp_FOUND)
  if(NOT TARGET Jsoncpp::Jsoncpp)
    add_library(Jsoncpp::Jsoncpp UNKNOWN IMPORTED)
    set_target_properties(Jsoncpp::Jsoncpp PROPERTIES
      IMPORTED_LOCATION "${JSONCPP_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${JSONCPP_INCLUDE_DIRS}"
    )
  endif()

  if(NOT TARGET Jsoncpp_lib)
    add_library(Jsoncpp_lib INTERFACE IMPORTED)
    set_target_properties(Jsoncpp_lib PROPERTIES
      INTERFACE_LINK_LIBRARIES "${JSONCPP_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${JSONCPP_INCLUDE_DIRS}"
    )
  endif()
endif()
