add_library(channel INTERFACE)

target_include_directories(channel INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTING)
    add_executable(channel_test spsc_test.cc)
    target_link_libraries(channel_test PRIVATE channel)
    target_link_libraries(channel_test PRIVATE doctest::doctest)

    add_test(NAME channel_test COMMAND channel_test)
endif()
