add_library(operator INTERFACE)

target_include_directories(operator INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTING)
    add_executable(operator_test rolling_test.cc)
    target_link_libraries(operator_test PRIVATE operator)
    target_link_libraries(operator_test PRIVATE doctest::doctest)

    add_test(NAME operator_test COMMAND operator_test)
endif()
