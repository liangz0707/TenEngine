/**
 * @file ApplicationImpl.h
 * @brief Application implementation internal structures (internal).
 */
#ifndef TE_APPLICATION_APPLICATION_IMPL_H
#define TE_APPLICATION_APPLICATION_IMPL_H

#include "te/application/Application.h"
#include "te/application/Window.h"
#include "te/application/Event.h"
#include "te/application/MainLoop.h"
#include "te/application/Platform.h"
#include "te/core/containers.h"
#include "te/core/platform.h"
#include <mutex>

namespace te {
namespace application {

/**
 * @brief Window data structure.
 */
struct WindowData {
  void* nativeHandle = nullptr;
  WindowCallback callback = nullptr;
  WindowDesc desc = {};
};

/**
 * @brief Tick callback data structure.
 */
struct TickCallbackData {
  TickCallback callback = nullptr;
  int32_t priority = 0;
};

}  // namespace application
}  // namespace te

#endif  // TE_APPLICATION_APPLICATION_IMPL_H
