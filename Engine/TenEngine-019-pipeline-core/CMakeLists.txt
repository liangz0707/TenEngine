# 019-PipelineCore; C++17, contract-first (specs/_contracts/019-pipelinecore-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_pipelinecore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

set(MY_DEPS "")
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(_ENGINE "${CMAKE_CURRENT_SOURCE_DIR}/..")
if(NOT DEFINED TENENGINE_008_RHI_DIR)
  set(TENENGINE_008_RHI_DIR "${_ENGINE}/TenEngine-008-rhi")
endif()
if(NOT DEFINED TENENGINE_009_RENDERCORE_DIR)
  set(TENENGINE_009_RENDERCORE_DIR "${_ENGINE}/TenEngine-009-render-core")
endif()
if(NOT DEFINED TENENGINE_011_MATERIAL_DIR)
  set(TENENGINE_011_MATERIAL_DIR "${_ENGINE}/TenEngine-011-material")
endif()
if(NOT DEFINED TENENGINE_012_MESH_DIR)
  set(TENENGINE_012_MESH_DIR "${_ENGINE}/TenEngine-012-mesh")
endif()
if(NOT DEFINED TENENGINE_013_RESOURCE_DIR)
  set(TENENGINE_013_RESOURCE_DIR "${_ENGINE}/TenEngine-013-resource")
endif()
if(NOT DEFINED TENENGINE_002_OBJECT_DIR)
  set(TENENGINE_002_OBJECT_DIR "${_ENGINE}/TenEngine-002-object")
endif()
if(EXISTS "${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake")
  include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
  tenengine_resolve_my_dependencies("019-pipelinecore" OUT_DEPS MY_DEPS)
endif()
if(NOT MY_DEPS)
  message(FATAL_ERROR "te_pipelinecore: Upstream (008-rhi, 009-rendercore, 011-material, 012-mesh) required. "
    "TenEngineHelpers not found or deps unresolved. Set TENENGINE_ROOT.")
endif()

set(TE_PIPELINECORE_SOURCES
  src/FrameGraph.cpp
  src/LogicalPipeline.cpp
  src/RenderItem.cpp
  src/CollectPass.cpp
  src/LogicalCommandBuffer.cpp
  src/Profiling.cpp
)

set(TE_PIPELINECORE_HEADERS
  include/te/pipelinecore/CollectPass.h
  include/te/pipelinecore/Config.h
  include/te/pipelinecore/FrameContext.h
  include/te/pipelinecore/FrameGraph.h
  include/te/pipelinecore/LogicalCommandBuffer.h
  include/te/pipelinecore/LogicalPipeline.h
  include/te/pipelinecore/Profiling.h
  include/te/pipelinecore/RenderItem.h
)

add_library(te_pipelinecore STATIC ${TE_PIPELINECORE_SOURCES} ${TE_PIPELINECORE_HEADERS})
target_include_directories(te_pipelinecore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(te_pipelinecore PUBLIC ${MY_DEPS})

source_group("Header Files" FILES ${TE_PIPELINECORE_HEADERS})
set_target_properties(te_pipelinecore PROPERTIES PUBLIC_HEADER "${TE_PIPELINECORE_HEADERS}")

# 显式添加 011/012 的 include，确保编译 019 源文件时能找到 te/material/*、te/mesh/*（作为上游被 020 等引用时 INTERFACE 传播可能未生效）
if(EXISTS "${TENENGINE_011_MATERIAL_DIR}/include")
  target_include_directories(te_pipelinecore PRIVATE "${TENENGINE_011_MATERIAL_DIR}/include")
endif()
if(EXISTS "${TENENGINE_012_MESH_DIR}/include")
  target_include_directories(te_pipelinecore PRIVATE "${TENENGINE_012_MESH_DIR}/include")
endif()
if(EXISTS "${TENENGINE_013_RESOURCE_DIR}/include")
  target_include_directories(te_pipelinecore PRIVATE "${TENENGINE_013_RESOURCE_DIR}/include")
endif()
if(EXISTS "${TENENGINE_002_OBJECT_DIR}/include")
  target_include_directories(te_pipelinecore PRIVATE "${TENENGINE_002_OBJECT_DIR}/include")
endif()

option(BUILD_TESTS "Build unit tests" ON)
# 作为上游被引用时不加入测试
if(BUILD_TESTS AND NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
