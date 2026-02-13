/**
 * @file WindowsWindowPlatform.h
 * @brief Windows platform window implementation (internal).
 */
#ifndef TE_APPLICATION_WINDOWS_WINDOW_PLATFORM_H
#define TE_APPLICATION_WINDOWS_WINDOW_PLATFORM_H

#include "te/application/Platform.h"
#include <windows.h>

namespace te {
namespace application {

/**
 * @brief Windows platform window implementation.
 */
class WindowsWindowPlatform : public IWindowPlatform {
 public:
  WindowsWindowPlatform();
  ~WindowsWindowPlatform() override;

  void* CreateNativeWindow(WindowDesc const& desc) override;
  void DestroyNativeWindow(void* handle) override;
  void SetWndProcHandler(void* handler) override;
  void SetWindowTitle(void* handle, char const* title) override;
  void SetWindowSize(void* handle, uint32_t width, uint32_t height) override;
  void SetWindowPosition(void* handle, int32_t x, int32_t y) override;
  void SetFullscreen(void* handle, bool fullscreen) override;
  DisplayInfo GetDisplayInfo(uint32_t displayIndex) const override;
  uint32_t EnumerateDisplays(DisplayInfo* displays, uint32_t maxCount) const override;

 private:
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  bool RegisterWindowClass();
  void* GetWindowUserData(HWND hwnd);
  void SetWindowUserData(HWND hwnd, void* data);

  static bool s_windowClassRegistered;
  static constexpr char const* s_windowClassName = "TenEngineWindow";
  static void* s_wndProcHandler;
};

}  // namespace application
}  // namespace te

#endif  // TE_APPLICATION_WINDOWS_WINDOW_PLATFORM_H
