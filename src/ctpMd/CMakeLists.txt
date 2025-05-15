add_library(ctp_md ctp_md.cpp)

target_include_directories(ctp_md PUBLIC ${CMAKE_SOURCE_DIR}/third_party/ctp/lin64)
target_include_directories(ctp_md PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ctp_md PUBLIC ${DYLIB_INCLUDE_DIRS})
target_link_libraries(ctp_md PUBLIC channel)
target_link_libraries(ctp_md PUBLIC parser)

if(BUILD_TESTING)
    add_executable(ctp_md_test ctp_md_test.cc)
    target_link_libraries(ctp_md_test PRIVATE ctp_md)
    target_link_libraries(ctp_md_test PRIVATE doctest::doctest)

    add_test(NAME ctp_md_test COMMAND ctp_md_test)
endif()
