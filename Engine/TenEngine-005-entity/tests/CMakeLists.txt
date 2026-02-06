# Tests for 005-Entity module

cmake_minimum_required(VERSION 3.16)
project(te_entity_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Test executable
add_executable(te_entity_tests
  unit/main.cpp
  unit/test_entity.cpp
  unit/test_component.cpp
  unit/test_component_query.cpp
  unit/test_entity_manager.cpp
)

target_include_directories(te_entity_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(te_entity_tests PRIVATE
  te_entity
  te_scene
  te_object
  te_core
)

# Set output directory
set_target_properties(te_entity_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
