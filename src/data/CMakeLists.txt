add_library(duck_loader duck_loader.cpp)

target_include_directories(duck_loader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(duck_loader PRIVATE duckdb)

if(BUILD_TESTING)
    add_executable(duck_loader_test duck_loader_test.cc)
    target_link_libraries(duck_loader_test PRIVATE duck_loader)
    target_link_libraries(duck_loader_test PRIVATE doctest::doctest)

    add_test(NAME duck_loader_test COMMAND duck_loader_test)
endif()
