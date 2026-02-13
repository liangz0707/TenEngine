# TenEngine root: build mesh, material, texture, pipeline and their dependencies.
# Usage: from repo root:
#   cmake -B build -S .
#   cmake --build build [--config Debug|Release]
cmake_minimum_required(VERSION 3.16)
project(TenEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection (for 003-application, etc.)
if(WIN32)
  set(TE_PLATFORM_WINDOWS ON)
  set(TE_PLATFORM_LINUX OFF)
  set(TE_PLATFORM_MACOS OFF)
elseif(UNIX AND APPLE)
  set(TE_PLATFORM_WINDOWS OFF)
  set(TE_PLATFORM_LINUX OFF)
  set(TE_PLATFORM_MACOS ON)
elseif(UNIX)
  set(TE_PLATFORM_WINDOWS OFF)
  set(TE_PLATFORM_LINUX ON)
  set(TE_PLATFORM_MACOS OFF)
else()
  set(TE_PLATFORM_WINDOWS OFF)
  set(TE_PLATFORM_LINUX OFF)
  set(TE_PLATFORM_MACOS OFF)
endif()

# Repo root = parent of Engine/
set(TENENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

# Point each module to Engine/TenEngine-xxx (helper default is TENENGINE_ROOT/TenEngine-xxx)
set(TENENGINE_001_CORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-001-core")
set(TENENGINE_002_OBJECT_DIR "${TENENGINE_ROOT}/Engine/TenEngine-002-object")
set(TENENGINE_004_SCENE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-004-scene")
set(TENENGINE_005_ENTITY_DIR "${TENENGINE_ROOT}/Engine/TenEngine-005-entity")
set(TENENGINE_008_RHI_DIR "${TENENGINE_ROOT}/Engine/TenEngine-008-rhi")
set(TENENGINE_009_RENDERCORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-009-render-core")
set(TENENGINE_010_SHADER_DIR "${TENENGINE_ROOT}/Engine/TenEngine-010-shader")
set(TENENGINE_013_RESOURCE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-013-resource")
set(TENENGINE_019_PIPELINECORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-019-pipeline-core")
set(TENENGINE_028_TEXTURE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-028-texture")
set(TENENGINE_029_WORLD_DIR "${TENENGINE_ROOT}/Engine/TenEngine-029-world")
set(TENENGINE_011_MATERIAL_DIR "${TENENGINE_ROOT}/Engine/TenEngine-011-material")
set(TENENGINE_012_MESH_DIR "${TENENGINE_ROOT}/Engine/TenEngine-012-mesh")
set(TENENGINE_020_PIPELINE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-020-pipeline")

# Optional: build only mesh, material, texture, pipeline (no tests from other modules)
option(TENENGINE_BUILD_MESH "Build 012-mesh" ON)
option(TENENGINE_BUILD_MATERIAL "Build 011-material" ON)
option(TENENGINE_BUILD_TEXTURE "Build 028-texture" ON)
option(TENENGINE_BUILD_PIPELINE "Build 020-pipeline" ON)

# Dependency order: 001 -> 002 -> 008 -> 009 -> 004 -> 005 -> 013 -> 010 -> 019 -> 029 -> 028 -> 011 -> 012 -> 020
# Mark each as added so downstream resolve_dependency() does not add again.
add_subdirectory(Engine/TenEngine-001-core)
set_property(GLOBAL PROPERTY TENENGINE_ADDED_001-core TRUE)
add_subdirectory(Engine/TenEngine-002-object)
set_property(GLOBAL PROPERTY TENENGINE_ADDED_002-object TRUE)
add_subdirectory(Engine/TenEngine-008-rhi)
set_property(GLOBAL PROPERTY TENENGINE_ADDED_008-rhi TRUE)
add_subdirectory(Engine/TenEngine-009-render-core)
set_property(GLOBAL PROPERTY TENENGINE_ADDED_009-rendercore TRUE)
add_subdirectory(Engine/TenEngine-004-scene)
set_property(GLOBAL PROPERTY TENENGINE_ADDED_004-scene TRUE)
add_subdirectory(Engine/TenEngine-005-entity)
set_property(GLOBAL PROPERTY TENENGINE_ADDED_005-entity TRUE)
add_subdirectory(Engine/TenEngine-013-resource)
set_property(GLOBAL PROPERTY TENENGINE_ADDED_013-resource TRUE)
add_subdirectory(Engine/TenEngine-010-shader)
set_property(GLOBAL PROPERTY TENENGINE_ADDED_010-shader TRUE)
add_subdirectory(Engine/TenEngine-019-pipeline-core)
set_property(GLOBAL PROPERTY TENENGINE_ADDED_019-pipelinecore TRUE)
add_subdirectory(Engine/TenEngine-029-world)
set_property(GLOBAL PROPERTY TENENGINE_ADDED_029-world TRUE)

if(TENENGINE_BUILD_TEXTURE)
  add_subdirectory(Engine/TenEngine-028-texture)
endif()
if(TENENGINE_BUILD_MATERIAL)
  add_subdirectory(Engine/TenEngine-011-material)
endif()
if(TENENGINE_BUILD_MESH)
  add_subdirectory(Engine/TenEngine-012-mesh)
endif()
if(TENENGINE_BUILD_PIPELINE)
  add_subdirectory(Engine/TenEngine-020-pipeline)
endif()

# ---------------------------------------------------------------------------
# Editor build (024-Editor, 017-UICore, 018-UI, ImGui)
# ---------------------------------------------------------------------------
option(TENENGINE_BUILD_EDITOR "Build 024-Editor and dependencies (017-UICore, 018-UI, ImGui)" OFF)

if(TENENGINE_BUILD_EDITOR)
  set(TENENGINE_003_APPLICATION_DIR "${TENENGINE_ROOT}/Engine/TenEngine-003-application")
  set(TENENGINE_006_INPUT_DIR "${TENENGINE_ROOT}/Engine/TenEngine-006-input")
  set(TENENGINE_007_SUBSYSTEMS_DIR "${TENENGINE_ROOT}/Engine/TenEngine-007-subsystems")
  set(TENENGINE_017_UICORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-017-uicore")
  set(TENENGINE_018_UI_DIR "${TENENGINE_ROOT}/Engine/TenEngine-018-ui")
  set(TENENGINE_024_EDITOR_DIR "${TENENGINE_ROOT}/Engine/TenEngine-024-editor")

  # ImGui via FetchContent
  include(FetchContent)
  FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90
  )
  set(IMGUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(IMGUI_BUILD_DEMO OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(imgui)

  # ImGui: core + Win32 + DX11 (no 008-RHI dependency)
  if(WIN32)
    add_library(imgui_backends STATIC
      ${imgui_SOURCE_DIR}/imgui.cpp
      ${imgui_SOURCE_DIR}/imgui_draw.cpp
      ${imgui_SOURCE_DIR}/imgui_tables.cpp
      ${imgui_SOURCE_DIR}/imgui_widgets.cpp
      ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
      ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    )
    target_include_directories(imgui_backends PUBLIC
      ${imgui_SOURCE_DIR}
      ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui_backends PUBLIC d3d11)
  endif()

  add_subdirectory(Engine/TenEngine-003-application)
  set_property(GLOBAL PROPERTY TENENGINE_ADDED_003-application TRUE)
  add_subdirectory(Engine/TenEngine-006-input)
  set_property(GLOBAL PROPERTY TENENGINE_ADDED_006-input TRUE)
  add_subdirectory(Engine/TenEngine-007-subsystems)
  set_property(GLOBAL PROPERTY TENENGINE_ADDED_007-subsystems TRUE)
  add_subdirectory(Engine/TenEngine-017-uicore)
  set_property(GLOBAL PROPERTY TENENGINE_ADDED_017-uicore TRUE)
  add_subdirectory(Engine/TenEngine-018-ui)
  set_property(GLOBAL PROPERTY TENENGINE_ADDED_018-ui TRUE)
  add_subdirectory(Engine/TenEngine-024-editor)

  # tenengine_editor executable (link texture/material/mesh so debugger can resolve symbols and Import works)
  add_executable(tenengine_editor Engine/TenEngine-024-editor/src/main_editor.cpp)
  target_link_libraries(tenengine_editor PRIVATE te_editor te_application te_core te_resource)
  if(TARGET te_texture)
    target_link_libraries(tenengine_editor PRIVATE te_texture)
  endif()
  if(TARGET te_material)
    target_link_libraries(tenengine_editor PRIVATE te_material)
  endif()
  if(TARGET te_mesh)
    target_link_libraries(tenengine_editor PRIVATE te_mesh)
  endif()
  target_include_directories(tenengine_editor PRIVATE
    ${TENENGINE_ROOT}/Engine/TenEngine-024-editor/include
    ${TENENGINE_ROOT}/Engine/TenEngine-003-application/include
    ${TENENGINE_ROOT}/Engine/TenEngine-001-core/include
    ${TENENGINE_ROOT}/Engine/TenEngine-013-resource/include
  )
  set_target_properties(tenengine_editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  if(WIN32)
    target_link_libraries(tenengine_editor PRIVATE user32 gdi32)
  endif()
endif()
