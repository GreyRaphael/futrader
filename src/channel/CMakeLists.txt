add_library(channel INTERFACE)

target_include_directories(channel INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTING)
    # test bounded SPSC channel
    add_executable(spsc_test spsc_test.cc)
    target_link_libraries(spsc_test PRIVATE channel)
    target_link_libraries(spsc_test PRIVATE doctest::doctest)
    
    add_test(NAME spsc_test COMMAND spsc_test)
    
    # test bounded MPSC channel
    add_executable(mpsc_test mpsc_test.cc)
    target_link_libraries(mpsc_test PRIVATE channel)
    target_link_libraries(mpsc_test PRIVATE doctest::doctest)

    add_test(NAME mpsc_test COMMAND mpsc_test)
endif()
