/**
 * @file EditorTypes.h
 * @brief Common types and enumerations for the editor module.
 */
#ifndef TE_EDITOR_EDITOR_TYPES_H
#define TE_EDITOR_EDITOR_TYPES_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace te {
namespace editor {

// Forward declarations
class IEditor;
class ISelectionManager;
class IEntity;

/**
 * @brief Gizmo transformation mode.
 */
enum class GizmoMode : uint8_t {
  Translate = 0,
  Rotate = 1,
  Scale = 2
};

/**
 * @brief Gizmo transformation space.
 */
enum class GizmoSpace : uint8_t {
  World = 0,
  Local = 1
};

/**
 * @brief Viewport rendering mode.
 */
enum class ViewportMode : uint8_t {
  Shaded = 0,
  Wireframe = 1,
  ShadedWireframe = 2,
  Depth = 3,
  Normals = 4,
  Albedo = 5
};

/**
 * @brief Editor play mode state.
 */
enum class PlayModeState : uint8_t {
  Stopped = 0,
  Playing = 1,
  Paused = 2
};

/**
 * @brief Camera navigation mode.
 */
enum class CameraNavigationMode : uint8_t {
  Fly = 0,       ///< WASD + mouse right-drag (FPS style)
  Orbit = 1,     ///< Alt + left-drag (orbit around target)
  Pan = 2,       ///< Middle-drag (pan)
  Zoom = 3       ///< Scroll wheel
};

/**
 * @brief Viewport layout type.
 */
enum class ViewportLayout : uint8_t {
  Single = 0,         ///< Single perspective viewport
  Quad = 1,           ///< Four viewports (Persp + Top + Front + Right)
  TwoHorizontal = 2,  ///< Two side by side
  TwoVertical = 3     ///< Two stacked vertically
};

/**
 * @brief Editor theme.
 */
enum class EditorTheme : uint8_t {
  Dark = 0,
  Light = 1,
  Classic = 2
};

/**
 * @brief Log level for console panel.
 */
enum class LogLevel : uint8_t {
  Info = 0,
  Warning = 1,
  Error = 2,
  Fatal = 3
};

/**
 * @brief Axis flags for transformation constraints.
 */
enum class AxisFlags : uint8_t {
  None = 0,
  X = 1 << 0,
  Y = 1 << 1,
  Z = 1 << 2,
  XY = X | Y,
  XZ = X | Z,
  YZ = Y | Z,
  XYZ = X | Y | Z
};

/**
 * @brief Editor tool type (active tool).
 */
enum class EditorTool : uint8_t {
  None = 0,
  Select = 1,
  Translate = 2,
  Rotate = 3,
  Scale = 4,
  Terrain = 5,
  Foliage = 6
};

/**
 * @brief Grid axis for viewport grid display.
 */
enum class GridAxis : uint8_t {
  XZ = 0,   ///< Ground plane (Y up)
  XY = 1,   ///< Front plane (Z up)
  YZ = 2    ///< Side plane (X up)
};

/**
 * @brief Pivot mode for transformations.
 */
enum class PivotMode : uint8_t {
  Center = 0,     ///< Use center of selection bounds
  Pivot = 1,      ///< Use pivot point of primary selected object
  BottomCenter = 2
};

/**
 * @brief Coordinate space indicator.
 */
enum class CoordinateSpace : uint8_t {
  World = 0,
  Local = 1,
  Screen = 2
};

/**
 * @brief History action types for undo/redo system.
 */
enum class HistoryActionType : uint8_t {
  Transform = 0,
  PropertyChange = 1,
  Create = 2,
  Delete = 3,
  ParentChange = 4,
  ComponentAdd = 5,
  ComponentRemove = 6,
  Multiple = 7,
  Custom = 8
};

/**
 * @brief Debug visualization flags.
 */
enum class DebugVisFlags : uint32_t {
  None = 0,
  Collision = 1 << 0,
  Navigation = 1 << 1,
  Wireframe = 1 << 2,
  Normals = 1 << 3,
  Bounds = 1 << 4,
  Overdraw = 1 << 5,
  LightComplexity = 1 << 6,
  ShaderComplexity = 1 << 7,
  Occlusion = 1 << 8,
  Physics = 1 << 9,
  AI = 1 << 10,
  All = 0xFFFFFFFF
};

/**
 * @brief Plugin hook points for editor extensibility.
 */
enum class PluginHook : uint8_t {
  OnEditorInit = 0,
  OnEditorShutdown = 1,
  OnSceneLoaded = 2,
  OnSceneSaved = 3,
  OnEntitySelected = 4,
  OnEntityCreated = 5,
  OnEntityDeleted = 6,
  OnComponentAdded = 7,
  OnComponentRemoved = 8,
  OnPlayModeEnter = 9,
  OnPlayModeExit = 10,
  OnViewportRender = 11,
  OnUpdate = 12
};

/**
 * @brief Console log entry.
 */
struct ConsoleLogEntry {
  const char* message = nullptr;
  LogLevel level = LogLevel::Info;
  uint64_t timestamp = 0;    ///< Unix timestamp in milliseconds
  const char* category = nullptr;
  int frameCount = 0;
};

/**
 * @brief Console log entry with C++ string support.
 */
struct ConsoleEntry {
  std::string message;
  LogLevel level = LogLevel::Info;
  uint64_t timestamp = 0;
  std::string category;
  int frameCount = 0;
  int count = 1;  ///< Collapse count
};

/**
 * @brief Gizmo axis hover state.
 */
struct GizmoHoverState {
  bool hoveringX = false;
  bool hoveringY = false;
  bool hoveringZ = false;
  bool hoveringXY = false;
  bool hoveringXZ = false;
  bool hoveringYZ = false;
  bool hoveringXYZ = false;
};

/**
 * @brief Gizmo operation result.
 */
struct GizmoOperation {
  bool active = false;
  GizmoMode mode = GizmoMode::Translate;
  float delta[3] = {0.0f, 0.0f, 0.0f};
  float newValue[3] = {0.0f, 0.0f, 0.0f};
  GizmoHoverState hover;
};

/**
 * @brief Camera bookmark data.
 */
struct CameraBookmark {
  float position[3] = {0.0f, 0.0f, 0.0f};
  float rotation[4] = {0.0f, 0.0f, 0.0f, 1.0f};  ///< Quaternion
  float target[3] = {0.0f, 0.0f, 0.0f};
  float distance = 10.0f;
  std::string name;
};

/**
 * @brief Selection change event data.
 */
struct SelectionChangeEvent {
  std::vector<IEntity*> previousSelection;
  std::vector<IEntity*> currentSelection;
  bool isAdditive = false;
};

/**
 * @brief Selection filter criteria.
 */
struct SelectionFilter {
  bool selectEntities = true;
  bool selectResources = false;
  bool selectComponents = false;
  int componentTypeFilter = 0;  ///< Component type ID filter
};

/**
 * @brief History action record for enhanced undo/redo.
 */
struct HistoryAction {
  uint64_t id = 0;
  HistoryActionType type = HistoryActionType::Custom;
  std::string description;
  uint64_t timestamp = 0;
  uint64_t targetId = 0;
  std::string propertyName;
  std::function<void()> undo;
  std::function<void()> redo;
  std::vector<HistoryAction> subActions;
};

/**
 * @brief History bookmark for quick navigation.
 */
struct HistoryBookmark {
  uint64_t actionId = 0;
  std::string name;
  uint64_t timestamp = 0;
};

/**
 * @brief Snap configuration settings.
 */
struct SnapConfig {
  bool gridSnapEnabled = false;
  float gridSize = 1.0f;
  float gridOrigin[3] = {0.0f, 0.0f, 0.0f};
  bool rotationSnapEnabled = false;
  float rotationSnapAngle = 15.0f;
  bool scaleSnapEnabled = false;
  float scaleSnapIncrement = 0.1f;
  bool surfaceSnapEnabled = false;
  float surfaceSnapOffset = 0.0f;
  bool vertexSnapEnabled = false;
  float vertexSnapDistance = 0.1f;
  bool edgeSnapEnabled = false;
  float edgeSnapDistance = 0.1f;
  bool pivotSnapEnabled = false;
};

/**
 * @brief Snap operation result.
 */
struct SnapResult {
  float snappedPosition[3] = {0.0f, 0.0f, 0.0f};
  float snappedRotation[3] = {0.0f, 0.0f, 0.0f};
  float snappedScale[3] = {1.0f, 1.0f, 1.0f};
  bool positionSnapped = false;
  bool rotationSnapped = false;
  bool scaleSnapped = false;
};

/**
 * @brief Toolbar button definition.
 */
struct ToolbarButton {
  std::string id;
  std::string tooltip;
  const char* icon = nullptr;
  bool enabled = true;
  bool toggle = false;
  bool toggled = false;
  int groupId = 0;
};

/**
 * @brief Menu item definition.
 */
struct MenuItem {
  std::string label;
  std::string shortcut;
  bool enabled = true;
  bool checked = false;
  bool separator = false;
  int id = 0;  // Menu item ID (matches IMainMenu::ID_* constants)
  bool hasSubmenu = false;
  std::vector<MenuItem> submenuItems;
};

/**
 * @brief Menu definition.
 */
struct MenuDef {
  std::string name;
  std::vector<MenuItem> items;
};

/**
 * @brief Background task info for status bar.
 */
struct BackgroundTask {
  std::string name;
  float progress = 0.0f;
  bool active = false;
  uint32_t id = 0;
};

/**
 * @brief Performance statistics for a frame.
 */
struct FrameStats {
  float frameTimeMs = 0.0f;
  float cpuTimeMs = 0.0f;
  float gpuTimeMs = 0.0f;
  int drawCalls = 0;
  int triangles = 0;
  int vertices = 0;
  size_t memoryUsed = 0;
};

/**
 * @brief Scene statistics.
 */
struct SceneStats {
  int entityCount = 0;
  int rootEntityCount = 0;
  int meshCount = 0;
  int lightCount = 0;
  int cameraCount = 0;
  int audioSourceCount = 0;
};

/**
 * @brief Editor action result.
 */
struct EditorResult {
  bool success = false;
  const char* message = nullptr;
};

/**
 * @brief Rectangle for UI layout.
 */
struct EditorRect {
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

/**
 * @brief Key binding definition.
 */
struct KeyBinding {
  int keyCode = 0;
  bool ctrl = false;
  bool alt = false;
  bool shift = false;
};

/**
 * @brief Asset drag-drop payload.
 */
struct AssetDragPayload {
  uint64_t resourceId;
  const char* assetPath;
  int resourceType;  ///< ResourceType enum value
};

/**
 * @brief Panel visibility state.
 */
struct PanelVisibility {
  std::string panelId;
  bool visible = true;
  bool docked = true;
  EditorRect dockRect;
};

/**
 * @brief Layout definition.
 */
struct LayoutDef {
  std::string name;
  ViewportLayout viewportLayout = ViewportLayout::Single;
  std::vector<PanelVisibility> panels;
  bool isBuiltin = false;
};

/**
 * @brief Auto-save settings.
 */
struct AutoSaveSettings {
  bool enabled = false;
  int intervalSeconds = 300;
  int maxBackups = 5;
};

/**
 * @brief Viewport preferences.
 */
struct ViewportPreferences {
  float fov = 60.0f;
  float nearClip = 0.1f;
  float farClip = 1000.0f;
  float cameraSpeed = 1.0f;
  float rotationSpeed = 0.5f;
  bool showGrid = true;
  float gridSize = 1.0f;
  bool showStats = false;
};

/**
 * @brief UI preferences.
 */
struct UIPreferences {
  EditorTheme theme = EditorTheme::Dark;
  int fontSize = 14;
  float uiScale = 1.0f;
  bool showTooltips = true;
  int undoHistorySize = 50;
};

/**
 * @brief External tool configuration.
 */
struct ExternalTool {
  std::string name;
  std::string path;
  std::string arguments;
};

/**
 * @brief Profiler scope timing.
 */
struct ProfilerScope {
  std::string name;
  float startTimeMs = 0.0f;
  float endTimeMs = 0.0f;
  float durationMs = 0.0f;
  int depth = 0;
  uint32_t color = 0;
};

/**
 * @brief Frame timing history entry.
 */
struct FrameTimingEntry {
  float frameTimeMs = 0.0f;
  float cpuTimeMs = 0.0f;
  float gpuTimeMs = 0.0f;
  int drawCalls = 0;
  int triangles = 0;
};

/**
 * @brief Component type statistics.
 */
struct ComponentStats {
  std::string typeName;
  int count = 0;
  int enabledCount = 0;
};

/**
 * @brief Key combination for shortcuts.
 */
struct KeyCombo {
  int keyCode = 0;
  bool ctrl = false;
  bool alt = false;
  bool shift = false;

  bool operator==(KeyCombo const& other) const {
    return keyCode == other.keyCode &&
           ctrl == other.ctrl &&
           alt == other.alt &&
           shift == other.shift;
  }

  bool operator!=(KeyCombo const& other) const {
    return !(*this == other);
  }
};

/**
 * @brief Key binding action definition.
 */
struct KeyBindingAction {
  std::string id;
  std::string displayName;
  std::string category;
  KeyCombo defaultBinding;
  KeyCombo currentBinding;
  std::function<void()> callback;
  bool enabled = true;
};

/**
 * @brief Editor command context.
 */
struct CommandContext {
  IEditor* editor = nullptr;
  ISelectionManager* selection = nullptr;
  IEntity* activeObject = nullptr;
};

/**
 * @brief Editor command definition.
 */
struct EditorCommand {
  std::string id;
  std::string displayName;
  std::string category;
  std::string menuItem;
  int priority = 0;
  std::function<void(const CommandContext&)> execute;
  std::function<bool(const CommandContext&)> canExecute;
  std::function<bool()> isChecked;
  bool visible = true;
  bool enabled = true;
};

/**
 * @brief Editor macro definition.
 */
struct EditorMacro {
  std::string name;
  std::vector<std::string> commandIds;
  bool enabled = true;
};

/**
 * @brief Plugin metadata.
 */
struct PluginInfo {
  std::string id;
  std::string name;
  std::string version;
  std::string author;
  std::string description;
  std::string path;
  bool enabled = false;
  bool loaded = false;
  bool hasError = false;
  std::string errorText;
};

/**
 * @brief Plugin menu item.
 */
struct PluginMenuItem {
  std::string path;
  std::string label;
  std::string shortcut;
  bool enabled = true;
  bool checked = false;
  std::function<void()> callback;
};

/**
 * @brief Plugin panel definition.
 */
struct PluginPanel {
  std::string id;
  std::string name;
  bool visible = true;
  bool canClose = true;
  bool canFloat = true;
  std::function<void()> onDraw;
};

/**
 * @brief Prefab instance data.
 */
struct PrefabInstance {
  uint64_t prefabId = 0;
  uint64_t instanceEntity = 0;
  bool isDirty = false;
  float localPosition[3] = {0.0f, 0.0f, 0.0f};
  float localRotation[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  float localScale[3] = {1.0f, 1.0f, 1.0f};
};

/**
 * @brief Prefab override data.
 */
struct PrefabOverride {
  int type = 0;  ///< Override type
  int componentId = 0;
  std::string propertyName;
};

/**
 * @brief Prefab asset data.
 */
struct PrefabData {
  uint64_t id = 0;
  std::string name;
  std::string path;
  std::vector<std::string> componentTypes;
  uint64_t lastModified = 0;
  int version = 1;
};

/**
 * @brief Scene search filter.
 */
struct SceneSearchFilter {
  std::string namePattern;
  bool caseSensitive = false;
  bool searchComponents = false;
  bool searchInactive = false;
  int componentTypeFilter = 0;
  int layerFilter = -1;  ///< -1 = all layers
  std::string tagFilter;
};

/**
 * @brief Multi-selection transform state.
 */
struct MultiSelectionTransform {
  float position[3] = {0.0f, 0.0f, 0.0f};
  float rotation[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  float scale[3] = {1.0f, 1.0f, 1.0f};
  PivotMode pivotMode = PivotMode::Center;
  CoordinateSpace transformSpace = CoordinateSpace::World;
};

/**
 * @brief Frame timing statistics.
 */
struct FrameTimingStats {
  float cpuTimeMs = 0.0f;
  float gpuTimeMs = 0.0f;
  float totalFrameTimeMs = 0.0f;
  float fps = 0.0f;
  std::vector<float> cpuTimeHistory;
  std::vector<float> gpuTimeHistory;
  int historyIndex = 0;
};

/**
 * @brief Rendering statistics.
 */
struct RenderStats {
  int drawCalls = 0;
  int triangles = 0;
  int vertices = 0;
  int textureBindings = 0;
  int shaderBindings = 0;
  int bufferBindings = 0;
  size_t vramUsed = 0;
  size_t vramBudget = 0;
};

/**
 * @brief Physics statistics.
 */
struct PhysicsStats {
  int activeBodies = 0;
  int sleepingBodies = 0;
  int totalColliders = 0;
  int activeContacts = 0;
  float physicsTimeMs = 0.0f;
};

/**
 * @brief Complete viewport statistics.
 */
struct ViewportStatistics {
  FrameTimingStats timing;
  RenderStats render;
  SceneStats scene;
  PhysicsStats physics;
  uint64_t frameNumber = 0;
  float totalTime = 0.0f;
};

/**
 * @brief Stats display settings.
 */
struct StatsDisplaySettings {
  bool showFPS = true;
  bool showFrameTime = true;
  bool showDrawCalls = true;
  bool showTriangleCount = false;
  bool showVertexCount = false;
  bool showMemoryUsage = false;
  bool showEntityCount = true;
  bool showLightCount = false;
  bool showPhysicsStats = false;
  bool showGraphs = false;
  float position[2] = {10.0f, 10.0f};
  float opacity = 0.8f;
  bool showBackground = true;
  int fontSize = 12;
};

/**
 * @brief Collision visualization settings.
 */
struct CollisionVisSettings {
  bool showColliders = true;
  bool showTriggers = true;
  bool showStatic = true;
  bool showDynamic = true;
  bool showContacts = false;
  bool showSleeping = false;
  float colliderColor[4] = {0.0f, 1.0f, 0.0f, 1.0f};
  float triggerColor[4] = {1.0f, 1.0f, 0.0f, 0.5f};
  float contactColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  float opacity = 0.5f;
  float lineWidth = 1.0f;
};

/**
 * @brief Navigation visualization settings.
 */
struct NavigationVisSettings {
  bool showMesh = true;
  bool showLinks = true;
  bool showObstacles = true;
  bool showPath = true;
  bool showLabels = false;
  float walkableColor[4] = {0.0f, 0.5f, 1.0f, 0.5f};
  float obstacleColor[4] = {1.0f, 0.0f, 0.0f, 0.8f};
  float linkColor[4] = {0.0f, 1.0f, 0.0f, 1.0f};
  float pathColor[4] = {1.0f, 1.0f, 0.0f, 1.0f};
  float opacity = 0.5f;
};

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_EDITOR_TYPES_H
