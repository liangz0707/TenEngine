# Integration tests for 012-Mesh module

enable_testing()

# Include TenEngineHelpers for test functions
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# Test mesh import
add_executable(test_mesh_import test_mesh_import.cpp)
target_link_libraries(test_mesh_import PRIVATE te_mesh)
if(TARGET te_resource)
  target_link_libraries(test_mesh_import PRIVATE te_resource)
endif()
if(TARGET te_object)
  target_link_libraries(test_mesh_import PRIVATE te_object)
endif()
if(TARGET te_core)
  target_link_libraries(test_mesh_import PRIVATE te_core)
endif()
add_test(NAME test_mesh_import COMMAND test_mesh_import)

# Test mesh load/unload
add_executable(test_mesh_load_unload test_mesh_load_unload.cpp)
target_link_libraries(test_mesh_load_unload PRIVATE te_mesh)
if(TARGET te_resource)
  target_link_libraries(test_mesh_load_unload PRIVATE te_resource)
endif()
if(TARGET te_object)
  target_link_libraries(test_mesh_load_unload PRIVATE te_object)
endif()
if(TARGET te_core)
  target_link_libraries(test_mesh_load_unload PRIVATE te_core)
endif()
if(TARGET te_deviceresource)
  target_link_libraries(test_mesh_load_unload PRIVATE te_deviceresource)
endif()
if(TARGET te_rendercore)
  target_link_libraries(test_mesh_load_unload PRIVATE te_rendercore)
endif()
if(TARGET te_rhi)
  target_link_libraries(test_mesh_load_unload PRIVATE te_rhi)
endif()
add_test(NAME test_mesh_load_unload COMMAND test_mesh_load_unload)

# Create test_data directory in build directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_data)
