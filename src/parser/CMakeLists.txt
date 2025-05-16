add_library(parser config_parser.cpp)

# ylt depends on frozen
target_link_libraries(parser PUBLIC frozen::frozen frozen::frozen-headers)
# use ylt
target_include_directories(parser PUBLIC ${CMAKE_SOURCE_DIR}/third_party)
# use toml++
target_link_libraries(parser PRIVATE tomlplusplus::tomlplusplus)
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTING)
    # test config_parser.c
    add_executable(config_parser_test config_parser_test.cc)
    target_link_libraries(config_parser_test PRIVATE parser)
    target_link_libraries(config_parser_test PRIVATE doctest::doctest)

    # copy real broker.toml
    if(DEFINED ENV{BROKER_PATH})
        set(BROKER_CFG_PATH $ENV{BROKER_PATH})
    else()
        set(BROKER_CFG_PATH "${CMAKE_SOURCE_DIR}/config/broker.toml")
    endif()
    message(STATUS "Using Broker path: ${BROKER_CFG_PATH}")
    
    configure_file(
        "${BROKER_CFG_PATH}"
        "${CMAKE_CURRENT_BINARY_DIR}/broker.toml"
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

    add_test(NAME error_parser_test COMMAND error_parser_test)

    # test struct_parser.hpp
    add_executable(struct_parser_test struct_parser_test.cc)
    target_link_libraries(struct_parser_test PRIVATE parser)
    target_link_libraries(struct_parser_test PRIVATE doctest::doctest)

    add_test(NAME struct_parser_test COMMAND struct_parser_test)
endif()
