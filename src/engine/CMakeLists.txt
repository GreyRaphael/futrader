add_library(engine_cta INTERFACE)

# public current library headers
target_include_directories(engine_cta INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
# use parsers
target_link_libraries(engine_cta INTERFACE parser)
# stra_cta
target_link_libraries(engine_cta INTERFACE stra_cta)
# use parallel_hashmap
target_include_directories(engine_cta INTERFACE ${PARALLEL_HASHMAP_INCLUDE_DIRS})
# use zmq
target_link_libraries(engine_cta INTERFACE libzmq)
# use taskflow
target_link_libraries(engine_cta INTERFACE Taskflow::Taskflow)
# use spsc
target_link_libraries(engine_cta INTERFACE channel)

if(BUILD_TESTING)
    add_executable(engine_cta_test engine_cta_test.cc)
    target_link_libraries(engine_cta_test PRIVATE engine_cta)
    target_link_libraries(engine_cta_test PRIVATE doctest::doctest)

    add_test(NAME engine_cta_test COMMAND engine_cta_test)
endif()