# 004-Scene module (TenEngine). Contract: specs/_contracts/004-scene-public-api.md
# Dependencies: 001-core, 002-object (per plan.md dependency introduction: source)
cmake_minimum_required(VERSION 3.16)
project(TenEngine-004-scene LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "TenEngine helpers")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
tenengine_resolve_my_dependencies("004-scene" OUT_DEPS MY_DEPS)

add_library(te_scene STATIC
  src/scene/world.cpp
  src/scene/scene_graph.cpp
  src/scene/hierarchy.cpp
  src/scene/level.cpp
)
target_include_directories(te_scene PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(te_scene PRIVATE ${MY_DEPS})

tenengine_add_module_test(NAME te_scene_test MODULE_TARGET te_scene
  SOURCES tests/scene/unit/test_scene_graph.cpp
  ENABLE_CTEST
)
tenengine_add_module_test(NAME te_scene_hierarchy_test MODULE_TARGET te_scene
  SOURCES tests/scene/unit/test_hierarchy.cpp
  ENABLE_CTEST
)
