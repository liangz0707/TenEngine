/**
 * @file discovery.hpp
 * @brief Discovery: ScanPlugins, RegisterFromPlugin (contract: 007-subsystems-public-api.md).
 * Only contract-declared types and API.
 */
#ifndef TE_SUBSYSTEMS_DISCOVERY_HPP
#define TE_SUBSYSTEMS_DISCOVERY_HPP

#include "te/subsystems/registry.hpp"

#include <vector>

namespace te {
namespace subsystems {

/**
 * Discovery result structure for error reporting.
 */
struct DiscoveryResult {
    bool success;
    std::vector<char const*> discoveredSubsystems;
    std::vector<char const*> failedPlugins;
    char const* errorMessage;
    
    DiscoveryResult() : success(true), errorMessage(nullptr) {}
    DiscoveryResult(bool s, char const* msg) : success(s), errorMessage(msg) {}
};

/**
 * Discovery: scan plugins and register subsystems; works with Core ModuleLoad.
 * moduleHandle: Core module handle (e.g. LoadLibrary return).
 */
class Discovery {
public:
    /**
     * Scan plugins from multiple paths and register subsystems.
     * pluginPaths: array of plugin directory paths or plugin file paths.
     * pathCount: number of paths in the array.
     * Returns DiscoveryResult with discovered subsystems and failed plugins.
     */
    static DiscoveryResult ScanPlugins(Registry& reg, 
                                      char const* const* pluginPaths, 
                                      size_t pathCount);
    
    /**
     * Scan plugins (backward compatibility - no-op).
     */
    static bool ScanPlugins(Registry& reg);
    
    /**
     * Register subsystems from a plugin module.
     * moduleHandle: Core module handle (e.g. LoadLibrary return).
     * Returns true on success, false on failure.
     */
    static bool RegisterFromPlugin(Registry& reg, void* moduleHandle);
    
    /**
     * Unregister subsystems from a plugin module.
     * This should be called before unloading the plugin module.
     * moduleHandle: Core module handle.
     * Returns true on success, false on failure.
     */
    static bool UnregisterFromPlugin(Registry& reg, void* moduleHandle);
};

}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_DISCOVERY_HPP
