add_library(ctp_td ctp_td.cpp)

target_include_directories(ctp_td PUBLIC ${CMAKE_SOURCE_DIR}/third_party/ctp/lin64)
target_include_directories(ctp_td PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ctp_td PUBLIC ${DYLIB_INCLUDE_DIRS})
target_link_libraries(ctp_td PUBLIC parser)

if(BUILD_TESTING)
    add_executable(ctp_td_test ctp_td_test.cc)
    target_link_libraries(ctp_td_test PRIVATE ctp_td)
    target_link_libraries(ctp_td_test PRIVATE doctest::doctest)

    add_test(NAME ctp_td_test COMMAND ctp_td_test)
endif()
