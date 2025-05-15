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

add_library(duckdb SHARED IMPORTED)
set_target_properties(duckdb PROPERTIES
  IMPORTED_LOCATION             "${CMAKE_CURRENT_SOURCE_DIR}/third_party/duckdb/lin64/libduckdb.so"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/third_party/duckdb/lin64"
)

include(CTest)
add_subdirectory(src/parser)
add_subdirectory(src/operator)
add_subdirectory(src/channel)

add_subdirectory(src/ctpMd)
add_subdirectory(src/ctpTd)
add_subdirectory(src/data)