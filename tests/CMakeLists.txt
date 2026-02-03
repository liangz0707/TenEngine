# Unit tests for 001-Core (contract: specs/_contracts/001-core-public-api.md).
add_executable(test_alloc unit/test_alloc.cpp)
target_link_libraries(test_alloc PRIVATE te_core)
add_test(NAME test_alloc COMMAND test_alloc)

add_executable(test_thread unit/test_thread.cpp)
target_link_libraries(test_thread PRIVATE te_core)
add_test(NAME test_thread COMMAND test_thread)

add_executable(test_platform unit/test_platform.cpp)
target_link_libraries(test_platform PRIVATE te_core)
add_test(NAME test_platform COMMAND test_platform)

add_executable(test_log unit/test_log.cpp)
target_link_libraries(test_log PRIVATE te_core)
add_test(NAME test_log COMMAND test_log)

add_executable(test_math unit/test_math.cpp)
target_link_libraries(test_math PRIVATE te_core)
add_test(NAME test_math COMMAND test_math)

add_executable(test_containers unit/test_containers.cpp)
target_link_libraries(test_containers PRIVATE te_core)
add_test(NAME test_containers COMMAND test_containers)

add_executable(test_module_load unit/test_module_load.cpp)
target_link_libraries(test_module_load PRIVATE te_core)
add_test(NAME test_module_load COMMAND test_module_load)

add_executable(test_engine unit/test_engine.cpp)
target_link_libraries(test_engine PRIVATE te_core)
add_test(NAME test_engine COMMAND test_engine)

add_executable(test_check unit/test_check.cpp)
target_link_libraries(test_check PRIVATE te_core)
add_test(NAME test_check COMMAND test_check)
