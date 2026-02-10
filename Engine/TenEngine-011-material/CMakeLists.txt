# 011-Material library; C++17, contract-first (specs/_contracts/011-material-public-api.md).
# Build root: current worktree (e.g. TenEngine-011-material). Dependencies: 009-rendercore, 010-shader (001-core via te_shader); all via SOURCE (add_subdirectory).
cmake_minimum_required(VERSION 3.16)
project(te_material LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

set(MY_DEPS "")
if(NOT DEFINED TENENGINE_CMAKE_DIR)
  set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
endif()
if(EXISTS "${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake")
  include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
  tenengine_resolve_my_dependencies("011-material" OUT_DEPS MY_DEPS)
endif()
if(NOT MY_DEPS)
  message(FATAL_ERROR "te_material: Upstream (009-rendercore, 010-shader) required. TenEngineHelpers not found or deps unresolved.")
endif()

# Use absolute paths so VS solution shows files regardless of build root
set(_MAT "${CMAKE_CURRENT_SOURCE_DIR}")
set(TE_MATERIAL_SOURCES
  ${_MAT}/src/material_json.cpp
  ${_MAT}/src/MaterialResource.cpp
  ${_MAT}/src/MaterialModuleInit.cpp
  ${_MAT}/src/storage_format.cpp
  ${_MAT}/src/factory.cpp
  ${_MAT}/src/material_def.cpp
  ${_MAT}/src/parameters.cpp
  ${_MAT}/src/instancing.cpp
  ${_MAT}/src/binding.cpp
)
set(TE_MATERIAL_HEADERS
  ${_MAT}/include/te/material/api.hpp
  ${_MAT}/include/te/material/binding.hpp
  ${_MAT}/include/te/material/factory.hpp
  ${_MAT}/include/te/material/instancing.hpp
  ${_MAT}/include/te/material/material_def.hpp
  ${_MAT}/include/te/material/material_json.hpp
  ${_MAT}/include/te/material/MaterialModuleInit.h
  ${_MAT}/include/te/material/MaterialResource.h
  ${_MAT}/include/te/material/parameters.hpp
  ${_MAT}/include/te/material/storage_format.hpp
  ${_MAT}/include/te/material/types.hpp
)

# 与 010-shader、009-rendercore 一致：只把本模块源/头加入工程，上游仅通过 target_link_libraries 引入
add_library(te_material STATIC ${TE_MATERIAL_SOURCES} ${TE_MATERIAL_HEADERS})
source_group("Source Files" FILES ${TE_MATERIAL_SOURCES})
source_group("Header Files" FILES ${TE_MATERIAL_HEADERS})
target_include_directories(te_material
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-013-resource/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-002-object/include
)
target_link_libraries(te_material PRIVATE ${MY_DEPS})

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
