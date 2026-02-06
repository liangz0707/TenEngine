/**
 * @file registry_state.hpp
 * @brief Internal state for Registry/Lifecycle coordination. Not part of public API.
 */
#ifndef TE_SUBSYSTEMS_DETAIL_REGISTRY_STATE_HPP
#define TE_SUBSYSTEMS_DETAIL_REGISTRY_STATE_HPP

#include <cstddef>

namespace te {
namespace subsystems {
namespace detail {

/**
 * Subsystem lifecycle state.
 */
enum class SubsystemState {
    Uninitialized,  // Registered but not initialized
    Initialized,    // Initialize() called successfully
    Started,        // Start() called successfully
    Stopped,        // Stop() called
    Shutdown        // Shutdown() called
};

/**
 * Check if registry is in shutdown state (all subsystems shutdown).
 */
bool IsShutdown();
void SetShutdown(bool value);

/**
 * Set state for a specific subsystem by typeInfo.
 */
void SetSubsystemState(void const* typeInfo, SubsystemState state);

/**
 * Get state for a specific subsystem by typeInfo.
 * Returns Uninitialized if subsystem not found.
 */
SubsystemState GetSubsystemState(void const* typeInfo);

/**
 * Check if subsystem is in a specific state.
 */
bool IsSubsystemInState(void const* typeInfo, SubsystemState state);

/**
 * Clear all subsystem states (for testing).
 */
void ClearAllStates();

}  // namespace detail
}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_DETAIL_REGISTRY_STATE_HPP
