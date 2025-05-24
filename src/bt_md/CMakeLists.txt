add_library(bt_md INTERFACE)

# public current library headers
target_include_directories(bt_md INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
# use duckdb
target_link_libraries(bt_md INTERFACE duckdb)
# use parsers
target_link_libraries(bt_md INTERFACE parser)

if(BUILD_TESTING)
    add_executable(bt_md_test bt_md_test.cc)
    target_link_libraries(bt_md_test PRIVATE bt_md)
    target_link_libraries(bt_md_test PRIVATE doctest::doctest)
    # use SPSC
    target_link_libraries(bt_md_test PRIVATE channel)

    # copy bt.toml
    configure_file(
        "${CMAKE_SOURCE_DIR}/config/bt.toml"
        "${CMAKE_CURRENT_BINARY_DIR}/bt.toml"
        COPYONLY
    )

    add_test(NAME bt_md_test COMMAND bt_md_test)
endif()

# publish bt tick to nng
add_executable(bt_pub bt_pub.cpp)
target_link_libraries(bt_pub PRIVATE bt_md)
target_link_libraries(bt_pub PRIVATE libzmq)

# copy nng.toml
configure_file(
    "${CMAKE_SOURCE_DIR}/config/nng.toml"
    "${CMAKE_CURRENT_BINARY_DIR}/nng.toml"
    COPYONLY
)