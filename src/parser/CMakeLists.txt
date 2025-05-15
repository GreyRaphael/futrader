add_library(parser config_parser.cc)

target_include_directories(parser PUBLIC ${CMAKE_SOURCE_DIR}/third_party)
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(parser PRIVATE tomlplusplus::tomlplusplus)

if(BUILD_TESTING)
    # test config_parser.c
    add_executable(config_parser_test config_parser_test.cc)
    target_link_libraries(config_parser_test PRIVATE parser)
    target_link_libraries(config_parser_test PRIVATE doctest::doctest)

    add_test(NAME config_parser_test COMMAND config_parser_test)

    # test string_parser.hpp
    add_executable(string_parser_test string_parser_test.cc)
    target_link_libraries(string_parser_test PRIVATE parser)
    target_link_libraries(string_parser_test PRIVATE doctest::doctest)

    add_test(NAME string_parser_test COMMAND string_parser_test)

    # test error_parser.hpp
    add_executable(error_parser_test error_parser_test.cc)
    target_link_libraries(error_parser_test PRIVATE parser)
    target_link_libraries(error_parser_test PRIVATE doctest::doctest)

    add_test(NAME error_parser_test COMMAND error_parser_test)
endif()
