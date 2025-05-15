add_library(parser config.cc)

target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(parser PRIVATE tomlplusplus::tomlplusplus)

if(BUILD_TESTING)
add_executable(parser_test config_test.cc)
    target_link_libraries(parser_test PRIVATE parser)
    target_link_libraries(parser_test PRIVATE doctest::doctest)
    target_include_directories(parser_test PRIVATE ${CMAKE_SOURCE_DIR}/third_party)

    add_test(NAME parser_test COMMAND parser_test)
endif()
