# 011-Material library; C++17, contract-first (specs/_contracts/011-material-public-api.md).
# Build root: current worktree (e.g. TenEngine-011-material). Dependencies: 009-rendercore, 010-shader, 013-resource (same pattern as 029-world).
cmake_minimum_required(VERSION 3.16)
project(te_material LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# 009 目录名为 TenEngine-009-render-core，与模块 id 009-rendercore 不一致，需显式指定
if(NOT DEFINED TENENGINE_009_RENDERCORE_DIR)
  if(DEFINED TENENGINE_ROOT)
    set(TENENGINE_009_RENDERCORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-009-render-core")
  else()
    set(TENENGINE_009_RENDERCORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-009-render-core")
  endif()
endif()

tenengine_resolve_my_dependencies("011-material" OUT_DEPS _material_deps)
if(NOT _material_deps)
  message(FATAL_ERROR "te_material: Dependencies (009-rendercore, 010-shader, 013-resource) could not be resolved.")
endif()

set(TE_MATERIAL_SOURCES
  src/material_json.cpp
  src/MaterialResource.cpp
  src/MaterialModuleInit.cpp
  src/storage_format.cpp
  src/factory.cpp
  src/material_def.cpp
  src/parameters.cpp
  src/instancing.cpp
  src/binding.cpp
)
set(TE_MATERIAL_HEADERS
  include/te/material/api.hpp
  include/te/material/binding.hpp
  include/te/material/factory.hpp
  include/te/material/instancing.hpp
  include/te/material/material_def.hpp
  include/te/material/material_json.hpp
  include/te/material/MaterialModuleInit.h
  include/te/material/MaterialParam.hpp
  include/te/material/MaterialResource.h
  include/te/material/parameters.hpp
  include/te/material/storage_format.hpp
  include/te/material/types.hpp
)

add_library(te_material STATIC ${TE_MATERIAL_SOURCES} ${TE_MATERIAL_HEADERS})

target_include_directories(te_material PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_include_directories(te_material PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(te_material PUBLIC ${_material_deps})

source_group("Source Files" FILES ${TE_MATERIAL_SOURCES})
source_group("Header Files" FILES ${TE_MATERIAL_HEADERS})
set_target_properties(te_material PROPERTIES PUBLIC_HEADER "${TE_MATERIAL_HEADERS}")

option(BUILD_TESTS "Build 011-Material tests" ON)
if(BUILD_TESTS AND TARGET te_material)
  enable_testing()
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/test_material_load.cpp")
    tenengine_add_module_test(
      NAME te_material_load_test
      MODULE_TARGET te_material
      SOURCES tests/unit/test_material_load.cpp
      ENABLE_CTEST
    )
  endif()
endif()
