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
find_package(nng CONFIG REQUIRED)

add_library(duckdb SHARED IMPORTED)
set_target_properties(duckdb PROPERTIES
  IMPORTED_LOCATION             "${CMAKE_SOURCE_DIR}/third_party/duckdb/libduckdb.so"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/third_party/duckdb"
)

add_library(ctp_mdapi SHARED IMPORTED)
set_target_properties(ctp_mdapi PROPERTIES
  IMPORTED_LOCATION             "${CMAKE_SOURCE_DIR}/third_party/ctp/thostmduserapi_se.so"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/third_party/ctp"
)

add_library(ctp_tdapi SHARED IMPORTED)
set_target_properties(ctp_tdapi PROPERTIES
  IMPORTED_LOCATION             "${CMAKE_SOURCE_DIR}/third_party/ctp/thosttraderapi_se.so"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/third_party/ctp"
)

add_library(tts_mdapi SHARED IMPORTED)
set_target_properties(tts_mdapi PROPERTIES
IMPORTED_LOCATION             "${CMAKE_SOURCE_DIR}/third_party/tts/thostmduserapi_se.so"
INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/third_party/tts"
)

add_library(tts_tdapi SHARED IMPORTED)
set_target_properties(tts_tdapi PROPERTIES
  IMPORTED_LOCATION             "${CMAKE_SOURCE_DIR}/third_party/tts/thosttraderapi_se.so"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/third_party/tts"
)

include(CTest)
# binary path is second param, so it go to build/parser
# add_subdirectory(src/parser parser)
# add_subdirectory(src/operator operator)
# add_subdirectory(src/channel channel)

# add_subdirectory(src/ctpMd ctpMd)
# add_subdirectory(src/ctpTd ctpTd)
# add_subdirectory(src/data data)