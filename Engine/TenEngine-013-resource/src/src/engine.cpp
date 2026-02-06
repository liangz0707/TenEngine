/**
 * @file engine.cpp
 * @brief Implementation of Init/Shutdown per contract (001-core-ABI.md).
 */

#include "te/core/engine.h"

namespace te {
namespace core {

static bool g_initialized = false;

bool Init(InitParams const* params) {
  (void)params;
  if (g_initialized) return true;
  g_initialized = true;
  return true;
}

void Shutdown() {
  g_initialized = false;
}

}  // namespace core
}  // namespace te
