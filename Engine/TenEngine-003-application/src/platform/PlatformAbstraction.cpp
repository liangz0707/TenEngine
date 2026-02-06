/**
 * @file PlatformAbstraction.cpp
 * @brief Platform abstraction factory implementation.
 */
#include "te/application/Platform.h"
#include "te/core/platform.h"

#if TE_PLATFORM_WINDOWS
#include "windows/WindowsWindowPlatform.h"
#include "windows/WindowsEventPumpPlatform.h"
#endif

namespace te {
namespace application {

IWindowPlatform* CreateWindowPlatform() {
#if TE_PLATFORM_WINDOWS
  return new WindowsWindowPlatform();
#elif TE_PLATFORM_LINUX
  // TODO: return new X11WindowPlatform();
  return nullptr;
#elif TE_PLATFORM_MACOS
  // TODO: return new CocoaWindowPlatform();
  return nullptr;
#elif TE_PLATFORM_ANDROID
  // TODO: return new AndroidWindowPlatform();
  return nullptr;
#elif TE_PLATFORM_IOS
  // TODO: return new IOSWindowPlatform();
  return nullptr;
#else
  return nullptr;
#endif
}

IEventPumpPlatform* CreateEventPumpPlatform() {
#if TE_PLATFORM_WINDOWS
  return new WindowsEventPumpPlatform();
#elif TE_PLATFORM_LINUX
  // TODO: return new X11EventPumpPlatform();
  return nullptr;
#elif TE_PLATFORM_MACOS
  // TODO: return new CocoaEventPumpPlatform();
  return nullptr;
#elif TE_PLATFORM_ANDROID
  // TODO: return new AndroidEventPumpPlatform();
  return nullptr;
#elif TE_PLATFORM_IOS
  // TODO: return new IOSEventPumpPlatform();
  return nullptr;
#else
  return nullptr;
#endif
}

}  // namespace application
}  // namespace te
