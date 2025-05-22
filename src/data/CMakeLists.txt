add_library(duck_loader duck_loader.cpp)
add_executable(nng_pub nng_pub.cpp)
add_executable(duck_pub duck_pub.cpp)

target_include_directories(duck_loader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# link 3rd duckdb so
target_link_libraries(duck_loader PRIVATE duckdb)
target_link_libraries(duck_loader PUBLIC parser)
target_link_libraries(duck_loader PUBLIC channel)

# publish CThostFtdcDepthMarketDataField to nng
target_link_libraries(nng_pub PRIVATE ctp_md)
target_link_libraries(nng_pub PRIVATE nng::nng)

# publish duckdb struct to nng
target_link_libraries(duck_pub PRIVATE duck_loader)
target_link_libraries(duck_pub PRIVATE nng::nng)

# copy *.so
configure_file(
    # input
    "${CMAKE_SOURCE_DIR}/third_party/ctp/lin64/thostmduserapi_se.so"
    # output
    "${CMAKE_CURRENT_BINARY_DIR}/ctp/thostmduserapi_se.so"
    # mode
    COPYONLY
)
configure_file(
    "${CMAKE_SOURCE_DIR}/third_party/tts/lin64/thostmduserapi_se.so"
    "${CMAKE_CURRENT_BINARY_DIR}/tts/thostmduserapi_se.so"
    COPYONLY
)

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

# copy duckdb.toml
configure_file(
    "${CMAKE_SOURCE_DIR}/config/duckdb.toml"
    "${CMAKE_CURRENT_BINARY_DIR}/duckdb.toml"
    COPYONLY
)

# copy broker tomls
if(DEFINED ENV{BROKERS_DIR})
    set(BROKERS_CFG_DIR $ENV{BROKERS_DIR})
else()
    set(BROKERS_CFG_DIR "${CMAKE_SOURCE_DIR}/config/brokers")
endif()
message(STATUS "Using Brokers path: ${BROKERS_CFG_DIR}")

file(GLOB BROKER_TOMLS "${BROKERS_CFG_DIR}/*.toml")
foreach(broker_toml IN LISTS BROKER_TOMLS)
    # extract filename only (no path)
    get_filename_component(fname "${broker_toml}" NAME)
    configure_file(
        "${broker_toml}"                            # input
        "${CMAKE_CURRENT_BINARY_DIR}/${fname}"  # output
        COPYONLY
    )
endforeach()


if(BUILD_TESTING)
    add_executable(duck_loader_test duck_loader_test.cc)
    target_link_libraries(duck_loader_test PRIVATE duck_loader)
    target_link_libraries(duck_loader_test PRIVATE doctest::doctest)

    add_test(NAME duck_loader_test COMMAND duck_loader_test)
endif()
