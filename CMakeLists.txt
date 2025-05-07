cmake_minimum_required(VERSION 3.30)

# must come *before* project()
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(VCPKG_OVERLAY_PORTS ${CMAKE_SOURCE_DIR}/overlays CACHE STRING "Vcpkg skipped ports")

project(futrader VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(tomlplusplus CONFIG REQUIRED)
find_path(DYLIB_INCLUDE_DIRS "dylib.hpp")

add_library(mdclient src/md/mdclient.cpp)
target_include_directories(mdclient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ctp/lin64)
target_link_libraries(mdclient PRIVATE tomlplusplus::tomlplusplus)
target_include_directories(mdclient PRIVATE ${DYLIB_INCLUDE_DIRS})

add_library(tdclient src/td/tdclient.cpp)
target_include_directories(tdclient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ctp/lin64)
target_link_libraries(tdclient PRIVATE tomlplusplus::tomlplusplus)
target_include_directories(tdclient PRIVATE ${DYLIB_INCLUDE_DIRS})

add_executable(futrader main.cpp)

target_include_directories(futrader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/md)
target_include_directories(futrader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/td)
target_link_libraries(futrader PRIVATE mdclient)
target_link_libraries(futrader PRIVATE tdclient)