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

# add_library(parser src/parser/config.cc)
# target_link_libraries(mdclient PRIVATE tomlplusplus::tomlplusplus)
include(CTest)
add_subdirectory(src/parser)
add_subdirectory(src/operator)
add_subdirectory(src/channel)


# add_library(duckdb SHARED IMPORTED)
# set_target_properties(duckdb PROPERTIES
#   IMPORTED_LOCATION             "${CMAKE_CURRENT_SOURCE_DIR}/third_party/duckdb/lin64/libduckdb.so"
#   INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/third_party/duckdb/lin64"
# )

# add_library(datareader src/datafeed/reader.cpp)
# target_link_libraries(datareader PRIVATE duckdb)

# add_library(mdclient src/ctpMd/mdclient.cpp)
# target_include_directories(mdclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ctp/lin64)
# target_link_libraries(mdclient PRIVATE frozen::frozen frozen::frozen-headers)
# target_include_directories(mdclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
# target_link_libraries(mdclient PRIVATE tomlplusplus::tomlplusplus)
# target_include_directories(mdclient PUBLIC ${DYLIB_INCLUDE_DIRS})

# add_library(tdclient src/ctpTrader/tdclient.cpp)
# target_include_directories(tdclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ctp/lin64)
# target_link_libraries(tdclient PRIVATE frozen::frozen frozen::frozen-headers)
# target_include_directories(tdclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
# target_link_libraries(tdclient PRIVATE tomlplusplus::tomlplusplus)
# target_include_directories(tdclient PUBLIC ${DYLIB_INCLUDE_DIRS})

# add_executable(futrader main.cpp)

# target_include_directories(futrader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ctpMd)
# target_include_directories(futrader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ctpTrader)
# target_link_libraries(futrader PRIVATE mdclient)
# target_link_libraries(futrader PRIVATE tdclient)
