# 012-Mesh worktree (build convention: docs/engine-build-module-convention.md).
# C++17, contract-first (specs/_contracts/012-mesh-ABI.md).
# Build root: worktree root. Dependencies: 001-core, 008-rhi, 009-rendercore, 013-resource, 030-device-resource-manager (resolve via TENENGINE_*_DIR when present).
cmake_minimum_required(VERSION 3.16)
project(TenEngine-012-mesh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 跳过上游依赖模块的测试，只构建本模块的测试
set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

# 上游 worktree：依赖模块在 Engine/ 下。从 012 目录构建时用兄弟路径；否则用 TENENGINE_ROOT/Engine/...
if(NOT DEFINED TENENGINE_ROOT)
  get_filename_component(_CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
  get_filename_component(TENENGINE_ROOT "${_CURRENT_DIR}" DIRECTORY)
endif()
set(_ENGINE "${CMAKE_CURRENT_SOURCE_DIR}/..")

# 为 012 及下游（009 会解析 008-rhi）设置 *_DIR，避免 TENENGINE_ROOT 导致路径变成 repo_root/TenEngine-xxx
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

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# --- 012-Mesh 依赖：001-core, 009-rendercore, 013-resource（CPU-only）---

# 解析依赖（按依赖顺序）
tenengine_resolve_dependency(013-resource RES_TARGET)
tenengine_resolve_dependency(009-rendercore RENDERCORE_TARGET)

# 检查te_core是否已由013-resource创建，如果没有则单独解析001-core
if(NOT TARGET te_core)
  if(NOT DEFINED TENENGINE_001_CORE_DIR)
    set(TENENGINE_001_CORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-001-core")
  endif()
  tenengine_resolve_dependency(001-core CORE_TARGET)
else()
  # 使用013-resource创建的te_core
  set(CORE_TARGET te_core)
endif()

# 检查te_object是否已由013-resource创建，如果没有则单独解析002-object
if(NOT TARGET te_object)
  if(NOT DEFINED TENENGINE_002_OBJECT_DIR)
    set(TENENGINE_002_OBJECT_DIR "${TENENGINE_ROOT}/Engine/TenEngine-002-object")
  endif()
  tenengine_resolve_dependency(002-object OBJECT_TARGET)
else()
  # 使用013-resource创建的te_object
  set(OBJECT_TARGET te_object)
endif()

# --- 012-Mesh library ---
set(MESH_SOURCES
  src/Mesh.cpp
  src/MeshAssetDesc.cpp
  src/MeshFactory.cpp
  src/MeshResource.cpp
  src/BuiltinMeshes.cpp
  src/MeshLoader.cpp
  src/MeshDeserializer.cpp
  src/MeshSerialize.cpp
  src/MeshModuleInit.cpp
  src/detail/mesh_data.cpp
)

set(MESH_HEADERS
  include/te/mesh/Mesh.h
  include/te/mesh/MeshAssetDesc.h
  include/te/mesh/MeshFactory.h
  include/te/mesh/MeshResource.h
  include/te/mesh/BuiltinMeshes.h
  include/te/mesh/MeshLoader.h
  include/te/mesh/MeshDeserializer.h
  include/te/mesh/MeshSerialize.h
  include/te/mesh/MeshModuleInit.h
  include/te/mesh/MeshImporters.h
  include/te/mesh/detail/mesh_data.hpp
)

# Third-party import: Assimp for FBX etc.; fast_obj and cgltf for OBJ/glTF.
option(TENENGINE_USE_ASSIMP "Enable Assimp for mesh import (FBX, etc.)" ON)
option(TENENGINE_USE_FAST_OBJ "Enable fast_obj for OBJ import" ON)
option(TENENGINE_USE_CGLTF "Enable cgltf for glTF/GLB import" ON)

if(TENENGINE_USE_ASSIMP)
  include(FetchContent)
  FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.4.0
  )
  set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
  set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
  FetchContent_MakeAvailable(assimp)
  list(APPEND MESH_SOURCES src/import/AssimpImporter.cpp)
  add_definitions(-DTENENGINE_USE_ASSIMP)
endif()

if(TENENGINE_USE_FAST_OBJ)
  include(FetchContent)
  FetchContent_Declare(
    fast_obj
    GIT_REPOSITORY https://github.com/thisistherk/fast_obj.git
    GIT_TAG        master
  )
  FetchContent_Populate(fast_obj)
  add_library(fast_obj INTERFACE)
  target_include_directories(fast_obj INTERFACE ${fast_obj_SOURCE_DIR})
  # FastObjImporter.cpp already defines FAST_OBJ_IMPLEMENTATION
  list(APPEND MESH_SOURCES src/import/FastObjImporter.cpp)
  add_definitions(-DTENENGINE_USE_FAST_OBJ)
endif()

if(TENENGINE_USE_CGLTF)
  include(FetchContent)
  FetchContent_Declare(
    cgltf
    GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
    GIT_TAG        master
  )
  FetchContent_Populate(cgltf)
  add_library(cgltf INTERFACE)
  target_include_directories(cgltf INTERFACE ${cgltf_SOURCE_DIR})
  # CgltfImporter.cpp already defines CGLTF_IMPLEMENTATION
  list(APPEND MESH_SOURCES src/import/CgltfImporter.cpp)
  add_definitions(-DTENENGINE_USE_CGLTF)
endif()

add_library(te_mesh STATIC
  ${MESH_SOURCES}
  ${MESH_HEADERS}
)
if(MSVC)
  target_compile_options(te_mesh PRIVATE $<$<CONFIG:Debug>:/Zi /Od /FS>)
  set_target_properties(te_mesh PROPERTIES
    COMPILE_PDB_NAME te_mesh
    COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
  )
endif()
target_include_directories(te_mesh PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(te_mesh
  PRIVATE ${CORE_TARGET}
  PUBLIC ${RENDERCORE_TARGET} ${RES_TARGET})

# Link 002-object if available
if(OBJECT_TARGET AND TARGET ${OBJECT_TARGET})
  target_link_libraries(te_mesh PUBLIC ${OBJECT_TARGET})
endif()

# Link third-party libraries if enabled
if(TENENGINE_USE_ASSIMP)
  target_link_libraries(te_mesh PRIVATE assimp::assimp)
  target_include_directories(te_mesh PRIVATE ${assimp_SOURCE_DIR}/include)
endif()

if(TENENGINE_USE_FAST_OBJ)
  target_link_libraries(te_mesh PRIVATE fast_obj)
endif()

if(TENENGINE_USE_CGLTF)
  target_link_libraries(te_mesh PRIVATE cgltf)
endif()

# Organize files in Visual Studio project view
source_group("Source Files" FILES ${MESH_SOURCES})
source_group("Source Files\\Detail" FILES src/detail/mesh_data.cpp)
source_group("Source Files\\Import" FILES 
  src/import/AssimpImporter.cpp
  src/import/FastObjImporter.cpp
  src/import/CgltfImporter.cpp
)
source_group("Header Files" FILES ${MESH_HEADERS})
source_group("Header Files\\Detail" FILES include/te/mesh/detail/mesh_data.hpp)

# 作为上游被引用时不加入测试（与 009/013 等一致）
if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
