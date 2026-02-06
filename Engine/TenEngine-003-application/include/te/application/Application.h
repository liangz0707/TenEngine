/**
 * @file Application.h
 * @brief Application lifecycle, window management, event system, and main loop (contract: specs/_contracts/003-application-ABI.md).
 * Only types and functions declared in the contract are exposed.
 */
#ifndef TE_APPLICATION_APPLICATION_H
#define TE_APPLICATION_APPLICATION_H

#include "Window.h"
#include "Event.h"
#include "MainLoop.h"
#include <cstdint>

namespace te {
namespace application {

/**
 * @brief Run mode enumeration per contract.
 */
enum class RunMode {
  Editor,   // Editor mode
  Game,     // Game mode
  Headless  // Headless mode (no window)
};

/**
 * @brief Initialization parameters per contract.
 * Optional parameters for application initialization.
 */
struct InitParams {
  /** Command line argument count. */
  int argc = 0;
  
  /** Command line arguments. */
  char const** argv = nullptr;
  
  /** Configuration file path (optional). */
  char const* configPath = nullptr;
};

/**
 * @brief Run parameters per contract.
 * Parameters for application main loop execution.
 */
struct RunParams {
  /** Window title for main window. */
  char const* windowTitle = "TenEngine";
  
  /** Window width in pixels. */
  uint32_t windowWidth = 1280;
  
  /** Window height in pixels. */
  uint32_t windowHeight = 720;
  
  /** Run mode (Editor/Game/Headless). */
  RunMode runMode = RunMode::Game;
  
  /** Optional tick callback (legacy support). */
  TickCallback tickCallback = nullptr;
};

/**
 * @brief Application main interface per contract.
 * 
 * Integrates window management, event system, and main loop functionality.
 * Design principle: Single interface with complete functionality, similar to Core module's Allocator pattern.
 */
class IApplication {
 public:
  virtual ~IApplication() = default;

  // ========== Lifecycle ==========

  /**
   * @brief Initialize application, parse parameters, prepare subsystems per contract.
   * @param params Optional initialization parameters
   * @return true on success, false on failure
   */
  virtual bool Initialize(InitParams const* params = nullptr) = 0;

  /**
   * @brief Enter main loop until exit per contract. Blocking call.
   * @param args Run parameters
   */
  virtual void Run(RunParams const& args) = 0;

  /**
   * @brief Pause application per contract (stop ticking, but keep windows and resources).
   */
  virtual void Pause() = 0;

  /**
   * @brief Resume application per contract.
   */
  virtual void Resume() = 0;

  /**
   * @brief Request exit, set exit code per contract. Main loop will exit next frame.
   * @param exitCode Exit code (default: 0)
   */
  virtual void RequestExit(int exitCode = 0) = 0;

  /**
   * @brief Get exit code per contract.
   * @return Exit code
   */
  virtual int GetExitCode() const = 0;

  /**
   * @brief Get current run mode per contract.
   * @return RunMode (Editor/Game/Headless)
   */
  virtual RunMode GetRunMode() const = 0;

  /**
   * @brief Set run mode per contract. Some mode switches may require restart.
   * @param mode RunMode to switch to
   * @return true on success, false if switch not supported
   */
  virtual bool SetRunMode(RunMode mode) = 0;

  // ========== Window Management ==========

  /**
   * @brief Create window per contract.
   * @param desc Window description
   * @return WindowId on success, InvalidWindowId (0) on failure
   */
  virtual WindowId CreateWindow(WindowDesc const& desc) = 0;

  /**
   * @brief Destroy window per contract.
   * @param windowId Window ID to destroy
   */
  virtual void DestroyWindow(WindowId windowId) = 0;

  /**
   * @brief Get main window ID per contract.
   * @return Main window ID
   */
  virtual WindowId GetMainWindow() const = 0;

  /**
   * @brief Set window title per contract.
   * @param windowId Window ID
   * @param title Window title
   */
  virtual void SetWindowTitle(WindowId windowId, char const* title) = 0;

  /**
   * @brief Set window size per contract.
   * @param windowId Window ID
   * @param width Window width
   * @param height Window height
   */
  virtual void SetWindowSize(WindowId windowId, uint32_t width, uint32_t height) = 0;

  /**
   * @brief Set window position per contract.
   * @param windowId Window ID
   * @param x Window X position
   * @param y Window Y position
   */
  virtual void SetWindowPosition(WindowId windowId, int32_t x, int32_t y) = 0;

  /**
   * @brief Set fullscreen mode per contract.
   * @param windowId Window ID
   * @param fullscreen true for fullscreen, false for windowed
   */
  virtual void SetFullscreen(WindowId windowId, bool fullscreen) = 0;

  /**
   * @brief Get native platform window handle per contract.
   * @param windowId Window ID
   * @return Native handle (HWND/X11 Window/NSWindow/etc.) or nullptr
   */
  virtual void* GetNativeHandle(WindowId windowId) const = 0;

  /**
   * @brief Get display information per contract.
   * @param displayIndex Display index
   * @return DisplayInfo structure
   */
  virtual DisplayInfo GetDisplayInfo(uint32_t displayIndex) const = 0;

  /**
   * @brief Enumerate all displays per contract.
   * @param displays Output array for display info
   * @param maxCount Maximum number of displays to return
   * @return Actual number of displays
   */
  virtual uint32_t EnumerateDisplays(DisplayInfo* displays, uint32_t maxCount) const = 0;

  /**
   * @brief Set window event callback per contract.
   * @param windowId Window ID
   * @param callback Window event callback function
   */
  virtual void SetWindowCallback(WindowId windowId, WindowCallback callback) = 0;

  // ========== Event System ==========

  /**
   * @brief Pump and process platform messages per contract. Should be called every frame in main loop.
   */
  virtual void PumpEvents() = 0;

  /**
   * @brief Push event to queue manually per contract (optional, for custom events).
   * @param event Event to push
   */
  virtual void PushEvent(Event const& event) = 0;

  /**
   * @brief Get event queue per contract (for Input module to consume directly).
   * @return Reference to event queue
   */
  virtual EventQueue const& GetEventQueue() const = 0;

  // ========== Main Loop ==========

  /**
   * @brief Get delta time of last frame per contract (in seconds).
   * @return Delta time
   */
  virtual float GetDeltaTime() const = 0;

  /**
   * @brief Get total running time per contract (in seconds).
   * @return Total time
   */
  virtual float GetTotalTime() const = 0;

  /**
   * @brief Get frame count per contract.
   * @return Frame count
   */
  virtual uint64_t GetFrameCount() const = 0;

  /**
   * @brief Set target FPS per contract.
   * @param fps Target FPS (0 means unlimited)
   */
  virtual void SetTargetFPS(uint32_t fps) = 0;

  /**
   * @brief Set time step mode per contract.
   * @param mode TimeStepMode (Fixed/Variable/Mixed), Variable is default
   */
  virtual void SetTimeStepMode(TimeStepMode mode) = 0;

  /**
   * @brief Register tick callback with priority per contract.
   * @param callback Tick callback function
   * @param priority Priority (higher priority executes first, default: 0)
   * @return TickCallbackId for unregistering
   */
  virtual TickCallbackId RegisterTickCallback(TickCallback callback, int32_t priority = 0) = 0;

  /**
   * @brief Unregister tick callback per contract.
   * @param callbackId Tick callback ID returned from RegisterTickCallback
   */
  virtual void UnregisterTickCallback(TickCallbackId callbackId) = 0;
};

/**
 * @brief Create application instance per contract.
 * @return IApplication pointer, or nullptr on failure. Caller responsible for deletion or engine manages.
 */
IApplication* CreateApplication();

}  // namespace application
}  // namespace te

#endif  // TE_APPLICATION_APPLICATION_H
