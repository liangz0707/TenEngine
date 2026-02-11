# 020-Pipeline; C++17, contract-first (specs/_contracts/020-pipeline-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_pipeline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 避免依赖链中 FetchContent 在配置阶段执行 git update（无 origin 时报错）
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Skip FetchContent update step")

set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

set(MY_DEPS "")
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# 009/019 目录名与模块 id 不一致，需显式指定
if(NOT DEFINED TENENGINE_009_RENDERCORE_DIR)
  if(DEFINED TENENGINE_ROOT)
    set(TENENGINE_009_RENDERCORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-009-render-core")
  else()
    set(TENENGINE_009_RENDERCORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-009-render-core")
  endif()
endif()
if(NOT DEFINED TENENGINE_019_PIPELINECORE_DIR)
  if(DEFINED TENENGINE_ROOT)
    set(TENENGINE_019_PIPELINECORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-019-pipeline-core")
  else()
    set(TENENGINE_019_PIPELINECORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-019-pipeline-core")
  endif()
endif()
if(EXISTS "${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake")
  include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
  tenengine_resolve_my_dependencies("020-pipeline" OUT_DEPS MY_DEPS)
endif()
if(NOT MY_DEPS)
  message(FATAL_ERROR "te_pipeline: Upstream (001-core, 004-scene, 005-entity, 029-world, 019-pipelinecore, 009-rendercore, 010-shader, 011-material, 012-mesh, 028-texture, 013-resource, 008-rhi) required. TenEngineHelpers not found or deps unresolved. Set TENENGINE_ROOT.")
endif()

set(TE_PIPELINE_SOURCES
  src/RenderPipeline.cpp
  src/PipelineImpl.cpp
  src/RenderableCollector.cpp
)

set(TE_PIPELINE_HEADERS
  include/te/pipeline/FrameContext.h
  include/te/pipeline/RenderingConfig.h
  include/te/pipeline/RenderPipeline.h
  include/te/pipeline/detail/PipelineImpl.h
  include/te/pipeline/detail/RenderableCollector.h
)

add_library(te_pipeline STATIC ${TE_PIPELINE_SOURCES} ${TE_PIPELINE_HEADERS})
target_include_directories(te_pipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(te_pipeline PUBLIC ${MY_DEPS})

source_group("Header Files" FILES ${TE_PIPELINE_HEADERS})
source_group("Header Files\\Detail" FILES
  include/te/pipeline/detail/PipelineImpl.h
  include/te/pipeline/detail/RenderableCollector.h
)
set_target_properties(te_pipeline PROPERTIES PUBLIC_HEADER "${TE_PIPELINE_HEADERS}")

option(BUILD_TESTS "Build unit tests" ON)
# 作为上游被引用时不加入测试
if(BUILD_TESTS AND NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
