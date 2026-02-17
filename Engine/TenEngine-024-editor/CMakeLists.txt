# TenEngine-024-editor: Editor module
# Depends: 001-core, 002-object, 003-application, 004-scene, 005-entity, 006-input,
#          007-subsystems, 013-resource, 029-world, 017-uicore, 018-ui
# Contract: specs/_contracts/024-editor-ABI.md

cmake_minimum_required(VERSION 3.16)
project(te_editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake" CACHE PATH "TenEngine helpers")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")
tenengine_resolve_my_dependencies("024-editor" OUT_DEPS MY_DEPS)

set(EDITOR_SOURCES
  # Core files
  src/Editor.cpp
  src/EntityAdapter.cpp
  src/FileDialog.cpp
  src/RenderViewportImpl.cpp
  src/UndoSystemImpl.cpp
  src/RenderingSettingsPanelImpl.cpp
  src/SceneViewImpl.cpp
  src/ResourceViewImpl.cpp
  src/PropertyPanelImpl.cpp
  
  # Phase 1: Core components
  src/GizmoImpl.cpp
  src/EditorCamera.cpp
  src/SelectionManager.cpp
  src/SnapSettings.cpp
  src/MainMenuImpl.cpp
  src/ToolbarImpl.cpp
  src/StatusBarImpl.cpp
  src/ConsolePanelImpl.cpp
  src/EditorPreferences.cpp
  src/ProfilerPanelImpl.cpp
  src/StatisticsPanelImpl.cpp
  src/LayoutManager.cpp
  
  # Phase 2-3: Enhanced components
  src/SceneSearch.cpp
  src/KeyBindingSystem.cpp
  src/EditorScripting.cpp
  src/DebugVisualization.cpp
  
  # Phase 4: Advanced features
  src/PrefabSystem.cpp
  src/PluginSystem.cpp
  src/HistoryManager.cpp
  src/MultiObjectEditing.cpp
  src/ViewportStats.cpp
)

if(WIN32)
  list(APPEND EDITOR_SOURCES src/ImGuiBackend.cpp)
endif()

set(EDITOR_HEADERS
  # Core files
  include/te/editor/Editor.h
  include/te/editor/EditorPanel.h
  include/te/editor/FileDialog.h
  include/te/editor/EntityAdapter.h
  include/te/editor/RenderingConfig.h
  include/te/editor/RenderingSettingsPanel.h
  include/te/editor/Viewport.h
  include/te/editor/SceneView.h
  include/te/editor/ResourceView.h
  include/te/editor/PropertyPanel.h
  include/te/editor/UndoSystem.h
  include/te/editor/ImGuiBackend.h
  
  # Phase 1: Core components
  include/te/editor/EditorTypes.h
  include/te/editor/Gizmo.h
  include/te/editor/EditorCamera.h
  include/te/editor/SelectionManager.h
  include/te/editor/SnapSettings.h
  include/te/editor/MainMenu.h
  include/te/editor/Toolbar.h
  include/te/editor/StatusBar.h
  include/te/editor/ConsolePanel.h
  include/te/editor/EditorPreferences.h
  include/te/editor/ProfilerPanel.h
  include/te/editor/StatisticsPanel.h
  include/te/editor/LayoutManager.h
  
  # Phase 2-3: Enhanced components
  include/te/editor/SceneSearch.h
  include/te/editor/KeyBindingSystem.h
  include/te/editor/EditorScripting.h
  include/te/editor/DebugVisualization.h
  
  # Phase 4: Advanced features
  include/te/editor/PrefabSystem.h
  include/te/editor/PluginSystem.h
  include/te/editor/HistoryManager.h
  include/te/editor/MultiObjectEditing.h
  include/te/editor/ViewportStats.h
)

add_library(te_editor STATIC
  ${EDITOR_SOURCES}
  ${EDITOR_HEADERS}
)

target_include_directories(te_editor PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(te_editor PRIVATE ${MY_DEPS})

if(TARGET imgui_backends)
  target_link_libraries(te_editor PRIVATE imgui_backends)
endif()
if(WIN32)
  target_link_libraries(te_editor PRIVATE d3d11 dxgi)
endif()

set_target_properties(te_editor PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

source_group("Source Files" FILES ${EDITOR_SOURCES})
source_group("Header Files" FILES ${EDITOR_HEADERS})
