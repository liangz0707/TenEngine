/**
 * @file EditorTypes.h
 * @brief Common types and enumerations for the editor module.
 */
#ifndef TE_EDITOR_EDITOR_TYPES_H
#define TE_EDITOR_EDITOR_TYPES_H

#include <cstdint>

namespace te {
namespace editor {

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

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_EDITOR_TYPES_H
