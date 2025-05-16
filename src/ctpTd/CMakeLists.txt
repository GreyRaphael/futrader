add_library(ctp_td ctp_td.cpp)

# only use ctp headers
target_include_directories(ctp_td PUBLIC ${CMAKE_SOURCE_DIR}/third_party/ctp/lin64)
# public current library headers
target_include_directories(ctp_td PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# use dylib
target_include_directories(ctp_td PUBLIC ${DYLIB_INCLUDE_DIRS})
# use parser
target_link_libraries(ctp_td PUBLIC parser)

if(BUILD_TESTING)
    add_executable(ctp_td_test ctp_td_test.cc)
    target_link_libraries(ctp_td_test PRIVATE ctp_td)
    target_link_libraries(ctp_td_test PRIVATE doctest::doctest)

    # copy *.so
    configure_file(
        # input
        "${CMAKE_SOURCE_DIR}/third_party/ctp/lin64/thosttraderapi_se.so"
        # output
        "${CMAKE_CURRENT_BINARY_DIR}/ctp/thosttraderapi_se.so"
        # mode
        COPYONLY
    )
    configure_file(
        "${CMAKE_SOURCE_DIR}/third_party/openctp/lin64/thosttraderapi_se.so"
        "${CMAKE_CURRENT_BINARY_DIR}/tts/thosttraderapi_se.so"
        COPYONLY
    )

    # copy errors.toml
    configure_file(
        "${CMAKE_SOURCE_DIR}/config/errors.toml"
        "${CMAKE_CURRENT_BINARY_DIR}/errors.toml"
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

    add_test(NAME ctp_td_test COMMAND ctp_td_test)
endif()
