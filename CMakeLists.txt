cmake_minimum_required(VERSION 3.30)

# must come *before* project()
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(VCPKG_OVERLAY_PORTS ${CMAKE_SOURCE_DIR}/overlays CACHE STRING "Vcpkg skipped ports")

project(futrader VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

add_library(mdclient src/md/mdclient.cpp)
# PUBLIC then executable can use the header files
target_include_directories(mdclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ctp/lin64)

add_executable(futrader main.cpp)

find_path(DYLIB_INCLUDE_DIRS "dylib.hpp")
target_include_directories(futrader PRIVATE ${DYLIB_INCLUDE_DIRS})

find_package(tomlplusplus CONFIG REQUIRED)
target_link_libraries(futrader PRIVATE tomlplusplus::tomlplusplus)

target_include_directories(futrader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/md)
target_link_libraries(futrader PRIVATE mdclient)