/**
 * @file registry_state.cpp
 * @brief Internal state for Registry/Lifecycle. Not part of public API.
 */
#include "te/subsystems/detail/registry_state.hpp"

#include <map>
#include <mutex>

namespace te {
namespace subsystems {
namespace detail {

static bool g_shutdown = false;
static std::map<void const*, SubsystemState> g_subsystemStates;
static std::mutex g_stateMutex;

bool IsShutdown() {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    return g_shutdown;
}

void SetShutdown(bool value) {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    g_shutdown = value;
}

void SetSubsystemState(void const* typeInfo, SubsystemState state) {
    if (!typeInfo)
        return;
    std::lock_guard<std::mutex> lock(g_stateMutex);
    g_subsystemStates[typeInfo] = state;
}

SubsystemState GetSubsystemState(void const* typeInfo) {
    if (!typeInfo)
        return SubsystemState::Uninitialized;
    std::lock_guard<std::mutex> lock(g_stateMutex);
    auto it = g_subsystemStates.find(typeInfo);
    if (it == g_subsystemStates.end())
        return SubsystemState::Uninitialized;
    return it->second;
}

bool IsSubsystemInState(void const* typeInfo, SubsystemState state) {
    return GetSubsystemState(typeInfo) == state;
}

void ClearAllStates() {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    g_subsystemStates.clear();
    g_shutdown = false;
}

}  // namespace detail
}  // namespace subsystems
}  // namespace te
