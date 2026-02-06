/**
 * @file WindowsEventPumpPlatform.cpp
 * @brief Windows platform event pump implementation.
 */
#include "WindowsEventPumpPlatform.h"
#include "te/core/platform.h"
#include <windows.h>

namespace te {
namespace application {

WindowsEventPumpPlatform::WindowsEventPumpPlatform() = default;

WindowsEventPumpPlatform::~WindowsEventPumpPlatform() = default;

bool WindowsEventPumpPlatform::PollEvent(PlatformEvent& event) {
  MSG msg;
  if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    // Store message data in platform event
    // Note: Caller is responsible for cleaning up the data after ConvertToEngineEvent
    WindowsMessage* winMsg = new WindowsMessage();
    winMsg->msg = msg;
    event.data = winMsg;
    return true;
  }
  event.data = nullptr;
  return false;
}

Event WindowsEventPumpPlatform::ConvertToEngineEvent(PlatformEvent const& platformEvent, WindowId windowId) {
  Event engineEvent = {};
  engineEvent.windowId = windowId;
  engineEvent.timestamp = static_cast<float>(te::core::HighResolutionTimer());

  if (!platformEvent.data) {
    engineEvent.type = EventType::AppWillTerminate;
    return engineEvent;
  }

  WindowsMessage* winMsg = static_cast<WindowsMessage*>(platformEvent.data);
  MSG& msg = winMsg->msg;

  switch (msg.message) {
    case WM_QUIT:
      engineEvent.type = EventType::AppWillTerminate;
      break;

    case WM_KEYDOWN:
      engineEvent.type = EventType::KeyDown;
      engineEvent.key.keyCode = static_cast<uint32_t>(msg.wParam);
      break;

    case WM_KEYUP:
      engineEvent.type = EventType::KeyUp;
      engineEvent.key.keyCode = static_cast<uint32_t>(msg.wParam);
      break;

    case WM_MOUSEMOVE:
      engineEvent.type = EventType::MouseMove;
      engineEvent.mouse.x = LOWORD(msg.lParam);
      engineEvent.mouse.y = HIWORD(msg.lParam);
      break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
      engineEvent.type = EventType::MouseButtonDown;
      engineEvent.mouse.x = LOWORD(msg.lParam);
      engineEvent.mouse.y = HIWORD(msg.lParam);
      break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
      engineEvent.type = EventType::MouseButtonUp;
      engineEvent.mouse.x = LOWORD(msg.lParam);
      engineEvent.mouse.y = HIWORD(msg.lParam);
      break;

    case WM_MOUSEWHEEL:
      engineEvent.type = EventType::MouseWheel;
      engineEvent.mouse.x = LOWORD(msg.lParam);
      engineEvent.mouse.y = HIWORD(msg.lParam);
      break;

    case WM_SIZE:
      engineEvent.type = EventType::WindowResized;
      break;

    case WM_MOVE:
      engineEvent.type = EventType::WindowMoved;
      break;

    case WM_SETFOCUS:
      engineEvent.type = EventType::WindowFocused;
      break;

    case WM_CLOSE:
      engineEvent.type = EventType::WindowClosed;
      break;

    default:
      engineEvent.type = EventType::AppWillTerminate;  // Unknown event
      break;
  }

  return engineEvent;
}

bool WindowsEventPumpPlatform::IsQuitEvent(PlatformEvent const& platformEvent) const {
  if (!platformEvent.data) {
    return true;
  }

  WindowsMessage* winMsg = static_cast<WindowsMessage*>(platformEvent.data);
  return winMsg->msg.message == WM_QUIT;
}

}  // namespace application
}  // namespace te
