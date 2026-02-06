/**
 * @file lifecycle.hpp
 * @brief Lifecycle: InitializeAll, StartAll, StopAll, ShutdownAll (contract: 007-subsystems-public-api.md).
 * Only contract-declared types and API.
 */
#ifndef TE_SUBSYSTEMS_LIFECYCLE_HPP
#define TE_SUBSYSTEMS_LIFECYCLE_HPP

#include "te/subsystems/registry.hpp"

#include <vector>

namespace te {
namespace subsystems {

/**
 * Lifecycle result structure for error reporting.
 */
struct LifecycleResult {
    bool success;
    std::vector<char const*> failedSubsystems;
    char const* errorMessage;
    
    LifecycleResult() : success(true), errorMessage(nullptr) {}
    LifecycleResult(bool s, char const* msg) : success(s), errorMessage(msg) {}
};

/**
 * Lifecycle: dependency topology + same-layer Priority.
 * InitializeAll returns false on cycle; main-loop single-threaded only.
 */
class Lifecycle {
public:
    /**
     * Initialize all subsystems in dependency order.
     * Returns LifecycleResult with success status and list of failed subsystems.
     */
    static LifecycleResult InitializeAll(Registry& reg);
    
    /**
     * Start all subsystems in priority order.
     * Returns LifecycleResult with success status and list of failed subsystems.
     */
    static LifecycleResult StartAll(Registry& reg);
    
    /**
     * Stop all subsystems in reverse priority order.
     * Returns LifecycleResult with success status and list of failed subsystems.
     */
    static LifecycleResult StopAll(Registry& reg);
    
    /**
     * Shutdown all subsystems in reverse dependency order.
     */
    static void ShutdownAll(Registry& reg);
    
    /**
     * Initialize a single subsystem by name.
     * Returns LifecycleResult with success status.
     */
    static LifecycleResult InitializeSubsystem(Registry& reg, char const* name);
    
    /**
     * Start a single subsystem by name.
     * Returns LifecycleResult with success status.
     */
    static LifecycleResult StartSubsystem(Registry& reg, char const* name);
    
    /**
     * Stop a single subsystem by name.
     * Returns LifecycleResult with success status.
     */
    static LifecycleResult StopSubsystem(Registry& reg, char const* name);
    
    /**
     * Shutdown a single subsystem by name.
     */
    static void ShutdownSubsystem(Registry& reg, char const* name);
    
    /**
     * Check if all dependencies of a subsystem are satisfied.
     * Returns true if all dependencies are registered and initialized.
     */
    static bool CheckDependencies(Registry& reg, char const* subsystemName);
    
    // Backward compatibility methods
    static bool InitializeAll(Registry const& reg) {
        return InitializeAll(const_cast<Registry&>(reg)).success;
    }
    static void StartAll(Registry const& reg) {
        StartAll(const_cast<Registry&>(reg));
    }
    static void StopAll(Registry const& reg) {
        StopAll(const_cast<Registry&>(reg));
    }
};

}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_LIFECYCLE_HPP
