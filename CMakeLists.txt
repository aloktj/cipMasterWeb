cmake_minimum_required(VERSION 3.16)
project(cipMasterWeb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Allow find_package to pick up custom modules shipped with this repo.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

add_subdirectory(tests)
