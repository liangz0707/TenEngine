# 010-Shader library; C++17, contract-first (specs/_contracts/010-shader-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_shader LANGUAGES CXX)

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
if(NOT DEFINED TENENGINE_008_RHI_DIR)
  set(TENENGINE_008_RHI_DIR "${_ENGINE}/TenEngine-008-rhi")
endif()
if(NOT DEFINED TENENGINE_009_RENDERCORE_DIR)
  set(TENENGINE_009_RENDERCORE_DIR "${_ENGINE}/TenEngine-009-render-core")
endif()
if(NOT DEFINED TENENGINE_013_RESOURCE_DIR)
  set(TENENGINE_013_RESOURCE_DIR "${_ENGINE}/TenEngine-013-resource")
endif()
if(EXISTS "${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake")
  include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
  tenengine_resolve_my_dependencies("010-shader" OUT_DEPS MY_DEPS)
endif()
if(NOT MY_DEPS)
  message(FATAL_ERROR "te_shader: Upstream (001-core, 008-rhi, 009-rendercore, 013-resource, 002-object) required. "
    "TenEngineHelpers not found or deps unresolved. Set TENENGINE_ROOT.")
endif()

# Third-party: vulkan-headers (shared with 008-RHI), glslang, spirv-cross
include(FetchContent)

# Vulkan-Headers: reuse if 008-RHI already fetched (per docs/third_party-integration-workflow.md §7)
if(NOT TARGET Vulkan::Headers)
  FetchContent_Declare(Vulkan-Headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG        v1.3.280
  )
  FetchContent_MakeAvailable(Vulkan-Headers)
endif()

# glslang
set(ENABLE_OPT OFF CACHE BOOL "Disable SPIR-V optimizer (avoids SPIRV-Tools)")
FetchContent_Declare(glslang
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG        15.4.0
)
set(SKIP_GLSLANG_INSTALL OFF CACHE BOOL "")
FetchContent_MakeAvailable(glslang)

# SPIRV-Cross: SPIR-V -> MSL/HLSL (required for Metal/D3D11 paths per plan)
option(TENENGINE_USE_SPIRV_CROSS "Enable SPIRV-Cross for MSL/HLSL" ON)
if(TENENGINE_USE_SPIRV_CROSS)
  FetchContent_Declare(spirv_cross
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
    GIT_TAG        main
  )
  set(SPIRV_CROSS_CLI OFF CACHE BOOL "")
  FetchContent_MakeAvailable(spirv_cross)
endif()

# DXC (D3D12): optional, require vcpkg or find_package when TE_RHI_D3D12
option(TE_RHI_D3D12 "Enable D3D12 backend (HLSL->DXIL via DXC)" OFF)

set(TE_SHADER_SOURCES
  src/ShaderCollection.cpp
  src/ShaderModuleInit.cpp
  src/ShaderResource.cpp
  src/shader/compiler_impl.cpp
  src/shader/factory.cpp
  src/shader/handle_impl.cpp
  src/shader/cache_impl.cpp
  src/shader/hot_reload_impl.cpp
  src/backends/glslang_backend.cpp
  src/backends/spirv_cross_backend.cpp
)
if(TE_RHI_D3D12 AND WIN32)
  list(APPEND TE_SHADER_SOURCES src/backends/dxc_backend.cpp)
endif()

set(TE_SHADER_HEADERS
  include/te/shader/api.hpp
  include/te/shader/cache.hpp
  include/te/shader/compiler.hpp
  include/te/shader/factory.hpp
  include/te/shader/handle.hpp
  include/te/shader/hot_reload.hpp
  include/te/shader/ShaderAssetDesc.h
  include/te/shader/ShaderCollection.h
  include/te/shader/ShaderModuleInit.h
  include/te/shader/ShaderResource.h
  include/te/shader/types.hpp
  include/te/shader/detail/cache_impl.hpp
  include/te/shader/detail/compiler_impl.hpp
  include/te/shader/detail/dxc_backend.hpp
  include/te/shader/detail/glslang_backend.hpp
  include/te/shader/detail/handle_impl.hpp
  include/te/shader/detail/hot_reload_impl.hpp
  include/te/shader/detail/spirv_cross_backend.hpp
)

add_library(te_shader STATIC ${TE_SHADER_SOURCES} ${TE_SHADER_HEADERS})
target_include_directories(te_shader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(te_shader PUBLIC
  ${MY_DEPS}
  Vulkan::Headers
  glslang
  SPIRV
)
if(MY_DEPS)
  target_compile_definitions(te_shader PRIVATE TE_SHADER_USE_CORE=1)
endif()
if(TENENGINE_USE_SPIRV_CROSS)
  target_link_libraries(te_shader PRIVATE spirv-cross-msl spirv-cross-hlsl spirv-cross-glsl)
  target_compile_definitions(te_shader PRIVATE TENENGINE_USE_SPIRV_CROSS=1)
endif()
if(TE_RHI_D3D12 AND WIN32)
  find_package(directx-dxc QUIET)
  if(directx-dxc_FOUND)
    target_link_libraries(te_shader PRIVATE Microsoft::DirectXShaderCompiler)
    target_compile_definitions(te_shader PRIVATE TE_RHI_D3D12=1 TE_SHADER_HAVE_DXC=1)
  endif()
endif()

source_group("Header Files" FILES ${TE_SHADER_HEADERS})
source_group("Header Files\\Detail" FILES
  include/te/shader/detail/cache_impl.hpp
  include/te/shader/detail/compiler_impl.hpp
  include/te/shader/detail/dxc_backend.hpp
  include/te/shader/detail/glslang_backend.hpp
  include/te/shader/detail/handle_impl.hpp
  include/te/shader/detail/hot_reload_impl.hpp
  include/te/shader/detail/spirv_cross_backend.hpp
)
set_target_properties(te_shader PROPERTIES PUBLIC_HEADER "${TE_SHADER_HEADERS}")

# Do not add tests when built as dependency (e.g. by 011-material) to avoid pulling shader tests into material project
if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  tenengine_add_module_test(
    NAME te_shader_compile_test
    MODULE_TARGET te_shader
    SOURCES tests/test_compile.cpp
    ENABLE_CTEST
  )
  tenengine_add_module_test(
    NAME te_shader_variants_test
    MODULE_TARGET te_shader
    SOURCES tests/test_variants.cpp
    ENABLE_CTEST
  )
  tenengine_add_module_test(
    NAME te_shader_cache_test
    MODULE_TARGET te_shader
    SOURCES tests/test_cache.cpp
    ENABLE_CTEST
  )
  tenengine_add_module_test(
    NAME te_shader_hot_reload_test
    MODULE_TARGET te_shader
    SOURCES tests/test_hot_reload.cpp
    ENABLE_CTEST
  )
  tenengine_add_module_test(
    NAME te_shader_hlsl_compile_test
    MODULE_TARGET te_shader
    SOURCES tests/test_hlsl_compile.cpp
    ENABLE_CTEST
  )
  tenengine_add_module_test(
    NAME te_shader_msl_hlsl_source_test
    MODULE_TARGET te_shader
    SOURCES tests/test_msl_hlsl_source.cpp
    ENABLE_CTEST
  )
  tenengine_add_module_test(
    NAME te_shader_reflection_test
    MODULE_TARGET te_shader
    SOURCES tests/test_reflection.cpp
    ENABLE_CTEST
  )
  target_compile_definitions(te_shader_reflection_test PRIVATE TE_SHADER_USE_CORE=1)
  target_link_libraries(te_shader_reflection_test PRIVATE te_rendercore)
  tenengine_add_module_test(
    NAME te_shader_vertex_input_reflection_test
    MODULE_TARGET te_shader
    SOURCES tests/test_vertex_input_reflection.cpp
    ENABLE_CTEST
  )
  target_compile_definitions(te_shader_vertex_input_reflection_test PRIVATE TE_SHADER_USE_CORE=1)
  target_link_libraries(te_shader_vertex_input_reflection_test PRIVATE te_rendercore)
endif()
