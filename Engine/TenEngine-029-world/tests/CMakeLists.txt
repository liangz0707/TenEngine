# Tests for 029-World module

cmake_minimum_required(VERSION 3.16)
project(te_world_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(te_world_tests
  unit/main.cpp
  unit/test_world_manager.cpp
)

target_include_directories(te_world_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(te_world_tests PRIVATE
  te_world
  te_scene
  te_entity
  te_resource
  te_core
  te_object
)

set_target_properties(te_world_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
