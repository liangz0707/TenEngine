/**
 * @file WindowsWindowPlatform.cpp
 * @brief Windows platform window implementation.
 */
#include "WindowsWindowPlatform.h"
#include "te/core/log.h"
#include "te/core/check.h"
#include <windows.h>

namespace te {
namespace application {

bool WindowsWindowPlatform::s_windowClassRegistered = false;

WindowsWindowPlatform::WindowsWindowPlatform() {
  if (!RegisterWindowClass()) {
    te::core::Log(te::core::LogLevel::Error, "Failed to register window class");
  }
}

WindowsWindowPlatform::~WindowsWindowPlatform() = default;

bool WindowsWindowPlatform::RegisterWindowClass() {
  if (s_windowClassRegistered) {
    return true;
  }

  WNDCLASSEX wc = {};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = GetModuleHandle(nullptr);
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszClassName = s_windowClassName;

  if (RegisterClassEx(&wc) == 0) {
    return false;
  }

  s_windowClassRegistered = true;
  return true;
}

LRESULT CALLBACK WindowsWindowPlatform::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}

void* WindowsWindowPlatform::GetWindowUserData(HWND hwnd) {
  return reinterpret_cast<void*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

void WindowsWindowPlatform::SetWindowUserData(HWND hwnd, void* data) {
  SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data));
}

void* WindowsWindowPlatform::CreateNativeWindow(WindowDesc const& desc) {
  if (!desc.IsValid()) {
    te::core::Log(te::core::LogLevel::Error, "Invalid window description");
    return nullptr;
  }

  if (!s_windowClassRegistered && !RegisterWindowClass()) {
    te::core::Log(te::core::LogLevel::Error, "Failed to register window class");
    return nullptr;
  }

  DWORD dwStyle = WS_OVERLAPPEDWINDOW;
  if (desc.borderless) {
    dwStyle = WS_POPUP;
  }
  if (!desc.resizable) {
    dwStyle &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
  }

  int x = desc.x;
  int y = desc.y;
  int width = static_cast<int>(desc.width);
  int height = static_cast<int>(desc.height);

  // Center window if x or y is -1
  if (x == -1 || y == -1) {
    DisplayInfo displayInfo = GetDisplayInfo(desc.displayIndex);
    if (x == -1) {
      x = static_cast<int>(displayInfo.x + (displayInfo.width - width) / 2);
    }
    if (y == -1) {
      y = static_cast<int>(displayInfo.y + (displayInfo.height - height) / 2);
    }
  }

  // Adjust window rect for client area
  RECT rect = {x, y, x + width, y + height};
  AdjustWindowRect(&rect, dwStyle, FALSE);
  width = rect.right - rect.left;
  height = rect.bottom - rect.top;
  x = rect.left;
  y = rect.top;

  HWND hwnd = CreateWindowEx(
      0,
      s_windowClassName,
      desc.title ? desc.title : "Window",
      dwStyle,
      x, y, width, height,
      nullptr,
      nullptr,
      GetModuleHandle(nullptr),
      nullptr
  );

  if (!hwnd) {
    te::core::Log(te::core::LogLevel::Error, "Failed to create window");
    return nullptr;
  }

  if (desc.alwaysOnTop) {
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }

  if (desc.fullscreen) {
    SetFullscreen(hwnd, true);
  }

  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

  return hwnd;
}

void WindowsWindowPlatform::DestroyNativeWindow(void* handle) {
  if (!handle) {
    return;
  }
  HWND hwnd = static_cast<HWND>(handle);
  DestroyWindow(hwnd);
}

void WindowsWindowPlatform::SetWindowTitle(void* handle, char const* title) {
  if (!handle || !title) {
    return;
  }
  HWND hwnd = static_cast<HWND>(handle);
  SetWindowText(hwnd, title);
}

void WindowsWindowPlatform::SetWindowSize(void* handle, uint32_t width, uint32_t height) {
  if (!handle) {
    return;
  }
  HWND hwnd = static_cast<HWND>(handle);
  SetWindowPos(hwnd, nullptr, 0, 0, static_cast<int>(width), static_cast<int>(height),
               SWP_NOMOVE | SWP_NOZORDER);
}

void WindowsWindowPlatform::SetWindowPosition(void* handle, int32_t x, int32_t y) {
  if (!handle) {
    return;
  }
  HWND hwnd = static_cast<HWND>(handle);
  SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void WindowsWindowPlatform::SetFullscreen(void* handle, bool fullscreen) {
  if (!handle) {
    return;
  }
  HWND hwnd = static_cast<HWND>(handle);

  if (fullscreen) {
    // Get window style
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

    // Store original style
    SetWindowLongPtr(hwnd, GWLP_USERDATA, style | (exStyle << 32));

    // Set fullscreen
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (monitor) {
      MONITORINFO mi = {sizeof(MONITORINFO)};
      if (GetMonitorInfo(monitor, &mi)) {
        SetWindowLongPtr(hwnd, GWL_STYLE, style & ~(WS_CAPTION | WS_THICKFRAME));
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
        SetWindowPos(hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_FRAMECHANGED);
      }
    }
  } else {
    // Restore windowed mode
    LONG_PTR userData = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (userData != 0) {
      LONG_PTR style = userData & 0xFFFFFFFF;
      LONG_PTR exStyle = (userData >> 32) & 0xFFFFFFFF;
      SetWindowLongPtr(hwnd, GWL_STYLE, style);
      SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
      SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
  }
}

DisplayInfo WindowsWindowPlatform::GetDisplayInfo(uint32_t displayIndex) const {
  DisplayInfo info = {};
  info.index = displayIndex;

  struct MonitorEnumData {
    DisplayInfo* info;
    uint32_t targetIndex;
    uint32_t currentIndex;
  };

  MonitorEnumData enumData = {&info, displayIndex, 0};

  EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
    MonitorEnumData* data = reinterpret_cast<MonitorEnumData*>(lParam);
    if (data->currentIndex == data->targetIndex) {
      MONITORINFOEX mi = {sizeof(MONITORINFOEX)};
      if (GetMonitorInfo(hMonitor, &mi)) {
        data->info->x = mi.rcMonitor.left;
        data->info->y = mi.rcMonitor.top;
        data->info->width = mi.rcMonitor.right - mi.rcMonitor.left;
        data->info->height = mi.rcMonitor.bottom - mi.rcMonitor.top;
        data->info->primary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;

        // Get DPI
        HMONITOR monitor = MonitorFromPoint({data->info->x, data->info->y}, MONITOR_DEFAULTTONEAREST);
        UINT dpiX = 96, dpiY = 96;
        if (monitor) {
          HDC hdc = GetDC(nullptr);
          if (hdc) {
            dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
            dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(nullptr, hdc);
          }
        }
        data->info->dpiX = static_cast<float>(dpiX);
        data->info->dpiY = static_cast<float>(dpiY);
        data->info->refreshRate = 60.0f;  // Default, could query from DEVMODE
      }
      return FALSE;  // Stop enumeration
    }
    data->currentIndex++;
    return TRUE;  // Continue enumeration
  }, reinterpret_cast<LPARAM>(&enumData));

  return info;
}

uint32_t WindowsWindowPlatform::EnumerateDisplays(DisplayInfo* displays, uint32_t maxCount) const {
  if (!displays || maxCount == 0) {
    return 0;
  }

  struct MonitorEnumData {
    DisplayInfo* displays;
    uint32_t maxCount;
    uint32_t count;
  };

  MonitorEnumData enumData = {displays, maxCount, 0};

  EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
    MonitorEnumData* data = reinterpret_cast<MonitorEnumData*>(lParam);
    if (data->count >= data->maxCount) {
      return FALSE;  // Stop enumeration
    }

    DisplayInfo& info = data->displays[data->count];
    info.index = data->count;

    MONITORINFOEX mi = {sizeof(MONITORINFOEX)};
    if (GetMonitorInfo(hMonitor, &mi)) {
      info.x = mi.rcMonitor.left;
      info.y = mi.rcMonitor.top;
      info.width = mi.rcMonitor.right - mi.rcMonitor.left;
      info.height = mi.rcMonitor.bottom - mi.rcMonitor.top;
      info.primary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;

      // Get DPI
      HDC hdc = GetDC(nullptr);
      if (hdc) {
        UINT dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        UINT dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        info.dpiX = static_cast<float>(dpiX);
        info.dpiY = static_cast<float>(dpiY);
        ReleaseDC(nullptr, hdc);
      }
      info.refreshRate = 60.0f;  // Default
    }

    data->count++;
    return TRUE;  // Continue enumeration
  }, reinterpret_cast<LPARAM>(&enumData));

  return enumData.count;
}

}  // namespace application
}  // namespace te
