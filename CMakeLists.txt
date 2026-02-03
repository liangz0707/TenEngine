# 019-PipelineCore; C++17, contract-first (specs/_contracts/019-pipelinecore-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_pipelinecore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建根目录：worktree 根（TenEngine-019-pipeline-core）。各依赖按源码引入。
set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

set(MY_DEPS "")
if(NOT DEFINED TENENGINE_CMAKE_DIR)
  set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
endif()
if(EXISTS "${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake")
  include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
  tenengine_resolve_my_dependencies("019-pipelinecore" OUT_DEPS MY_DEPS)
endif()
if(NOT MY_DEPS)
  message(FATAL_ERROR "te_pipelinecore: Upstream (008-rhi, 009-rendercore) required. "
    "TenEngineHelpers not found or deps unresolved. Set TENENGINE_ROOT or TENENGINE_008_RHI_DIR/TENENGINE_009_RENDERCORE_DIR.")
endif()

set(TE_PIPELINECORE_SOURCES
  src/FrameGraph.cpp
  src/LogicalPipeline.cpp
  src/RenderItem.cpp
  src/CollectPass.cpp
  src/LogicalCommandBuffer.cpp
  src/Profiling.cpp
)

add_library(te_pipelinecore STATIC ${TE_PIPELINECORE_SOURCES})
target_include_directories(te_pipelinecore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(te_pipelinecore PUBLIC ${MY_DEPS})

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
