# Unit tests for 012-Mesh module

# Include TenEngineHelpers for test functions
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# Test mesh API
tenengine_add_module_test(
  NAME test_mesh_api
  MODULE_TARGET te_mesh
  SOURCES test_mesh_api.cpp
  ENABLE_CTEST
)

# Test mesh resource
tenengine_add_module_test(
  NAME test_mesh_resource
  MODULE_TARGET te_mesh
  SOURCES test_mesh_resource.cpp
  ENABLE_CTEST
)

# Test mesh device (GPU resource creation)
tenengine_add_module_test(
  NAME test_mesh_device
  MODULE_TARGET te_mesh
  SOURCES test_mesh_device.cpp
  ENABLE_CTEST
)
