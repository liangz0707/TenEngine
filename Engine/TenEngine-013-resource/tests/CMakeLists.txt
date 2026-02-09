# Unit tests for 013-Resource module (contract: specs/_contracts/013-resource-ABI.md).
# Note: Only 013-Resource module tests are included here. 
# Upstream module tests (001-core, 004-scene, etc.) should not be included per build convention.

enable_testing()

# Test ResourceId
add_executable(test_resource_id unit/test_resource_id.cpp)
target_link_libraries(test_resource_id PRIVATE te_resource te_object)
add_test(NAME test_resource_id COMMAND test_resource_id)

# Test ResourceManager
add_executable(test_resource_manager unit/test_resource_manager.cpp)
target_link_libraries(test_resource_manager PRIVATE te_resource te_object)
add_test(NAME test_resource_manager COMMAND test_resource_manager)

# Test IResource base class
add_executable(test_resource unit/test_resource.cpp)
target_link_libraries(test_resource PRIVATE te_resource te_object)
add_test(NAME test_resource COMMAND test_resource)
