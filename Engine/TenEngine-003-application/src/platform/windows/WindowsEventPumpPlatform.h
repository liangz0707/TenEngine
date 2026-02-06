/**
 * @file WindowsEventPumpPlatform.h
 * @brief Windows platform event pump implementation (internal).
 */
#ifndef TE_APPLICATION_WINDOWS_EVENT_PUMP_PLATFORM_H
#define TE_APPLICATION_WINDOWS_EVENT_PUMP_PLATFORM_H

#include "te/application/Platform.h"
#include <windows.h>

namespace te {
namespace application {

/**
 * @brief Windows platform event pump implementation.
 */
class WindowsEventPumpPlatform : public IEventPumpPlatform {
 public:
  WindowsEventPumpPlatform();
  ~WindowsEventPumpPlatform() override;

  bool PollEvent(PlatformEvent& event) override;
  Event ConvertToEngineEvent(PlatformEvent const& platformEvent, WindowId windowId) override;
  bool IsQuitEvent(PlatformEvent const& platformEvent) const override;

 private:
  struct WindowsMessage {
    MSG msg;
  };
};

}  // namespace application
}  // namespace te

#endif  // TE_APPLICATION_WINDOWS_EVENT_PUMP_PLATFORM_H
