cmake_minimum_required(VERSION 3.30)

# must come *before* project()
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(VCPKG_OVERLAY_PORTS ${CMAKE_SOURCE_DIR}/overlays CACHE STRING "Vcpkg skipped ports")

project(futrader VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(doctest CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)
find_path(DYLIB_INCLUDE_DIRS "dylib.hpp")
find_package(frozen CONFIG REQUIRED)
find_package(ZeroMQ CONFIG REQUIRED)

add_library(duckdb SHARED IMPORTED)
set_target_properties(duckdb PROPERTIES
  IMPORTED_LOCATION             "${CMAKE_SOURCE_DIR}/third_party/duckdb/libduckdb.so"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/third_party/duckdb"
)

include(CTest)
# binary path is second param, so it go to build/parser
add_subdirectory(src/parser parser)
add_subdirectory(src/operator operator)
add_subdirectory(src/channel channel)

add_subdirectory(src/ctp_md ctp_md)
add_subdirectory(src/bt_md bt_md)
add_subdirectory(src/ctp_td ctp_td)
# add_subdirectory(src/data data)