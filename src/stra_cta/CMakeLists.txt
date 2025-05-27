add_library(stra_cta INTERFACE)

# public current library headers
target_include_directories(stra_cta INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
# use parsers
target_link_libraries(stra_cta INTERFACE parser)
# use operator
target_link_libraries(stra_cta INTERFACE operator)

if(BUILD_TESTING)
    add_executable(stra_cta_test stra_cta_test.cc)
    target_link_libraries(stra_cta_test PRIVATE stra_cta)
    target_link_libraries(stra_cta_test PRIVATE doctest::doctest)

    add_test(NAME stra_cta_test COMMAND stra_cta_test)
endif()