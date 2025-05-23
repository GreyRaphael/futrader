# prebuilt not like lockfree::SPSC(only as header-only)
add_library(ctp_md ctp_md.hpp)

# public current library headers
target_include_directories(ctp_md PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# just use ctp headers
target_include_directories(ctp_md PRIVATE ${CMAKE_SOURCE_DIR}/third_party/ctp)
# use dylib
target_include_directories(ctp_md PRIVATE ${DYLIB_INCLUDE_DIRS})
# use parsers
target_link_libraries(ctp_md PUBLIC parser)

if(BUILD_TESTING)
    add_executable(ctp_md_test ctp_md_test.cc)
    target_link_libraries(ctp_md_test PRIVATE ctp_md)
    target_link_libraries(ctp_md_test PRIVATE doctest::doctest)
    # use SPSC
    target_link_libraries(ctp_md_test PRIVATE channel)

    # copy *.so
    configure_file(
        # input
        "${CMAKE_SOURCE_DIR}/third_party/ctp/thostmduserapi_se.so"
        # output
        "${CMAKE_CURRENT_BINARY_DIR}/ctp/thostmduserapi_se.so"
        # mode
        COPYONLY
    )
    configure_file(
        "${CMAKE_SOURCE_DIR}/third_party/tts/thostmduserapi_se.so"
        "${CMAKE_CURRENT_BINARY_DIR}/tts/thostmduserapi_se.so"
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

    add_test(NAME ctp_md_test COMMAND ctp_md_test)
endif()
