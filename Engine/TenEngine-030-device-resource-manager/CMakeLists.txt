# 030-DeviceResourceManager (this worktree); C++17, contract-first.
# 构建根目录：本 worktree 根目录。依赖 001-core、008-rhi、013-resource 以源码引入，见 docs/engine-build-module-convention.md §3。
# 上游模块在 G:\AIHUMAN\WorkSpaceSDD 下查找（TENENGINE_ROOT），禁止使用独立模式。
cmake_minimum_required(VERSION 3.16)
project(TenEngine-030-device-resource-manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 上游 worktree 根目录：所有依赖模块都在 Engine/ 目录下
# TENENGINE_ROOT 应该指向包含 Engine 目录的父目录（即 TenEngine 仓库根目录）
if(NOT DEFINED TENENGINE_ROOT)
  # 从当前模块路径推导：Engine/TenEngine-030-device-resource-manager -> Engine -> TenEngine
  get_filename_component(_CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
  get_filename_component(_ENGINE_DIR "${_CURRENT_DIR}" DIRECTORY)
  set(TENENGINE_ROOT "${_ENGINE_DIR}")
endif()

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# --- 030-DeviceResourceManager 依赖：001-core, 008-rhi, 013-resource（源码，自 TENENGINE_ROOT/Engine 解析）---
# 所有依赖模块都在 ${TENENGINE_ROOT}/Engine/ 目录下
# 设置正确的依赖模块路径（包括013-resource需要的002-object路径）
if(NOT DEFINED TENENGINE_001_CORE_DIR)
  set(TENENGINE_001_CORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-001-core")
endif()
if(NOT DEFINED TENENGINE_002_OBJECT_DIR)
  set(TENENGINE_002_OBJECT_DIR "${TENENGINE_ROOT}/Engine/TenEngine-002-object")
endif()
if(NOT DEFINED TENENGINE_008_RHI_DIR)
  set(TENENGINE_008_RHI_DIR "${TENENGINE_ROOT}/Engine/TenEngine-008-rhi")
endif()
if(NOT DEFINED TENENGINE_013_RESOURCE_DIR)
  set(TENENGINE_013_RESOURCE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-013-resource")
endif()

# 先解析013-resource，因为它会创建te_core（001-core）和te_object（002-object）
# 013-resource在其CMakeLists.txt中会将001-core作为in-tree依赖创建te_core
# 注意：013-resource会使用TENENGINE_002_OBJECT_DIR来解析002-object
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

# 解析008-rhi
tenengine_resolve_dependency(008-rhi RHI_TARGET)
# 028-texture: According to dependency map, 030 should NOT depend on 028-texture.
# 030's implementation files may include TextureResource.h for type casting, but this should be
# handled through forward declarations and delayed includes in implementation files only.
# We do NOT add 028-texture's include directories or link to it here to maintain proper dependency boundaries.

# --- 030-DeviceResourceManager library ---
set(DEVICE_RESOURCE_MANAGER_SOURCES
  src/DeviceResourceManager.cpp
  src/CommandListPool.cpp
  src/StagingBufferManager.cpp
  src/TextureDevice.cpp
  src/internal/TextureDeviceImpl.cpp
  src/internal/ResourceUploadHelper.cpp
)

set(DEVICE_RESOURCE_MANAGER_HEADERS
  include/te/deviceresource/DeviceResourceManager.h
  include/te/deviceresource/CommandListPool.h
  include/te/deviceresource/StagingBufferManager.h
)

add_library(te_deviceresource STATIC
  ${DEVICE_RESOURCE_MANAGER_SOURCES}
  ${DEVICE_RESOURCE_MANAGER_HEADERS}
)
target_include_directories(te_deviceresource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Link to RHI first - this will add 008-rhi's include directories via PUBLIC interface
# Use PUBLIC for RHI and RES so that test targets can access their headers
target_link_libraries(te_deviceresource 
  PRIVATE ${CORE_TARGET}
  PUBLIC ${RHI_TARGET} ${RES_TARGET})
# Note: 030 does NOT depend on 028-texture according to dependency map.
# Implementation files that need TextureResource.h should use forward declarations in headers
# and include the header only in .cpp files. The include path should be provided by the build
# system when 028-texture is built, but we don't add it here to maintain dependency boundaries.

# Organize files in Visual Studio project view
source_group("Source Files" FILES 
  src/DeviceResourceManager.cpp
  src/CommandListPool.cpp
  src/StagingBufferManager.cpp
  src/TextureDevice.cpp
)
source_group("Source Files\\Internal" FILES
  src/internal/TextureDeviceImpl.cpp
  src/internal/ResourceUploadHelper.cpp
)
source_group("Header Files" FILES ${DEVICE_RESOURCE_MANAGER_HEADERS})

# Enable testing for this module
enable_testing()

# 030-device-resource-manager 本模块测试始终加入（不受上游设置的 TENENGINE_SKIP_DEPENDENCY_TESTS 影响）
if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  add_subdirectory(tests)
endif()
