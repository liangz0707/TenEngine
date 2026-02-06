# 013-Resource tests; link te_resource only (te_core via dependency).
# Include path for 001-core (FileRead etc.) when test uses 001 API.
set(TENENGINE_001_CORE_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../../TenEngine-001-core/include")

add_executable(test_resource_contract contract/test_abi_contract.cpp)
target_link_libraries(test_resource_contract PRIVATE te_resource)
add_test(NAME test_resource_contract COMMAND test_resource_contract)

add_executable(test_resource_unit unit/test_resource_id_and_cache.cpp)
target_include_directories(test_resource_unit PRIVATE ${TENENGINE_001_CORE_INCLUDE})
target_link_libraries(test_resource_unit PRIVATE te_resource)
add_test(NAME test_resource_unit COMMAND test_resource_unit)

add_executable(test_resource_integration integration/test_load_sync_unload.cpp)
target_include_directories(test_resource_integration PRIVATE ${TENENGINE_001_CORE_INCLUDE})
target_link_libraries(test_resource_integration PRIVATE te_resource)
add_test(NAME test_resource_integration COMMAND test_resource_integration)
