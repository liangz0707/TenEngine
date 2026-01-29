# Unit tests for 001-Core (contract: specs/_contracts/001-core-public-api.md).
add_executable(test_alloc unit/test_alloc.cpp)
target_link_libraries(test_alloc PRIVATE te_core)
add_test(NAME test_alloc COMMAND test_alloc)
