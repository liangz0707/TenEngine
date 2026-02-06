# Tests for 004-Scene module

cmake_minimum_required(VERSION 3.16)
project(te_scene_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Test executable
add_executable(te_scene_tests
  unit/main.cpp
  unit/test_scene_manager.cpp
  unit/test_scene_world.cpp
  unit/test_spatial_query.cpp
  unit/test_node_managers.cpp
  unit/test_octree.cpp
  unit/test_quadtree.cpp
)

target_include_directories(te_scene_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(te_scene_tests PRIVATE
  te_scene
  te_core
)

# Set output directory
set_target_properties(te_scene_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
