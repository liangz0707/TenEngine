# 024-Editor Module ABI

- **Contract**: [024-editor-public-api.md](./024-editor-public-api.md) (Capabilities and types description)
- **This Document**: 024-Editor external ABI explicit table.
- **Layout Convention**: **Left** scene resource manager, **bottom** resource browser, **right** property panel, **center** render viewport; viewport supports **click picking**, **drag-drop from resource manager**; right property panel can **display various component properties**.

## ABI Table

Column Definition: **Module | Namespace | Class | Export | Interface | Header | Symbol | Description**

### Core Interface

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 024-Editor | te::editor | IEditor | Abstract Interface | Editor Main Entry | te/editor/Editor.h | IEditor::Run | `void Run(EditorContext const& ctx);` Enter editor main loop |
| 024-Editor | te::editor | — | Struct | Editor Context | te/editor/Editor.h | EditorContext | `projectRootPath, windowHandle, application, resourceManager` |
| 024-Editor | te::editor | — | Free Function/Factory | Create Editor | te/editor/Editor.h | CreateEditor | `IEditor* CreateEditor(EditorContext const& ctx);` |
| 024-Editor | te::editor | — | Struct | Editor Types | te/editor/EditorTypes.h | EditorTypes | `GizmoMode, PlayModeState, ViewportMode` enums and structs |

### Existing Panel Interfaces

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 024-Editor | te::editor | ISceneView | Abstract Interface | Scene Tree View | te/editor/SceneView.h | ISceneView | `SetLevelHandle, SetSelection, GetSelection, OnDraw` Scene hierarchy, Entity tree |
| 024-Editor | te::editor | IResourceView | Abstract Interface | Resource Browser | te/editor/ResourceView.h | IResourceView | `OnDraw, SetRootPath, SetOnOpenLevel, SetOnDeleteLevel, SetResourceManager, ImportFiles` |
| 024-Editor | te::editor | IPropertyPanel | Abstract Interface | Property Panel | te/editor/PropertyPanel.h | IPropertyPanel | `Undo, Redo, CanUndo, CanRedo, OnDraw, SetSelection` Display component properties |
| 024-Editor | te::editor | IViewport | Abstract Interface | Render Viewport | te/editor/Viewport.h | IViewport | `PickInViewport, DropFromResourceManager, GetWidth, GetHeight, SetSize` |
| 024-Editor | te::editor | IRenderingSettingsPanel | Abstract Interface | Rendering Settings | te/editor/RenderingSettingsPanel.h | IRenderingSettingsPanel | `Show, Hide, IsVisible, GetConfig, SetConfig, SaveConfig, LoadConfig` |
| 024-Editor | te::editor | IEntity | Abstract Interface | Entity Adapter | te/editor/EntityAdapter.h | IEntity | `GetEntity` Wraps Entity for viewport picking |
| 024-Editor | te::editor | IEditorPanel | Abstract Interface | Panel Base | te/editor/EditorPanel.h | IEditorPanel | `SetDocked, IsFloating, SetPosition, SetSize, GetTitle, CanClose, OnDraw` |

### P0 Core Component Interfaces

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 024-Editor | te::editor | IGizmo | Abstract Interface | Gizmo Transform Tool | te/editor/Gizmo.h | IGizmo | `SetMode, GetMode, SetSpace, GetSpace, SetTarget, GetTarget, OnMouseDown, OnMouseMove, OnMouseUp, IsHovered, IsActive, GetHoverState, OnDraw, SetSize, GetSize, SetCameraMatrices, SetViewportSize` Translate/Rotate/Scale gizmo |
| 024-Editor | te::editor | IEditorCamera | Abstract Interface | Editor Camera | te/editor/EditorCamera.h | IEditorCamera | `SetViewportSize, OnInput, OnMouseDown/Up/Move/Wheel, OnKeyDown/Up, GetViewMatrix, GetProjectionMatrix, GetPosition, SetPosition, FocusOn, ResetView, FrameSelection, SetNavigationMode, SetMoveSpeed, SaveBookmark, LoadBookmark, ScreenPointToRay` Fly/orbit mode camera |
| 024-Editor | te::editor | ISelectionManager | Abstract Interface | Selection Manager | te/editor/SelectionManager.h | ISelectionManager | `Select, SelectMultiple, AddToSelection, ToggleSelection, Deselect, ClearSelection, IsSelected, GetSelection, BeginBoxSelection, UpdateBoxSelection, EndBoxSelection, CopySelection, CutSelection` Multi-select, box selection, highlight |
| 024-Editor | te::editor | ISnapSettings | Abstract Interface | Snap Settings | te/editor/SnapSettings.h | ISnapSettings | `SetGridSnapEnabled, SetGridSize, SetRotationSnapEnabled, SetRotationSnapAngle, SetScaleSnapEnabled, SnapPosition, SnapRotation, SnapScale, ApplySnap` Grid/rotation/scale snap |
| 024-Editor | te::editor | IMainMenu | Abstract Interface | Main Menu | te/editor/MainMenu.h | IMainMenu | `OnDraw, AddMenu, RemoveMenu, ClearMenus, GetMenu, SetItemEnabled, SetItemChecked, SetOnMenuItemClicked, AddRecentFile, ClearRecentFiles, InitializeStandardMenus` File/Edit/View/GameObject menu |
| 024-Editor | te::editor | IToolbar | Abstract Interface | Toolbar | te/editor/Toolbar.h | IToolbar | `OnDraw, SetTransformTool, GetTransformTool, SetPlayModeState, GetPlayModeState, SetOnPlayClicked, SetSnapEnabled, SetViewportMode, SetGridVisible, SetGizmoSpace, AddButton` Transform tools, play controls |
| 024-Editor | te::editor | IStatusBar | Abstract Interface | Status Bar | te/editor/StatusBar.h | IStatusBar | `OnDraw, SetLevelName, SetSelectionCount, SetFPS, SetFrameTime, SetMemoryUsage, AddBackgroundTask, UpdateBackgroundTask, RemoveBackgroundTask, SetStatusMessage` Level name, FPS, selection count |
| 024-Editor | te::editor | IConsolePanel | Abstract Interface | Console Panel | te/editor/ConsolePanel.h | IConsolePanel | `OnDraw, Log, LogInfo, LogWarning, LogError, Clear, GetEntries, SetSearchFilter, SetLogLevelFilter, SetCollapseEnabled, SetAutoScroll, SetCommandCallback` Log display, filtering |
| 024-Editor | te::editor | IEditorPreferences | Abstract Interface | Editor Preferences | te/editor/EditorPreferences.h | IEditorPreferences | `SetTheme, GetTheme, SetFontSize, SetUIScale, SetViewportFOV, SetAutoSaveEnabled, SetKeyBinding, GetKeyBinding, AddExternalTool, Save, Load, ResetToDefaults` Theme, key bindings, settings |
| 024-Editor | te::editor | IProfilerPanel | Abstract Interface | Profiler Panel | te/editor/ProfilerPanel.h | IProfilerPanel | `OnDraw, UpdateFrameStats, GetCurrentStats, GetAverageFrameTime, BeginScope, EndScope, ClearScopes, SetHistoryLength, Pause, Resume, SetVisible` CPU/GPU frame times |
| 024-Editor | te::editor | IStatisticsPanel | Abstract Interface | Statistics Panel | te/editor/StatisticsPanel.h | IStatisticsPanel | `OnDraw, SetEntityCount, SetRootEntityCount, SetComponentStats, SetSceneBounds, UpdateStats, RequestRefresh, SetVisible` Entity/Component counts |
| 024-Editor | te::editor | ILayoutManager | Abstract Interface | Layout Manager | te/editor/LayoutManager.h | ILayoutManager | `ApplyLayout, GetCurrentLayoutName, SaveCurrentLayout, AddLayout, SetPanelVisible, TogglePanel, DockPanel, FloatPanel, SetViewportLayout, SaveToFile, LoadFromFile, ResetToDefault` Panel layout save/load |

### P1 New Component Interfaces

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 024-Editor | te::editor | ISceneSearch | Abstract Interface | Scene Search | te/editor/SceneSearch.h | ISceneSearch | `Search, SetFilter, GetFilter, ClearResults, GetResults, MatchesFilter, GetHistory, OnDraw, FocusSearch` Name/type search filter |
| 024-Editor | te::editor | IKeyBindingSystem | Abstract Interface | Key Binding System | te/editor/KeyBindingSystem.h | IKeyBindingSystem | `RegisterAction, UnregisterAction, RegisterStandardActions, GetAction, GetBinding, SetBinding, ResetBinding, ProcessKeyEvent, ExecuteAction, SaveBindings, LoadBindings, OnDraw, StartRebind, FindConflicts` Key binding registration, execution |
| 024-Editor | te::editor | IEditorScripting | Abstract Interface | Editor Scripting | te/editor/EditorScripting.h | IEditorScripting | `RegisterCommand, UnregisterCommand, RegisterStandardCommands, ExecuteCommand, CanExecuteCommand, GetCommand, GetAllCommands, GetCommandsByCategory, CreateMacro, ExecuteMacro, ExecuteScript, ExecuteScriptString` Command registration, macros, script execution |

### P2 Debug Tool Interfaces

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 024-Editor | te::editor | IDebugVisualization | Abstract Interface | Debug Visualization | te/editor/DebugVisualization.h | IDebugVisualization | `SetFlags, GetFlags, EnableVisualization, SetCollisionSettings, SetNavigationSettings, OnDraw, DrawEntityCollision, DrawNavigationMesh, DrawLine, DrawBox, DrawSphere, DrawCylinder, DrawCapsule, DrawArrow, DrawText` Collision/nav/bounds visualization |

### P3 Advanced Feature Interfaces

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 024-Editor | te::editor | IPrefabSystem | Abstract Interface | Prefab System | te/editor/PrefabSystem.h | IPrefabSystem | `CreatePrefabFromEntity, CreateEmptyPrefab, InstantiatePrefab, GetPrefab, IsPrefabInstance, GetPrefabInstance, ApplyToPrefab, RevertToPrefab, BreakPrefabInstance, AddOverride, GetOverrides, CreateVariant, SavePrefab, LoadPrefab` Prefab creation, instantiation, overrides |
| 024-Editor | te::editor | IPluginSystem | Abstract Interface | Plugin System | te/editor/PluginSystem.h | IPluginSystem | `ScanForPlugins, GetDiscoveredPlugins, LoadPlugin, UnloadPlugin, LoadAllPlugins, IsPluginLoaded, GetPluginInfo, GetPlugin, SetPluginEnabled, RegisterHook, TriggerHook, GetAllPluginMenuItems, GetAllPluginPanels` Plugin loading, hooks, menus |
| 024-Editor | te::editor | IPlugin | Abstract Interface | Plugin Interface | te/editor/PluginSystem.h | IPlugin | `GetInfo, Initialize, Shutdown, GetMenuItems, GetPanels, OnHook` Plugin base interface |
| 024-Editor | te::editor | IHistoryManager | Abstract Interface | History Manager | te/editor/HistoryManager.h | IHistoryManager | `BeginCompoundAction, EndCompoundAction, RecordAction, RecordPropertyChange, PauseRecording, ResumeRecording, Undo, Redo, CanUndo, CanRedo, GetActionCount, GetActionAt, FindActions, JumpToAction, CreateBookmark, GetBookmarks, JumpToBookmark, ClearHistory, SaveHistory, LoadHistory, OnDraw` Enhanced undo/redo, bookmarks |
| 024-Editor | te::editor | IMultiObjectEditor | Abstract Interface | Multi-Object Editor | te/editor/MultiObjectEditing.h | IMultiObjectEditor | `SetSelection, GetSelection, GetTransform, SetPosition, SetRotation, SetScale, Move, Rotate, Scale, SetPivotMode, SetTransformSpace, GetSelectionCenter, GetSelectionBounds, GetCommonProperties, IsPropertyMixed, SetProperty, GetCommonComponents, AddComponent, GroupSelected, Align, Distribute, ParentToActive, DuplicateSelection, DeleteSelection, SetActiveObject` Multi-selection editing, alignment |
| 024-Editor | te::editor | IViewportStats | Abstract Interface | Viewport Stats | te/editor/ViewportStats.h | IViewportStats | `BeginFrame, EndFrame, UpdateTiming, UpdateRenderStats, UpdateSceneStats, UpdatePhysicsStats, AddDrawCall, ResetStats, GetStats, GetFPS, SetDisplaySettings, SetVisible, OnDraw, DrawTimingGraph` FPS, draw calls, memory stats |

### Undo System

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 024-Editor | te::editor | IUndoSystem | Abstract Interface | Undo/Redo System | te/editor/UndoSystem.h | IUndoSystem | `PushCommand, Undo, Redo, CanUndo, CanRedo` Command stack |
| 024-Editor | te::editor | ICommand | Abstract Interface | Command Interface | te/editor/UndoSystem.h | ICommand | `Execute, Undo, Redo` Command base for undo system |

### Play Mode Control Interface

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 024-Editor | te::editor | IEditor | Abstract Interface | Enter Play Mode | te/editor/Editor.h | IEditor::EnterPlayMode | `void EnterPlayMode();` Start running game |
| 024-Editor | te::editor | IEditor | Abstract Interface | Pause Play Mode | te/editor/Editor.h | IEditor::PausePlayMode | `void PausePlayMode();` Pause game |
| 024-Editor | te::editor | IEditor | Abstract Interface | Stop Play Mode | te/editor/Editor.h | IEditor::StopPlayMode | `void StopPlayMode();` Stop and return to edit mode |
| 024-Editor | te::editor | IEditor | Abstract Interface | Single Frame Step | te/editor/Editor.h | IEditor::StepFrame | `void StepFrame();` Advance one frame |
| 024-Editor | te::editor | IEditor | Abstract Interface | Play Mode State | te/editor/Editor.h | IEditor::IsInPlayMode | `bool IsInPlayMode() const;` Query current state |
| 024-Editor | te::editor | IEditor | Abstract Interface | Get Play Mode State | te/editor/Editor.h | IEditor::GetPlayModeState | `PlayModeState GetPlayModeState() const;` Get detailed state |

### Layout Management Interface

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 024-Editor | te::editor | IEditor | Abstract Interface | Save Layout | te/editor/Editor.h | IEditor::SaveLayout | `void SaveLayout(char const* path);` Save panel layout |
| 024-Editor | te::editor | IEditor | Abstract Interface | Load Layout | te/editor/Editor.h | IEditor::LoadLayout | `void LoadLayout(char const* path);` Load panel layout |
| 024-Editor | te::editor | IEditor | Abstract Interface | Reset Layout | te/editor/Editor.h | IEditor::ResetLayout | `void ResetLayout();` Reset default layout |

### ImGui Backend

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 024-Editor | te::editor | — | Free Function | Register WndProc | te/editor/ImGuiBackend.h | ImGuiBackend_RegisterWndProcHandler | `void ImGuiBackend_RegisterWndProcHandler(void* application);` Register ImGui input handler |
| 024-Editor | te::editor | — | Free Function | Initialize | te/editor/ImGuiBackend.h | ImGuiBackend_Init | `bool ImGuiBackend_Init(void* hwnd, int width, int height);` |
| 024-Editor | te::editor | — | Free Function | Shutdown | te/editor/ImGuiBackend.h | ImGuiBackend_Shutdown | `void ImGuiBackend_Shutdown();` |
| 024-Editor | te::editor | — | Free Function | New Frame | te/editor/ImGuiBackend.h | ImGuiBackend_NewFrame | `void ImGuiBackend_NewFrame();` |
| 024-Editor | te::editor | — | Free Function | Render | te/editor/ImGuiBackend.h | ImGuiBackend_Render | `void ImGuiBackend_Render();` |
| 024-Editor | te::editor | — | Free Function | Resize | te/editor/ImGuiBackend.h | ImGuiBackend_Resize | `void ImGuiBackend_Resize(int width, int height);` |
| 024-Editor | te::editor | — | Free Function | Get Dropped Paths | te/editor/ImGuiBackend.h | ImGuiBackend_GetAndClearDroppedPaths | `std::vector<std::string> ImGuiBackend_GetAndClearDroppedPaths();` |
| 024-Editor | te::editor | — | Free Function | Get Window Handle | te/editor/ImGuiBackend.h | ImGuiBackend_GetWindowHandle | `void* ImGuiBackend_GetWindowHandle();` |

### File Dialog

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 024-Editor | te::editor | — | Free Function | Open File Dialog Multi | te/editor/FileDialog.h | OpenFileDialogMulti | `std::vector<std::string> OpenFileDialogMulti(char const* filterDesc, char const* filterSpec, char const* initialDir);` Multi-select file dialog |

## Factory Functions

| Function Signature | Header | Description |
|-------------------|--------|-------------|
| `IEditor* CreateEditor(EditorContext const& ctx);` | te/editor/Editor.h | Create editor instance |
| `IGizmo* CreateGizmo();` | te/editor/Gizmo.h | Create gizmo instance |
| `IEditorCamera* CreateEditorCamera();` | te/editor/EditorCamera.h | Create editor camera |
| `ISelectionManager* CreateSelectionManager();` | te/editor/SelectionManager.h | Create selection manager |
| `ISnapSettings* CreateSnapSettings();` | te/editor/SnapSettings.h | Create snap settings |
| `IMainMenu* CreateMainMenu();` | te/editor/MainMenu.h | Create main menu |
| `IToolbar* CreateToolbar();` | te/editor/Toolbar.h | Create toolbar |
| `IStatusBar* CreateStatusBar();` | te/editor/StatusBar.h | Create status bar |
| `IConsolePanel* CreateConsolePanel();` | te/editor/ConsolePanel.h | Create console panel |
| `IEditorPreferences* CreateEditorPreferences();` | te/editor/EditorPreferences.h | Create editor preferences |
| `IProfilerPanel* CreateProfilerPanel();` | te/editor/ProfilerPanel.h | Create profiler panel |
| `IStatisticsPanel* CreateStatisticsPanel();` | te/editor/StatisticsPanel.h | Create statistics panel |
| `ILayoutManager* CreateLayoutManager();` | te/editor/LayoutManager.h | Create layout manager |
| `ISceneSearch* CreateSceneSearch();` | te/editor/SceneSearch.h | Create scene search |
| `IKeyBindingSystem* CreateKeyBindingSystem();` | te/editor/KeyBindingSystem.h | Create key binding system |
| `IEditorScripting* CreateEditorScripting();` | te/editor/EditorScripting.h | Create scripting system |
| `IDebugVisualization* CreateDebugVisualization();` | te/editor/DebugVisualization.h | Create debug visualization |
| `IPrefabSystem* CreatePrefabSystem();` | te/editor/PrefabSystem.h | Create prefab system |
| `IPluginSystem* CreatePluginSystem();` | te/editor/PluginSystem.h | Create plugin system |
| `IHistoryManager* CreateHistoryManager();` | te/editor/HistoryManager.h | Create history manager |
| `IMultiObjectEditor* CreateMultiObjectEditor();` | te/editor/MultiObjectEditing.h | Create multi-object editor |
| `IViewportStats* CreateViewportStats();` | te/editor/ViewportStats.h | Create viewport stats |
| `IUndoSystem* CreateUndoSystem(int maxDepth = 50);` | te/editor/UndoSystem.h | Create undo system |
| `IPropertyPanel* CreatePropertyPanel(IUndoSystem* undoSystem);` | te/editor/PropertyPanel.h | Create property panel |
| `ISceneView* CreateSceneView();` | te/editor/SceneView.h | Create scene view |
| `IResourceView* CreateResourceView();` | te/editor/ResourceView.h | Create resource view |
| `IViewport* CreateRenderViewport();` | te/editor/Viewport.h | Create render viewport |
| `IRenderingSettingsPanel* CreateRenderingSettingsPanel();` | te/editor/RenderingSettingsPanel.h | Create rendering settings panel |
| `IEntity* CreateEntityAdapter(te::entity::Entity* entity);` | te/editor/EntityAdapter.h | Create entity adapter |

## File Structure

```
Engine/TenEngine-024-editor/
├── include/te/editor/
│   ├── Editor.h                  # Main interface
│   ├── EditorTypes.h             # Type definitions (enums, structs)
│   ├── EditorPanel.h             # Panel base class
│   ├── Gizmo.h                   # Gizmo transform tool
│   ├── EditorCamera.h            # Editor camera
│   ├── SelectionManager.h        # Selection management
│   ├── SnapSettings.h            # Snap settings
│   ├── MainMenu.h                # Main menu
│   ├── Toolbar.h                 # Toolbar
│   ├── StatusBar.h               # Status bar
│   ├── ConsolePanel.h            # Console panel
│   ├── EditorPreferences.h       # Editor preferences
│   ├── ProfilerPanel.h           # Profiler panel
│   ├── StatisticsPanel.h         # Scene statistics
│   ├── LayoutManager.h           # Layout management
│   ├── SceneSearch.h             # Scene search
│   ├── KeyBindingSystem.h        # Key binding system
│   ├── EditorScripting.h         # Editor scripting
│   ├── DebugVisualization.h      # Debug visualization
│   ├── PrefabSystem.h            # Prefab system
│   ├── PluginSystem.h            # Plugin system
│   ├── HistoryManager.h          # History manager
│   ├── MultiObjectEditing.h      # Multi-object editing
│   ├── ViewportStats.h           # Viewport statistics
│   ├── Viewport.h                # Viewport interface
│   ├── SceneView.h               # Scene tree view
│   ├── ResourceView.h            # Resource browser
│   ├── PropertyPanel.h           # Property panel
│   ├── UndoSystem.h              # Undo system
│   ├── EntityAdapter.h           # Entity adapter
│   ├── FileDialog.h              # File dialog
│   ├── ImGuiBackend.h            # ImGui backend
│   └── RenderingConfig.h         # Rendering config struct
└── src/
    └── [Corresponding implementation files]
```

## Version History

| Version | Date | Change Description |
|---------|------|-------------------|
| 4.0.0 | 2026-02-22 | Comprehensive update to match actual code implementation; all interfaces, enums, structs documented |
| 3.0.0 | 2026-02-07 | Added P3 advanced features: Prefab, Plugin, History, MultiObject, ViewportStats |
| 2.0.0 | 2026-02-06 | Added P1-P2 features: SceneSearch, KeyBinding, Scripting, DebugVis |
| 1.0.0 | 2026-02-05 | Initial version: core panels + P0 components |

*Source: User stories US-lifecycle-002, US-editor-001~003; Contract capabilities: PickInViewport, DropFromResourceManager, UndoRedo, Gizmo, Camera, Selection, PlayMode, KeyBinding, DebugVis, Prefab, Plugin.*
