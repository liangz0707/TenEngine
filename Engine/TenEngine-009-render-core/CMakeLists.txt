# 009-RenderCore library; C++17, contract-first (specs/_contracts/009-rendercore-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_rendercore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

set(MY_DEPS "")
if(NOT DEFINED TENENGINE_CMAKE_DIR)
  set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake")
endif()
if(EXISTS "${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake")
  include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
  tenengine_resolve_my_dependencies("009-rendercore" OUT_DEPS MY_DEPS)
endif()
if(NOT MY_DEPS)
  message(FATAL_ERROR "te_rendercore: Upstream (001-core, 008-rhi) required. "
    "TenEngineHelpers not found or deps unresolved. Set TENENGINE_ROOT or TENENGINE_001_CORE_DIR/TENENGINE_008_RHI_DIR.")
endif()

set(TE_RENDERCORE_SOURCES
  src/render_core/pass_protocol.cpp
  src/render_core/resource_desc.cpp
  src/render_core/uniform_buffer.cpp
  src/render_core/uniform_layout.cpp
  src/rendercore/PipelineStateDesc.cpp
  src/rendercore/RenderPipelineStateImpl.cpp
  src/rendercore/ShaderEntryImpl.cpp
  src/rendercore/ShadingStateImpl.cpp
  src/rendercore/RenderMaterialImpl.cpp
  src/rendercore/RenderMeshImpl.cpp
  src/rendercore/RenderElementImpl.cpp
  src/rendercore/RenderResourceFactory.cpp
)

set(TE_RENDERCORE_HEADERS
  include/te/rendercore/api.hpp
  include/te/rendercore/pass_protocol.hpp
  include/te/rendercore/resource_desc.hpp
  include/te/rendercore/shader_reflection.hpp
  include/te/rendercore/types.hpp
  include/te/rendercore/uniform_buffer.hpp
  include/te/rendercore/uniform_layout.hpp
  include/te/rendercore/PipelineStateDesc.hpp
  include/te/rendercore/IRenderPipelineState.hpp
  include/te/rendercore/IShaderEntry.hpp
  include/te/rendercore/IShadingState.hpp
  include/te/rendercore/IRenderMaterial.hpp
  include/te/rendercore/IRenderMesh.hpp
  include/te/rendercore/IRenderElement.hpp
  include/te/rendercore/IRenderTexture.hpp
  include/te/rendercore/ShaderBytecodeDesc.hpp
  include/te/rendercore/impl/RenderPipelineStateImpl.hpp
  include/te/rendercore/impl/ShaderEntryImpl.hpp
  include/te/rendercore/impl/ShadingStateImpl.hpp
  include/te/rendercore/impl/RenderMaterialImpl.hpp
  include/te/rendercore/impl/RenderMeshImpl.hpp
  include/te/rendercore/impl/RenderElementImpl.hpp
  include/te/rendercore/RenderResourceFactory.hpp
)

add_library(te_rendercore STATIC ${TE_RENDERCORE_SOURCES} ${TE_RENDERCORE_HEADERS})
target_include_directories(te_rendercore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(te_rendercore PUBLIC ${MY_DEPS})

source_group("Header Files" FILES ${TE_RENDERCORE_HEADERS})
set_target_properties(te_rendercore PROPERTIES PUBLIC_HEADER "${TE_RENDERCORE_HEADERS}")

option(BUILD_TESTS "Build unit and contract tests" ON)
# Skip tests if TENENGINE_SKIP_DEPENDENCY_TESTS is set (when used as a dependency)
if(BUILD_TESTS AND NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
