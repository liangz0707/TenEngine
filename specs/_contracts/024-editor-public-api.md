# Contract: 024-Editor Module Public API

## Status: **IMPLEMENTED**

## Applicable Module

- **Implementer**: 024-Editor (L4; viewport, scene tree, property panel, resource editing, menus; no downstream)
- **Specification**: `docs/module-specs/024-editor.md`
- **Dependencies**: 001-Core, 002-Object, 003-Application, 006-Input, 008-RHI, 013-Resource, 004-Scene, 005-Entity, 020-Pipeline, 018-UI, 028-Texture

## Consumers

- None (L4 consumer side)

## Capability List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| IEditor | Main editor interface; Run, panel accessors, play mode control, layout management | Managed by application |
| EditorContext | Editor initialization context; projectRootPath, windowHandle, application, resourceManager | Passed to CreateEditor |
| ISceneView | Scene tree view (left panel); hierarchy, selection, drag-drop | Managed by IEditor |
| IResourceView | Resource browser (bottom panel); resource list, preview, import | Managed by IEditor |
| IPropertyPanel | Property panel (right panel); component property display and editing | Managed by IEditor |
| IViewport | Render viewport (center panel); viewport rendering, picking, drag-drop | Managed by IEditor |
| IRenderingSettingsPanel | Rendering settings panel; show/hide, config load/save | Managed by IEditor |
| IGizmo | Gizmo transformation tool; translate/rotate/scale, hover state, camera matrices | Managed by IEditor |
| IEditorCamera | Editor camera controller; fly/orbit mode, bookmarks, viewport, ray casting | Managed by IEditor |
| ISelectionManager | Selection manager; multi-select, box selection, highlight, clipboard | Managed by IEditor |
| ISnapSettings | Snap settings; grid/rotation/scale/surface/vertex/edge snap | Managed by IEditor |
| IMainMenu | Main menu bar; File/Edit/View/GameObject/Tools/Help, recent files | Managed by IEditor |
| IToolbar | Toolbar; transform tools, play controls, snap, grid, viewport mode | Managed by IEditor |
| IStatusBar | Status bar; level name, FPS, memory, background tasks | Managed by IEditor |
| IConsolePanel | Console log panel; log levels, filtering, search, command input | Managed by IEditor |
| IEditorPreferences | Editor preferences; theme, key bindings, auto-save, viewport settings | Managed by IEditor |
| IProfilerPanel | Profiler panel; CPU/GPU frame times, scope timing, history | Managed by IEditor |
| IStatisticsPanel | Scene statistics panel; entity/component counts, scene bounds | Managed by IEditor |
| ILayoutManager | Layout manager; panel docking, layout presets, persistence | Managed by IEditor |
| ISceneSearch | Scene search; name pattern, component type filter, search history | Created via factory |
| IKeyBindingSystem | Key binding system; action registration, binding modification, conflicts | Created via factory |
| IEditorScripting | Editor scripting; command registration, macros, script execution | Created via factory |
| IDebugVisualization | Debug visualization; collision/nav/wireframe/bounds visualization | Created via factory |
| IPrefabSystem | Prefab system; prefab creation, instantiation, overrides, variants | Created via factory |
| IPluginSystem | Plugin system; plugin discovery, loading, hooks, menu/panel integration | Created via factory |
| IHistoryManager | History manager; enhanced undo/redo, bookmarks, search | Created via factory |
| IMultiObjectEditor | Multi-object editor; multi-selection editing, alignment, distribution | Created via factory |
| IViewportStats | Viewport statistics; FPS, draw calls, timing graphs | Created via factory |
| IUndoSystem | Undo/redo system; command stack, undo/redo operations | Created via factory |
| ICommand | Command interface for undo system; Execute, Undo, Redo | Managed by IUndoSystem |
| IEntity | Entity adapter for editor; wraps Entity for viewport picking | Created via factory |
| IEditorPanel | Base panel interface; dock/float, position, size, title | Base for panels |

### Enumerations

| Name | Values | Description |
|------|--------|-------------|
| GizmoMode | Translate, Rotate, Scale | Gizmo transformation mode |
| GizmoSpace | World, Local | Gizmo coordinate space |
| ViewportMode | Shaded, Wireframe, ShadedWireframe, Depth, Normals, Albedo | Viewport rendering mode |
| PlayModeState | Stopped, Playing, Paused | Editor play mode state |
| CameraNavigationMode | Fly, Orbit, Pan, Zoom | Camera navigation mode |
| ViewportLayout | Single, Quad, TwoHorizontal, TwoVertical | Viewport layout type |
| EditorTheme | Dark, Light, Classic | Editor UI theme |
| LogLevel | Info, Warning, Error, Fatal | Console log level |
| AxisFlags | None, X, Y, Z, XY, XZ, YZ, XYZ | Axis constraint flags |
| EditorTool | None, Select, Translate, Rotate, Scale, Terrain, Foliage | Active editor tool |
| GridAxis | XZ, XY, YZ | Grid display plane |
| PivotMode | Center, Pivot, BottomCenter | Transform pivot mode |
| CoordinateSpace | World, Local, Screen | Coordinate space indicator |
| HistoryActionType | Transform, PropertyChange, Create, Delete, ParentChange, ComponentAdd, ComponentRemove, Multiple, Custom | History action types |
| DebugVisFlags | None, Collision, Navigation, Wireframe, Normals, Bounds, Overdraw, LightComplexity, ShaderComplexity, Occlusion, Physics, AI, All | Debug visualization flags |
| PluginHook | OnEditorInit, OnEditorShutdown, OnSceneLoaded, OnSceneSaved, OnEntitySelected, OnEntityCreated, OnEntityDeleted, OnComponentAdded, OnComponentRemoved, OnPlayModeEnter, OnPlayModeExit, OnViewportRender, OnUpdate | Plugin hook points |

### Structures

| Name | Fields | Description |
|------|--------|-------------|
| EditorContext | projectRootPath, windowHandle, application, resourceManager | Editor initialization context |
| GizmoHoverState | hoveringX, hoveringY, hoveringZ, hoveringXY, hoveringXZ, hoveringYZ, hoveringXYZ | Gizmo axis hover state |
| GizmoOperation | active, mode, delta, newValue, hover | Gizmo operation result |
| CameraBookmark | position, rotation, target, distance, name | Camera bookmark data |
| SelectionChangeEvent | previousSelection, currentSelection, isAdditive | Selection change event data |
| SelectionFilter | selectEntities, selectResources, selectComponents, componentTypeFilter | Selection filter criteria |
| HistoryAction | id, type, description, timestamp, targetId, propertyName, undo, redo, subActions | History action record |
| HistoryBookmark | actionId, name, timestamp | History bookmark |
| SnapConfig | gridSnapEnabled, gridSize, gridOrigin, rotationSnapEnabled, rotationSnapAngle, scaleSnapEnabled, scaleSnapIncrement, surfaceSnapEnabled, surfaceSnapOffset, vertexSnapEnabled, vertexSnapDistance, edgeSnapEnabled, edgeSnapDistance, pivotSnapEnabled | Snap configuration |
| SnapResult | snappedPosition, snappedRotation, snappedScale, positionSnapped, rotationSnapped, scaleSnapped | Snap operation result |
| ToolbarButton | id, tooltip, icon, enabled, toggle, toggled, groupId | Toolbar button definition |
| MenuItem | label, shortcut, enabled, checked, separator, id, hasSubmenu, submenuItems | Menu item definition |
| MenuDef | name, items | Menu definition |
| BackgroundTask | name, progress, active, id | Background task info |
| ConsoleLogEntry | message, level, timestamp, category, frameCount | Console log entry (C-style) |
| ConsoleEntry | message, level, timestamp, category, frameCount, count | Console entry (C++ string) |
| FrameStats | frameTimeMs, cpuTimeMs, gpuTimeMs, drawCalls, triangles, vertices, memoryUsed | Frame statistics |
| SceneStats | entityCount, rootEntityCount, meshCount, lightCount, cameraCount, audioSourceCount | Scene statistics |
| EditorResult | success, message | Editor action result |
| EditorRect | x, y, width, height | UI layout rectangle |
| KeyBinding | keyCode, ctrl, alt, shift | Key binding definition |
| AssetDragPayload | resourceId, assetPath, resourceType | Asset drag-drop payload |
| PanelVisibility | panelId, visible, docked, dockRect | Panel visibility state |
| LayoutDef | name, viewportLayout, panels, isBuiltin | Layout definition |
| AutoSaveSettings | enabled, intervalSeconds, maxBackups | Auto-save settings |
| ViewportPreferences | fov, nearClip, farClip, cameraSpeed, rotationSpeed, showGrid, gridSize, showStats | Viewport preferences |
| UIPreferences | theme, fontSize, uiScale, showTooltips, undoHistorySize | UI preferences |
| ExternalTool | name, path, arguments | External tool configuration |
| RenderingConfig | renderPath, enableShadows, enableIBL, exposure | Rendering configuration |
| ProfilerScope | name, startTimeMs, endTimeMs, durationMs, depth, color | Profiler scope timing |
| FrameTimingEntry | frameTimeMs, cpuTimeMs, gpuTimeMs, drawCalls, triangles | Frame timing history |
| ComponentStats | typeName, count, enabledCount | Component type statistics |
| KeyCombo | keyCode, ctrl, alt, shift | Key combination for shortcuts |
| KeyBindingAction | id, displayName, category, defaultBinding, currentBinding, callback, enabled | Key binding action |
| CommandContext | editor, selection, activeObject | Editor command context |
| EditorCommand | id, displayName, category, menuItem, priority, execute, canExecute, isChecked, visible, enabled | Editor command definition |
| EditorMacro | name, commandIds, enabled | Macro definition |
| PluginInfo | id, name, version, author, description, path, enabled, loaded, hasError, errorText | Plugin metadata |
| PluginMenuItem | path, label, shortcut, enabled, checked, callback | Plugin menu item |
| PluginPanel | id, name, visible, canClose, canFloat, onDraw | Plugin panel definition |
| PrefabInstance | prefabId, instanceEntity, isDirty, localPosition, localRotation, localScale | Prefab instance data |
| PrefabOverride | type, componentId, propertyName | Prefab override data |
| PrefabData | id, name, path, componentTypes, lastModified, version | Prefab asset data |
| SceneSearchFilter | namePattern, caseSensitive, searchComponents, searchInactive, componentTypeFilter, layerFilter, tagFilter | Scene search filter |
| MixedValue\<T\> | value, isMixed, isSet | Mixed-state property value |
| MultiSelectionTransform | position, rotation, scale, pivotMode, transformSpace | Multi-selection transform |
| FrameTimingStats | cpuTimeMs, gpuTimeMs, totalFrameTimeMs, fps, cpuTimeHistory, gpuTimeHistory, historyIndex | Frame timing statistics |
| RenderStats | drawCalls, triangles, vertices, textureBindings, shaderBindings, bufferBindings, vramUsed, vramBudget | Rendering statistics |
| PhysicsStats | activeBodies, sleepingBodies, totalColliders, activeContacts, physicsTimeMs | Physics statistics |
| ViewportStatistics | timing, render, scene, physics, frameNumber, totalTime | Complete viewport statistics |
| StatsDisplaySettings | showFPS, showFrameTime, showDrawCalls, showTriangleCount, showVertexCount, showMemoryUsage, showEntityCount, showLightCount, showPhysicsStats, showGraphs, position, opacity, showBackground, fontSize | Stats display settings |
| CollisionVisSettings | showColliders, showTriggers, showStatic, showDynamic, showContacts, showSleeping, colliderColor, triggerColor, contactColor, opacity, lineWidth | Collision visualization settings |
| NavigationVisSettings | showMesh, showLinks, showObstacles, showPath, showLabels, walkableColor, obstacleColor, linkColor, pathColor, opacity | Navigation visualization settings |

### Capabilities (Provider Guarantees)

| ID | Capability | Description |
|----|------------|-------------|
| 1 | Viewport | IViewport, camera, gizmo, Pipeline/RHI integration; picking, drag-drop |
| 2 | Scene Tree | ISceneView, hierarchy, selection, drag-drop, Scene/Entity integration |
| 3 | Property Panel | IPropertyPanel, Object reflection integration, editing, undo |
| 4 | Resource Browser | IResourceView, list, preview, import, Resource integration |
| 5 | Menus | IMainMenu, main menu bar, toolbar, keyboard shortcuts |
| 6 | Gizmo | IGizmo, translate/rotate/scale transformation tool |
| 7 | Editor Camera | IEditorCamera, fly mode, orbit mode, zoom, focus, bookmarks |
| 8 | Selection Management | ISelectionManager, multi-select, box selection, highlight, selection events |
| 9 | Snap Settings | ISnapSettings, grid snap, rotation snap, scale snap |
| 10 | Main Menu | IMainMenu, File/Edit/View/GameObject/Tools/Help menus |
| 11 | Toolbar | IToolbar, transform tool toggle, play control buttons |
| 12 | Status Bar | IStatusBar, level name, FPS, selection count, background tasks |
| 13 | Console | IConsolePanel, log display, level filtering, search, command input |
| 14 | Preferences | IEditorPreferences, theme, fonts, key binding mapping |
| 15 | Profiling | IProfilerPanel, CPU/GPU frame times, draw calls |
| 16 | Scene Statistics | IStatisticsPanel, entity count, component count |
| 17 | Layout Management | ILayoutManager, layout save/load, preset layouts |
| 18 | Play Mode | EnterPlayMode, PausePlayMode, StopPlayMode, StepFrame |
| 19 | Scene Search | ISceneSearch, name fuzzy search, type filtering |
| 20 | Key Binding System | IKeyBindingSystem, key binding registration, modification, conflict detection |
| 21 | Editor Scripting | IEditorScripting, command registration, macros, script execution |
| 22 | Debug Visualization | IDebugVisualization, collision visualization, navigation visualization |
| 23 | Prefab System | IPrefabSystem, prefab creation, instantiation, overrides, variants |
| 24 | Plugin System | IPluginSystem, plugin loading, hooks, menu/panel integration |
| 25 | History Manager | IHistoryManager, enhanced undo/redo, bookmarks, search |
| 26 | Multi-Object Editing | IMultiObjectEditor, multi-selection editing, alignment, distribution |
| 27 | Viewport Statistics | IViewportStats, FPS, draw calls, timing graphs |
| 28 | Undo System | IUndoSystem, ICommand, command stack, undo/redo |
| 29 | File Dialog | OpenFileDialogMulti, multi-select file dialog |
| 30 | ImGui Backend | ImGuiBackend_Init, Shutdown, NewFrame, Render, Resize |

## Version / ABI

- Follows Constitution: Public API versioned; breaking changes increment MAJOR.
- Current Version: 4.0.0

## Constraints

- Must be used after all dependency modules are initialized.
- Layout and interaction conventions (left scene tree, bottom resource browser, right property panel) are documented by implementation.
- Some editing features are restricted in Play mode.

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 024-Editor contract |
| 2026-02-05 | Unified directory; capability list in table format |
| 2026-02-06 | Added P0 capabilities: Gizmo, Camera, Selection, Snap, Menu, Toolbar, StatusBar, Console, Preferences, Profiler, Statistics, Layout, Play mode control |
| 2026-02-07 | Added P1/P2 capabilities: SceneSearch, KeyBindingSystem, EditorScripting, DebugVisualization |
| 2026-02-22 | Comprehensive update to match actual code implementation; added all types, enums, structures; updated capability list |
