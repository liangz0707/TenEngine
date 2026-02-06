/**
 * @file registry_state.cpp
 * @brief Internal state for Registry/Lifecycle. Not part of public API.
 */
#include "te/subsystems/detail/registry_state.hpp"

namespace te {
namespace subsystems {
namespace detail {

static bool g_shutdown = false;

bool IsShutdown() { return g_shutdown; }
void SetShutdown(bool value) { g_shutdown = value; }

}  // namespace detail
}  // namespace subsystems
}  // namespace te
