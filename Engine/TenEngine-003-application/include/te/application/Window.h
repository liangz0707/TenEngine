/**
 * @file Window.h
 * @brief Window-related types and structures (contract: specs/_contracts/003-application-ABI.md).
 */
#ifndef TE_APPLICATION_WINDOW_H
#define TE_APPLICATION_WINDOW_H

#include <cstdint>

namespace te {
namespace application {

/** Window ID type. InvalidWindowId = 0. */
using WindowId = uint32_t;

/** Invalid window ID constant. */
constexpr WindowId InvalidWindowId = 0;

/**
 * @brief Window description for creation per contract.
 * 
 * Default values provide a standard windowed mode window.
 * Use -1 for x/y to center the window on the specified display.
 */
struct WindowDesc {
  /** Window title. */
  char const* title = "Window";
  
  /** Window width in pixels. */
  uint32_t width = 800;
  
  /** Window height in pixels. */
  uint32_t height = 600;
  
  /** Window X position (-1 means center horizontally). */
  int32_t x = -1;
  
  /** Window Y position (-1 means center vertically). */
  int32_t y = -1;
  
  /** Whether window is resizable. */
  bool resizable = true;
  
  /** Whether window has no border. */
  bool borderless = false;
  
  /** Whether window stays on top of other windows. */
  bool alwaysOnTop = false;
  
  /** Whether window starts in fullscreen mode. */
  bool fullscreen = false;
  
  /** Display index to create window on (0 = primary display). */
  uint32_t displayIndex = 0;

  /**
   * @brief Validate window description.
   * @return true if valid, false otherwise
   */
  bool IsValid() const {
    return width > 0 && height > 0 && title != nullptr;
  }
};

/**
 * @brief Window event type.
 */
enum class WindowEventType {
    Created,
    Destroyed,
    Resized,
    Moved,
    Focused,
    Unfocused,
    Closed,
    Minimized,
    Maximized,
    Restored
};

/**
 * @brief Window event structure.
 */
struct WindowEvent {
    WindowEventType type;
    WindowId windowId;
    union {
        struct {
            uint32_t width;
            uint32_t height;
        } resized;
        struct {
            int32_t x;
            int32_t y;
        } moved;
        // Other event data...
    };
};

/**
 * @brief Window event callback function type per contract.
 * @param windowId Window ID
 * @param event Window event pointer (cast to WindowEvent const*)
 */
using WindowCallback = void (*)(WindowId windowId, void const* event);

/**
 * @brief Display information structure per contract.
 */
struct DisplayInfo {
  /** Display index. */
  uint32_t index = 0;
  
  /** Display X position in virtual screen coordinates. */
  int32_t x = 0;
  
  /** Display Y position in virtual screen coordinates. */
  int32_t y = 0;
  
  /** Display width in pixels. */
  uint32_t width = 0;
  
  /** Display height in pixels. */
  uint32_t height = 0;
  
  /** Horizontal DPI. */
  float dpiX = 96.0f;
  
  /** Vertical DPI. */
  float dpiY = 96.0f;
  
  /** Refresh rate in Hz. */
  float refreshRate = 60.0f;
  
  /** Whether this is the primary display. */
  bool primary = false;
};

}  // namespace application
}  // namespace te

#endif  // TE_APPLICATION_WINDOW_H
