/**
 * @file discovery.cpp
 * @brief Discovery implementation (contract: 007-subsystems-public-api.md).
 * Uses 001-Core module_load (LoadLibrary, GetSymbol) per contract.
 */
#include "te/subsystems/discovery.hpp"
#include "te/core/module_load.h"
#include "te/core/log.h"
#include "te/core/platform.h"

#include <vector>
#include <string>
#include <map>
#include <algorithm>

namespace te {
namespace subsystems {

namespace {
/** Plugin export convention: void fn(Registry*). */
using RegisterFn = void (*)(Registry*);
using UnregisterFn = void (*)(Registry*);
constexpr char const* kRegisterSymbol = "te_subsystems_register";
constexpr char const* kUnregisterSymbol = "te_subsystems_unregister";

// Track which subsystems belong to which plugin
static std::map<void*, std::vector<void const*>> g_pluginSubsystems;
}  // namespace

DiscoveryResult Discovery::ScanPlugins(Registry& reg, 
                                      char const* const* pluginPaths, 
                                      size_t pathCount) {
    DiscoveryResult result;
    
    if (!pluginPaths || pathCount == 0) {
        result.success = false;
        result.errorMessage = "Plugin paths array is null or empty";
        return result;
    }
    
    std::vector<std::string> discovered;
    std::vector<std::string> failed;
    
    for (size_t i = 0; i < pathCount; ++i) {
        if (!pluginPaths[i])
            continue;
        
        std::string pluginPath(pluginPaths[i]);
        std::string logMsg = "Scanning plugin: " + pluginPath;
        te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
        
        // Load plugin module
        te::core::ModuleHandle handle = te::core::LoadLibrary(pluginPath.c_str());
        if (!handle) {
            failed.push_back(pluginPath);
            std::string errorMsg = "Failed to load plugin: " + pluginPath;
            te::core::Log(te::core::LogLevel::Error, errorMsg.c_str());
            continue;
        }
        
        // Try to register subsystems from plugin
        if (RegisterFromPlugin(reg, handle)) {
            discovered.push_back(pluginPath);
            std::string logMsg = "Successfully registered subsystems from plugin: " + pluginPath;
            te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
        } else {
            failed.push_back(pluginPath);
            std::string logMsg = "No subsystems found in plugin: " + pluginPath;
            te::core::Log(te::core::LogLevel::Warn, logMsg.c_str());
            // Unload if no subsystems registered
            te::core::UnloadLibrary(handle);
        }
    }
    
    // Convert to C strings for result
    // Use static vectors to keep strings alive for the lifetime of the result
    static std::vector<std::string> s_discoveredStrings;
    static std::vector<std::string> s_failedStrings;
    s_discoveredStrings = discovered;
    s_failedStrings = failed;
    
    result.discoveredSubsystems.clear();
    result.failedPlugins.clear();
    
    for (auto const& s : s_discoveredStrings) {
        result.discoveredSubsystems.push_back(s.c_str());
    }
    for (auto const& s : s_failedStrings) {
        result.failedPlugins.push_back(s.c_str());
    }
    
    result.success = failed.empty() || !discovered.empty();
    if (!result.success && !failed.empty()) {
        result.errorMessage = "All plugins failed to load";
    }
    
    return result;
}

bool Discovery::ScanPlugins(Registry& reg) {
    /* Contract: ScanPlugins scans loaded plugins. Core does not expose "list loaded modules".
     * Minimal impl: no-op; return true. Caller may use RegisterFromPlugin per loaded module. */
    return true;
}

bool Discovery::RegisterFromPlugin(Registry& reg, void* moduleHandle) {
    if (!moduleHandle)
        return false;
    
    auto handle = static_cast<te::core::ModuleHandle>(moduleHandle);
    
    // Look for registration function
    auto* registerFn = reinterpret_cast<RegisterFn>(
        te::core::GetSymbol(handle, kRegisterSymbol));
    
    if (!registerFn) {
        // Plugin doesn't export subsystem registration function - not an error
        return false;
    }
    
    // Get current subsystem count before registration
    auto subsystemsBefore = reg.GetAllSubsystems();
    size_t countBefore = subsystemsBefore.size();
    
    // Call registration function
    try {
        registerFn(&reg);
    } catch (...) {
        te::core::Log(te::core::LogLevel::Error, 
                      "Exception during plugin subsystem registration");
        return false;
    }
    
    // Track which subsystems were registered by this plugin
    auto subsystemsAfter = reg.GetAllSubsystems();
    std::vector<void const*> pluginSubsystems;
    
    for (auto* subsystem : subsystemsAfter) {
        bool found = false;
        for (auto* before : subsystemsBefore) {
            if (subsystem == before) {
                found = true;
                break;
            }
        }
        if (!found) {
            pluginSubsystems.push_back(subsystem->GetDescriptor().typeInfo);
        }
    }
    
    if (!pluginSubsystems.empty()) {
        g_pluginSubsystems[moduleHandle] = pluginSubsystems;
        std::string logMsg = "Registered " + std::to_string(pluginSubsystems.size()) + " subsystems from plugin";
        te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
        return true;
    }
    
    return false;
}

bool Discovery::UnregisterFromPlugin(Registry& reg, void* moduleHandle) {
    if (!moduleHandle)
        return false;
    
    auto it = g_pluginSubsystems.find(moduleHandle);
    if (it == g_pluginSubsystems.end()) {
        // No subsystems registered from this plugin
        return true;
    }
    
    // Look for unregistration function (optional)
    auto handle = static_cast<te::core::ModuleHandle>(moduleHandle);
    auto* unregisterFn = reinterpret_cast<UnregisterFn>(
        te::core::GetSymbol(handle, kUnregisterSymbol));
    
    if (unregisterFn) {
        try {
            unregisterFn(&reg);
        } catch (...) {
            te::core::Log(te::core::LogLevel::Error, "Exception during plugin subsystem unregistration");
        }
    }
    
    // Unregister all subsystems from this plugin
    for (void const* typeInfo : it->second) {
        Registry::Unregister(typeInfo);
    }
    
    g_pluginSubsystems.erase(it);
    
    te::core::Log(te::core::LogLevel::Info, "Unregistered subsystems from plugin");
    
    return true;
}

}  // namespace subsystems
}  // namespace te
