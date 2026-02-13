/**
 * @file Platform.h
 * @brief Platform abstraction layer interfaces for window and event handling (contract: specs/_contracts/003-application-ABI.md).
 * Platform-specific implementations should implement these interfaces.
 */
#ifndef TE_APPLICATION_PLATFORM_H
#define TE_APPLICATION_PLATFORM_H

#include "Window.h"
#include "Event.h"
#include <cstdint>

namespace te {
namespace application {

// Forward declarations
struct WindowDesc;
struct DisplayInfo;

/**
 * @brief Platform-specific event structure (opaque, platform-dependent).
 * Converted to Event by platform implementation.
 */
struct PlatformEvent {
  void* data;  // Platform-specific event data
};

/**
 * @brief Platform window abstraction interface per contract.
 * 
 * Each platform (Windows/X11/Cocoa/Android/iOS) implements this interface
 * to provide native window creation and management.
 */
struct IWindowPlatform {
  virtual ~IWindowPlatform() = default;

  /**
   * @brief Create native platform window.
   * @param desc Window description
   * @return Native window handle (HWND/X11 Window/NSWindow/etc.) or nullptr on failure
   */
  virtual void* CreateNativeWindow(WindowDesc const& desc) = 0;

  /**
   * @brief Destroy native platform window.
   * @param handle Native window handle
   */
  virtual void DestroyNativeWindow(void* handle) = 0;

  /**
   * @brief Set window title.
   * @param handle Native window handle
   * @param title Window title
   */
  virtual void SetWindowTitle(void* handle, char const* title) = 0;

  /**
   * @brief Set window size.
   * @param handle Native window handle
   * @param width Window width
   * @param height Window height
   */
  virtual void SetWindowSize(void* handle, uint32_t width, uint32_t height) = 0;

  /**
   * @brief Set window position.
   * @param handle Native window handle
   * @param x Window X position
   * @param y Window Y position
   */
  virtual void SetWindowPosition(void* handle, int32_t x, int32_t y) = 0;

  /**
   * @brief Set fullscreen mode.
   * @param handle Native window handle
   * @param fullscreen true for fullscreen, false for windowed
   */
  virtual void SetFullscreen(void* handle, bool fullscreen) = 0;

  /**
   * @brief Get display information for specified display index.
   * @param displayIndex Display index
   * @return DisplayInfo structure
   */
  virtual DisplayInfo GetDisplayInfo(uint32_t displayIndex) const = 0;

  /**
   * @brief Enumerate all displays.
   * @param displays Output array for display info
   * @param maxCount Maximum number of displays to return
   * @return Actual number of displays
   */
  virtual uint32_t EnumerateDisplays(DisplayInfo* displays, uint32_t maxCount) const = 0;

  /**
   * @brief Set optional WndProc handler (Windows only). Used by ImGui to receive input.
   * @param handler Function pointer: LRESULT (*)(HWND, UINT, WPARAM, LPARAM). nullptr to clear.
   */
  virtual void SetWndProcHandler(void* handler) { (void)handler; }
};

/**
 * @brief Platform event pump abstraction interface per contract.
 * 
 * Each platform implements this interface to provide native event polling
 * and conversion to engine Event structures.
 */
struct IEventPumpPlatform {
  virtual ~IEventPumpPlatform() = default;

  /**
   * @brief Poll platform event (non-blocking).
   * @param event Output platform event
   * @return true if event polled, false if no events available
   */
  virtual bool PollEvent(PlatformEvent& event) = 0;

  /**
   * @brief Convert platform event to engine Event.
   * @param platformEvent Platform-specific event
   * @param windowId Window ID associated with the event
   * @return Engine Event structure
   */
  virtual Event ConvertToEngineEvent(PlatformEvent const& platformEvent, WindowId windowId) = 0;

  /**
   * @brief Check if platform event is a quit/close event.
   * @param platformEvent Platform-specific event
   * @return true if quit event
   */
  virtual bool IsQuitEvent(PlatformEvent const& platformEvent) const = 0;
};

/**
 * @brief Create platform-specific window implementation.
 * @return IWindowPlatform pointer, or nullptr if platform not supported
 */
IWindowPlatform* CreateWindowPlatform();

/**
 * @brief Create platform-specific event pump implementation.
 * @return IEventPumpPlatform pointer, or nullptr if platform not supported
 */
IEventPumpPlatform* CreateEventPumpPlatform();

}  // namespace application
}  // namespace te

#endif  // TE_APPLICATION_PLATFORM_H
