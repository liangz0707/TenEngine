# 010-Shader library; C++17, contract-first (specs/_contracts/010-shader-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_shader LANGUAGES CXX)

enable_testing()
set(CMAKE_CXX_STANDARD 17)

# Must depend on upstream modules; standalone build is not allowed.
# Do not introduce upstream modules' tests when building this module.
set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

set(MY_DEPS "")
if(NOT DEFINED TENENGINE_CMAKE_DIR)
  set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake")
endif()
if(EXISTS "${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake")
  include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
  tenengine_resolve_my_dependencies("010-shader" OUT_DEPS MY_DEPS)
endif()
if(NOT MY_DEPS)
  message(FATAL_ERROR "te_shader: Upstream modules (001-core, 008-rhi, 009-rendercore) are required. "
    "TenEngineHelpers not found at TENENGINE_CMAKE_DIR=${TENENGINE_CMAKE_DIR} or dependencies could not be resolved.")
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
  GIT_TAG        14.0.0
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

add_library(te_shader STATIC ${TE_SHADER_SOURCES})
target_include_directories(te_shader PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(te_shader PRIVATE
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
