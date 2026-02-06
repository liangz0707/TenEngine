/**
 * @file Application.cpp
 * @brief Application implementation.
 */
#include "te/application/Application.h"
#include "ApplicationImpl.h"
#include "te/core/log.h"
#include "te/core/check.h"
#include "te/core/platform.h"
#include "te/application/Platform.h"
#include <algorithm>
#include <thread>
#include <chrono>

namespace te {
namespace application {

/**
 * @brief Application implementation class.
 */
class Application : public IApplication {
 public:
  Application() 
    : m_nextWindowId(1)
    , m_mainWindowId(InvalidWindowId)
    , m_windowPlatform(nullptr)
    , m_eventPumpPlatform(nullptr)
    , m_runMode(RunMode::Game)
    , m_isRunning(false)
    , m_isPaused(false)
    , m_exitCode(0)
    , m_timeStepMode(TimeStepMode::Variable)
    , m_targetFPS(0)
    , m_deltaTime(0.0f)
    , m_totalTime(0.0f)
    , m_frameCount(0)
    , m_nextCallbackId(1)
    , m_lastFrameTime(0.0)
  {}

  ~Application() override {
    // Clean up all windows
    for (auto& pair : m_windows) {
      if (m_windowPlatform) {
        m_windowPlatform->DestroyNativeWindow(pair.second.nativeHandle);
      }
    }
    m_windows.clear();

    // Clean up platform interfaces
    delete m_windowPlatform;
    delete m_eventPumpPlatform;
  }

  // ========== Lifecycle ==========

  bool Initialize(InitParams const* params) override {
    if (m_isRunning) {
      te::core::Log(te::core::LogLevel::Warn, "Application already initialized");
      return true;
    }

    // Create platform interfaces
    m_windowPlatform = CreateWindowPlatform();
    if (!m_windowPlatform) {
      te::core::Log(te::core::LogLevel::Error, "Failed to create window platform");
      return false;
    }

    m_eventPumpPlatform = CreateEventPumpPlatform();
    if (!m_eventPumpPlatform) {
      te::core::Log(te::core::LogLevel::Error, "Failed to create event pump platform");
      delete m_windowPlatform;
      m_windowPlatform = nullptr;
      return false;
    }

    // Initialize time
    m_lastFrameTime = te::core::HighResolutionTimer();

    te::core::Log(te::core::LogLevel::Info, "Application initialized");
    return true;
  }

  void Run(RunParams const& args) override {
    if (!m_windowPlatform || !m_eventPumpPlatform) {
      te::core::Log(te::core::LogLevel::Error, "Application not initialized");
      return;
    }

    m_isRunning = true;
    m_runMode = args.runMode;

    // Create main window if not headless
    if (m_runMode != RunMode::Headless) {
      WindowDesc desc;
      desc.title = args.windowTitle;
      desc.width = args.windowWidth;
      desc.height = args.windowHeight;
      WindowId mainWindow = CreateWindow(desc);
      if (mainWindow == InvalidWindowId) {
        te::core::Log(te::core::LogLevel::Error, "Failed to create main window");
        m_isRunning = false;
        return;
      }
      m_mainWindowId = mainWindow;
    }

    // Register legacy tick callback if provided
    if (args.tickCallback) {
      RegisterTickCallback(args.tickCallback, 0);
    }

    // Main loop
    while (m_isRunning) {
      double currentTime = te::core::HighResolutionTimer();
      m_deltaTime = static_cast<float>(currentTime - m_lastFrameTime);
      m_lastFrameTime = currentTime;
      m_totalTime += m_deltaTime;

      // Pump events
      PumpEvents();

      // Check exit condition
      if (!m_isRunning) {
        break;
      }

      // Execute tick callbacks if not paused
      if (!m_isPaused) {
        // Sort callbacks by priority (higher priority first)
        te::core::Array<std::pair<TickCallbackId, TickCallbackData>> sortedCallbacks;
        for (const auto& pair : m_tickCallbacks) {
          sortedCallbacks.push_back(pair);
        }
        std::sort(sortedCallbacks.begin(), sortedCallbacks.end(),
                  [](const auto& a, const auto& b) {
                    return a.second.priority > b.second.priority;
                  });

        // Execute callbacks
        for (const auto& pair : sortedCallbacks) {
          if (pair.second.callback) {
            pair.second.callback(m_deltaTime);
          }
        }
      }

      // Frame rate control
      if (m_targetFPS > 0) {
        double targetFrameTime = 1.0 / static_cast<double>(m_targetFPS);
        double elapsed = te::core::HighResolutionTimer() - currentTime;
        if (elapsed < targetFrameTime) {
          double sleepTime = targetFrameTime - elapsed;
          std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
        }
      }

      m_frameCount++;
    }

    // Cleanup
    te::core::Log(te::core::LogLevel::Info, "Application main loop exited");
  }

  void Pause() override {
    m_isPaused = true;
  }

  void Resume() override {
    m_isPaused = false;
  }

  void RequestExit(int exitCode) override {
    m_exitCode = exitCode;
    m_isRunning = false;
  }

  int GetExitCode() const override {
    return m_exitCode;
  }

  RunMode GetRunMode() const override {
    return m_runMode;
  }

  bool SetRunMode(RunMode mode) override {
    // Simple mode switch, may require restart for some modes
    m_runMode = mode;
    return true;
  }

  // ========== Window Management ==========

  WindowId CreateWindow(WindowDesc const& desc) override {
    if (!desc.IsValid()) {
      te::core::Log(te::core::LogLevel::Error, "Invalid window description");
      return InvalidWindowId;
    }

    if (!m_windowPlatform) {
      te::core::Log(te::core::LogLevel::Error, "Window platform not initialized");
      return InvalidWindowId;
    }

    void* nativeHandle = m_windowPlatform->CreateNativeWindow(desc);
    if (!nativeHandle) {
      te::core::Log(te::core::LogLevel::Error, "Failed to create native window");
      return InvalidWindowId;
    }

    WindowId windowId = m_nextWindowId++;
    WindowData windowData;
    windowData.nativeHandle = nativeHandle;
    windowData.desc = desc;
    m_windows[windowId] = windowData;

    // Set as main window if first window
    if (m_mainWindowId == InvalidWindowId) {
      m_mainWindowId = windowId;
    }

    // Send WindowCreated event
    Event event = {};
    event.type = EventType::WindowCreated;
    event.windowId = windowId;
    event.timestamp = static_cast<float>(te::core::HighResolutionTimer());
    PushEvent(event);

    return windowId;
  }

  void DestroyWindow(WindowId windowId) override {
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) {
      te::core::Log(te::core::LogLevel::Warn, "Window not found");
      return;
    }

    if (m_windowPlatform) {
      m_windowPlatform->DestroyNativeWindow(it->second.nativeHandle);
    }

    // Send WindowDestroyed event
    Event event = {};
    event.type = EventType::WindowDestroyed;
    event.windowId = windowId;
    event.timestamp = static_cast<float>(te::core::HighResolutionTimer());
    PushEvent(event);

    m_windows.erase(it);

    // Update main window if destroyed
    if (m_mainWindowId == windowId) {
      m_mainWindowId = m_windows.empty() ? InvalidWindowId : m_windows.begin()->first;
    }
  }

  WindowId GetMainWindow() const override {
    return m_mainWindowId;
  }

  void SetWindowTitle(WindowId windowId, char const* title) override {
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) {
      return;
    }

    if (m_windowPlatform) {
      m_windowPlatform->SetWindowTitle(it->second.nativeHandle, title);
    }
    it->second.desc.title = title;
  }

  void SetWindowSize(WindowId windowId, uint32_t width, uint32_t height) override {
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) {
      return;
    }

    if (m_windowPlatform) {
      m_windowPlatform->SetWindowSize(it->second.nativeHandle, width, height);
    }
    it->second.desc.width = width;
    it->second.desc.height = height;
  }

  void SetWindowPosition(WindowId windowId, int32_t x, int32_t y) override {
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) {
      return;
    }

    if (m_windowPlatform) {
      m_windowPlatform->SetWindowPosition(it->second.nativeHandle, x, y);
    }
    it->second.desc.x = x;
    it->second.desc.y = y;
  }

  void SetFullscreen(WindowId windowId, bool fullscreen) override {
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) {
      return;
    }

    if (m_windowPlatform) {
      m_windowPlatform->SetFullscreen(it->second.nativeHandle, fullscreen);
    }
    it->second.desc.fullscreen = fullscreen;
  }

  void* GetNativeHandle(WindowId windowId) const override {
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) {
      return nullptr;
    }
    return it->second.nativeHandle;
  }

  DisplayInfo GetDisplayInfo(uint32_t displayIndex) const override {
    if (m_windowPlatform) {
      return m_windowPlatform->GetDisplayInfo(displayIndex);
    }
    return DisplayInfo{};
  }

  uint32_t EnumerateDisplays(DisplayInfo* displays, uint32_t maxCount) const override {
    if (m_windowPlatform) {
      return m_windowPlatform->EnumerateDisplays(displays, maxCount);
    }
    return 0;
  }

  void SetWindowCallback(WindowId windowId, WindowCallback callback) override {
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) {
      return;
    }
    it->second.callback = callback;
  }

  // ========== Event System ==========

  void PumpEvents() override {
    if (!m_eventPumpPlatform) {
      return;
    }

    PlatformEvent platformEvent = {};
    while (m_eventPumpPlatform->PollEvent(platformEvent)) {
      // Check for quit event
      if (m_eventPumpPlatform->IsQuitEvent(platformEvent)) {
        RequestExit(0);
        continue;
      }

      // Convert to engine event
      // Find window ID from native handle (simplified - assumes main window)
      WindowId windowId = m_mainWindowId;
      Event engineEvent = m_eventPumpPlatform->ConvertToEngineEvent(platformEvent, windowId);

      // Clean up platform event data
      if (platformEvent.data) {
        delete static_cast<void*>(platformEvent.data);
      }

      // Push to queue
      PushEvent(engineEvent);

      // Handle window callbacks
      if (engineEvent.type == EventType::WindowResized ||
          engineEvent.type == EventType::WindowMoved ||
          engineEvent.type == EventType::WindowFocused ||
          engineEvent.type == EventType::WindowClosed) {
        auto it = m_windows.find(windowId);
        if (it != m_windows.end() && it->second.callback) {
          WindowEvent windowEvent = {};
          windowEvent.windowId = windowId;
          switch (engineEvent.type) {
            case EventType::WindowResized:
              windowEvent.type = WindowEventType::Resized;
              break;
            case EventType::WindowMoved:
              windowEvent.type = WindowEventType::Moved;
              break;
            case EventType::WindowFocused:
              windowEvent.type = WindowEventType::Focused;
              break;
            case EventType::WindowClosed:
              windowEvent.type = WindowEventType::Closed;
              break;
            default:
              break;
          }
          it->second.callback(windowId, &windowEvent);
        }
      }
    }
  }

  void PushEvent(Event const& event) override {
    m_eventQueue.Push(event);
  }

  EventQueue const& GetEventQueue() const override {
    return m_eventQueue;
  }

  EventQueue& GetEventQueue() override {
    return m_eventQueue;
  }

  // ========== Main Loop ==========

  float GetDeltaTime() const override {
    return m_deltaTime;
  }

  float GetTotalTime() const override {
    return m_totalTime;
  }

  uint64_t GetFrameCount() const override {
    return m_frameCount;
  }

  void SetTargetFPS(uint32_t fps) override {
    m_targetFPS = fps;
  }

  void SetTimeStepMode(TimeStepMode mode) override {
    m_timeStepMode = mode;
  }

  TickCallbackId RegisterTickCallback(TickCallback callback, int32_t priority) override {
    if (!callback) {
      return 0;
    }

    TickCallbackId id = m_nextCallbackId++;
    TickCallbackData data;
    data.callback = callback;
    data.priority = priority;
    m_tickCallbacks[id] = data;
    return id;
  }

  void UnregisterTickCallback(TickCallbackId callbackId) override {
    m_tickCallbacks.erase(callbackId);
  }

 private:
  EventQueue m_eventQueue;
  te::core::Map<WindowId, WindowData> m_windows;
  WindowId m_nextWindowId;
  WindowId m_mainWindowId;
  IWindowPlatform* m_windowPlatform;
  IEventPumpPlatform* m_eventPumpPlatform;
  RunMode m_runMode;
  bool m_isRunning;
  bool m_isPaused;
  int m_exitCode;
  TimeStepMode m_timeStepMode;
  uint32_t m_targetFPS;
  float m_deltaTime;
  float m_totalTime;
  uint64_t m_frameCount;
  TickCallbackId m_nextCallbackId;
  te::core::Map<TickCallbackId, TickCallbackData> m_tickCallbacks;
  double m_lastFrameTime;
};

IApplication* CreateApplication() {
  try {
    return new Application();
  } catch (...) {
    te::core::Log(te::core::LogLevel::Error, "Exception creating application");
    return nullptr;
  }
}

}  // namespace application
}  // namespace te
