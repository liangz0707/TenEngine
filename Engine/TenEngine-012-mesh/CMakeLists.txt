# 012-Mesh (this worktree); C++17, contract-first.
# 构建根目录：本 worktree 根目录。依赖 001-core、008-rhi、009-rendercore、013-resource 以源码引入，见 docs/engine-build-module-convention.md §3。
# 上游模块在 G:\AIHUMAN\WorkSpaceSDD 下查找（TENENGINE_ROOT），禁止使用独立模式。
cmake_minimum_required(VERSION 3.16)
project(TenEngine-012-mesh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 上游 worktree 根目录：在此目录下查找 TenEngine-001-core、TenEngine-008-rhi 等
if(NOT DEFINED TENENGINE_ROOT)
  set(TENENGINE_ROOT "G:/AIHUMAN/WorkSpaceSDD")
endif()
# 009 实际目录名为 TenEngine-009-render-core（与 mod_id 009-rendercore 不一致）
set(TENENGINE_009_RENDERCORE_DIR "${TENENGINE_ROOT}/TenEngine-009-render-core")

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# --- 012-Mesh 依赖：001-core, 008-rhi, 009-rendercore, 013-resource（源码，自 TENENGINE_ROOT 解析）---
tenengine_resolve_dependency(001-core CORE_TARGET)
tenengine_resolve_dependency(008-rhi RHI_TARGET)
tenengine_resolve_dependency(009-rendercore RC_TARGET)
tenengine_resolve_dependency(013-resource RES_TARGET)

# --- 012-Mesh library ---
add_library(te_mesh
  src/mesh/MeshFactory.cpp
  src/mesh/Mesh.cpp
  src/mesh/MeshDevice.cpp
  src/mesh/MeshLoader.cpp
  src/mesh/MeshDeserializer.cpp
  src/mesh/MeshSerialize.cpp
)
target_include_directories(te_mesh PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(te_mesh PRIVATE ${CORE_TARGET} ${RHI_TARGET} ${RC_TARGET} ${RES_TARGET})

# 012-mesh 本模块测试始终加入（不受上游 009 设置的 TENENGINE_SKIP_DEPENDENCY_TESTS 影响）
add_subdirectory(tests)
