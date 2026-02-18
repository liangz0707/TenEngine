# 020-Pipeline; C++17, thread scheduling only.
cmake_minimum_required(VERSION 3.16)
project(te_pipeline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Skip FetchContent update step")
set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

set(MY_DEPS "")
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(_ENGINE "${CMAKE_CURRENT_SOURCE_DIR}/..")
if(NOT DEFINED TENENGINE_001_CORE_DIR)
  set(TENENGINE_001_CORE_DIR "${_ENGINE}/TenEngine-001-core")
endif()
if(NOT DEFINED TENENGINE_002_OBJECT_DIR)
  set(TENENGINE_002_OBJECT_DIR "${_ENGINE}/TenEngine-002-object")
endif()
if(NOT DEFINED TENENGINE_004_SCENE_DIR)
  set(TENENGINE_004_SCENE_DIR "${_ENGINE}/TenEngine-004-scene")
endif()
if(NOT DEFINED TENENGINE_005_ENTITY_DIR)
  set(TENENGINE_005_ENTITY_DIR "${_ENGINE}/TenEngine-005-entity")
endif()
if(NOT DEFINED TENENGINE_008_RHI_DIR)
  set(TENENGINE_008_RHI_DIR "${_ENGINE}/TenEngine-008-rhi")
endif()
if(NOT DEFINED TENENGINE_009_RENDERCORE_DIR)
  set(TENENGINE_009_RENDERCORE_DIR "${_ENGINE}/TenEngine-009-render-core")
endif()
if(NOT DEFINED TENENGINE_010_SHADER_DIR)
  set(TENENGINE_010_SHADER_DIR "${_ENGINE}/TenEngine-010-shader")
endif()
if(NOT DEFINED TENENGINE_013_RESOURCE_DIR)
  set(TENENGINE_013_RESOURCE_DIR "${_ENGINE}/TenEngine-013-resource")
endif()
if(NOT DEFINED TENENGINE_019_PIPELINECORE_DIR)
  set(TENENGINE_019_PIPELINECORE_DIR "${_ENGINE}/TenEngine-019-pipeline-core")
endif()
if(NOT DEFINED TENENGINE_028_TEXTURE_DIR)
  set(TENENGINE_028_TEXTURE_DIR "${_ENGINE}/TenEngine-028-texture")
endif()
if(NOT DEFINED TENENGINE_029_WORLD_DIR)
  set(TENENGINE_029_WORLD_DIR "${_ENGINE}/TenEngine-029-world")
endif()
if(NOT DEFINED TENENGINE_011_MATERIAL_DIR)
  set(TENENGINE_011_MATERIAL_DIR "${_ENGINE}/TenEngine-011-material")
endif()
if(NOT DEFINED TENENGINE_012_MESH_DIR)
  set(TENENGINE_012_MESH_DIR "${_ENGINE}/TenEngine-012-mesh")
endif()
if(EXISTS "${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake")
  include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
  tenengine_resolve_my_dependencies("020-pipeline" OUT_DEPS MY_DEPS)
endif()
if(NOT MY_DEPS)
  message(FATAL_ERROR "te_pipeline: Upstream (001-core, 004-scene, 005-entity, 029-world, 019-pipelinecore, 009-rendercore, 010-shader, 011-material, 012-mesh, 028-texture, 013-resource, 008-rhi) required. TenEngineHelpers not found or deps unresolved. Set TENENGINE_ROOT.")
endif()

# Skip if already built (e.g. added from root and again via dependency)
if(TARGET te_pipeline)
  return()
endif()

set(TE_PIPELINE_SOURCES
  src/ThreadQueue.cpp
  src/PipelineContext.cpp
  src/RenderPipeline.cpp
  src/BuiltinMeshes.cpp
  src/BuiltinMaterials.cpp
  src/Culling.cpp
  src/RenderableCollector.cpp
  src/PipelineScheduler.cpp
)

set(TE_PIPELINE_HEADERS
  include/te/pipeline/ThreadQueue.h
  include/te/pipeline/RenderingConfig.h
  include/te/pipeline/PipelineContext.h
  include/te/pipeline/RenderPipeline.h
  include/te/pipeline/BuiltinMeshes.h
  include/te/pipeline/BuiltinMaterials.h
  include/te/pipeline/Culling.h
  include/te/pipeline/PipelineScheduler.h
  include/te/pipeline/detail/RenderableCollector.h
)

add_library(te_pipeline STATIC ${TE_PIPELINE_SOURCES} ${TE_PIPELINE_HEADERS})
target_include_directories(te_pipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(te_pipeline PUBLIC ${MY_DEPS})

source_group("Header Files" FILES ${TE_PIPELINE_HEADERS})
set_target_properties(te_pipeline PROPERTIES PUBLIC_HEADER "${TE_PIPELINE_HEADERS}")

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS AND NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
