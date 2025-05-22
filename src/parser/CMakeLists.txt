add_library(parser config_parser.cpp)

# ylt depends on frozen
target_link_libraries(parser PUBLIC frozen::frozen frozen::frozen-headers)
# use ylt
target_include_directories(parser PUBLIC ${CMAKE_SOURCE_DIR}/third_party)
# use toml++
target_link_libraries(parser PRIVATE tomlplusplus::tomlplusplus)
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# use ctp headers
target_include_directories(parser PUBLIC ${CMAKE_SOURCE_DIR}/third_party/ctp)
target_include_directories(parser PUBLIC ${CMAKE_SOURCE_DIR}/src/common)

if(BUILD_TESTING)
    # test config_parser.c
    add_executable(config_parser_test config_parser_test.cc)
    target_link_libraries(config_parser_test PRIVATE parser)
    target_link_libraries(config_parser_test PRIVATE doctest::doctest)

    # copy brokers tomls   
    configure_file(
        "${CMAKE_SOURCE_DIR}/config/brokers/ctp.toml"
        "${CMAKE_CURRENT_BINARY_DIR}/ctp.toml"
        COPYONLY
    )
    configure_file(
        "${CMAKE_SOURCE_DIR}/config/brokers/tts.toml"
        "${CMAKE_CURRENT_BINARY_DIR}/tts.toml"
        COPYONLY
    )

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

    # copy errors.toml
    configure_file(
        "${CMAKE_SOURCE_DIR}/config/errors.toml"
        "${CMAKE_CURRENT_BINARY_DIR}/errors.toml"
        COPYONLY
    )
    # copy nng.toml
    configure_file(
        "${CMAKE_SOURCE_DIR}/config/nng.toml"
        "${CMAKE_CURRENT_BINARY_DIR}/nng.toml"
        COPYONLY
    )

    add_test(NAME error_parser_test COMMAND error_parser_test)

    # test struct_parser.hpp
    add_executable(struct_parser_test struct_parser_test.cc)
    target_link_libraries(struct_parser_test PRIVATE parser)
    target_link_libraries(struct_parser_test PRIVATE doctest::doctest)

    add_test(NAME struct_parser_test COMMAND struct_parser_test)

    # test tick_parser.hpp
    add_executable(tick_parser_test tick_parser_test.cc)
    target_link_libraries(tick_parser_test PRIVATE parser)
    target_link_libraries(tick_parser_test PRIVATE doctest::doctest)

    add_test(NAME tick_parser_test COMMAND tick_parser_test)
endif()
