# 012-Mesh worktree (build convention: docs/engine-build-module-convention.md).
# C++17, contract-first (specs/_contracts/012-mesh-ABI.md).
# Build root: worktree root. Dependencies: 001-core, 008-rhi, 009-rendercore, 013-resource, 030-device-resource-manager (resolve via TENENGINE_*_DIR when present).
cmake_minimum_required(VERSION 3.16)
project(TenEngine-012-mesh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 跳过上游依赖模块的测试，只构建本模块的测试
set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

# 上游 worktree 根目录：所有依赖模块都在 Engine/ 目录下
# TENENGINE_ROOT 应该指向包含 Engine 目录的父目录（即 TenEngine 仓库根目录）
if(NOT DEFINED TENENGINE_ROOT)
  # 从当前模块路径推导：Engine/TenEngine-012-mesh -> Engine -> TenEngine
  get_filename_component(_CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
  get_filename_component(_ENGINE_DIR "${_CURRENT_DIR}" DIRECTORY)
  set(TENENGINE_ROOT "${_ENGINE_DIR}")
endif()

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# --- 012-Mesh 依赖：001-core, 008-rhi, 009-rendercore, 013-resource, 030-device-resource-manager（源码，自 TENENGINE_ROOT/Engine 解析）---
# 所有依赖模块都在 ${TENENGINE_ROOT}/Engine/ 目录下
# 设置正确的依赖模块路径
if(NOT DEFINED TENENGINE_001_CORE_DIR)
  set(TENENGINE_001_CORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-001-core")
endif()
if(NOT DEFINED TENENGINE_002_OBJECT_DIR)
  set(TENENGINE_002_OBJECT_DIR "${TENENGINE_ROOT}/Engine/TenEngine-002-object")
endif()
if(NOT DEFINED TENENGINE_008_RHI_DIR)
  set(TENENGINE_008_RHI_DIR "${TENENGINE_ROOT}/Engine/TenEngine-008-rhi")
endif()
if(NOT DEFINED TENENGINE_009_RENDERCORE_DIR)
  set(TENENGINE_009_RENDERCORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-009-render-core")
endif()
if(NOT DEFINED TENENGINE_013_RESOURCE_DIR)
  set(TENENGINE_013_RESOURCE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-013-resource")
endif()
if(NOT DEFINED TENENGINE_030_DEVICE_RESOURCE_MANAGER_DIR)
  set(TENENGINE_030_DEVICE_RESOURCE_MANAGER_DIR "${TENENGINE_ROOT}/Engine/TenEngine-030-device-resource-manager")
endif()

# 解析依赖（按依赖顺序）
# 先解析013-resource，因为它会创建te_core（001-core）和te_object（002-object）
tenengine_resolve_dependency(013-resource RES_TARGET)

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

# 解析其他依赖
tenengine_resolve_dependency(008-rhi RHI_TARGET)
tenengine_resolve_dependency(009-rendercore RENDERCORE_TARGET)
tenengine_resolve_dependency(030-device-resource-manager DEVICERESOURCE_TARGET)
# Fix: Ensure the correct target name is used (te_deviceresource, not te_device_resource_manager)
if(NOT TARGET ${DEVICERESOURCE_TARGET} AND TARGET te_deviceresource)
  set(DEVICERESOURCE_TARGET te_deviceresource)
endif()

# --- 012-Mesh library ---
set(MESH_SOURCES
  src/Mesh.cpp
  src/MeshAssetDesc.cpp
  src/MeshFactory.cpp
  src/MeshDevice.cpp
  src/MeshResource.cpp
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
  include/te/mesh/MeshDevice.h
  include/te/mesh/MeshResource.h
  include/te/mesh/MeshLoader.h
  include/te/mesh/MeshDeserializer.h
  include/te/mesh/MeshSerialize.h
  include/te/mesh/MeshModuleInit.h
  include/te/mesh/MeshImporters.h
  include/te/mesh/detail/mesh_data.hpp
)

# 第三方库集成（可选）
option(TENENGINE_USE_ASSIMP "Enable Assimp for mesh import" OFF)
option(TENENGINE_USE_FAST_OBJ "Enable fast_obj for OBJ import" OFF)
option(TENENGINE_USE_CGLTF "Enable cgltf for glTF import" OFF)

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
target_include_directories(te_mesh PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Explicitly add deviceresource include directory
if(DEFINED TENENGINE_030_DEVICE_RESOURCE_MANAGER_DIR)
  target_include_directories(te_mesh PRIVATE "${TENENGINE_030_DEVICE_RESOURCE_MANAGER_DIR}/include")
endif()
target_link_libraries(te_mesh 
  PRIVATE ${CORE_TARGET}
  PUBLIC ${RHI_TARGET} ${RENDERCORE_TARGET} ${RES_TARGET} ${DEVICERESOURCE_TARGET})

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
