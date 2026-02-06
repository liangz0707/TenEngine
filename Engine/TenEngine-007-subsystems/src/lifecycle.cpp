/**
 * @file lifecycle.cpp
 * @brief Lifecycle implementation (contract: 007-subsystems-public-api.md).
 */
#include "te/subsystems/lifecycle.hpp"
#include "te/subsystems/detail/registry_detail.hpp"
#include "te/subsystems/detail/registry_state.hpp"
#include "te/core/log.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>
#include <sstream>

namespace te {
namespace subsystems {

namespace {

std::string TypeInfoToName(void const* typeInfo) {
    if (!typeInfo) return {};
    auto const* ti = static_cast<std::type_info const*>(typeInfo);
    return ti->name();
}

std::string GetSubsystemName(SubsystemDescriptor const& desc) {
    if (desc.name && desc.name[0] != '\0')
        return std::string(desc.name);
    return TypeInfoToName(desc.typeInfo);
}

/** Kahn's algorithm; returns sorted indices or empty on cycle. */
bool TopoSort(std::vector<detail::SubsystemEntry> const& entries,
              std::map<std::string, size_t> const& nameToIdx,
              std::vector<size_t>& out,
              std::vector<std::string>& cycleInfo) {
    size_t const n = entries.size();
    std::vector<std::vector<size_t>> adj(n);  // adj[j] = {i : i depends on j}
    std::vector<int> inDeg(n, 0);

    for (size_t i = 0; i < n; ++i) {
        auto const& desc = entries[i].first;
        for (size_t k = 0; k < desc.dependencyCount && desc.dependencies && desc.dependencies[k]; ++k) {
            std::string depName(desc.dependencies[k]);
            auto it = nameToIdx.find(depName);
            if (it != nameToIdx.end() && it->second != i) {
                size_t j = it->second;
                adj[j].push_back(i);
                ++inDeg[i];
            } else if (it == nameToIdx.end()) {
                // Missing dependency - log warning but don't fail
                std::string subsystemName = GetSubsystemName(desc);
                std::string msg = "Subsystem '" + subsystemName + "' depends on missing subsystem '" + depName + "'";
                te::core::Log(te::core::LogLevel::Warn, msg.c_str());
            }
        }
    }

    std::vector<size_t> q;
    for (size_t i = 0; i < n; ++i)
        if (inDeg[i] == 0)
            q.push_back(i);

    out.clear();
    while (!q.empty()) {
        std::sort(q.begin(), q.end(), [&entries](size_t a, size_t b) {
            return entries[a].first.priority < entries[b].first.priority;  // asc for init
        });
        size_t u = q.back();
        q.pop_back();
        out.push_back(u);
        for (size_t i : adj[u]) {
            --inDeg[i];
            if (inDeg[i] == 0)
                q.push_back(i);
        }
    }

    if (out.size() != n) {
        // Build cycle information
        cycleInfo.clear();
        for (size_t i = 0; i < n; ++i) {
            if (inDeg[i] > 0) {
                cycleInfo.push_back(GetSubsystemName(entries[i].first));
            }
        }
        return false;  // cycle detected
    }
    return true;
}

}  // namespace

LifecycleResult Lifecycle::InitializeAll(Registry& reg) {
    auto ents = detail::GetEntriesForLifecycle();
    if (ents.empty())
        return LifecycleResult(true, nullptr);

    // Build name to index mapping (use descriptor name if available)
    std::map<std::string, size_t> nameToIdx;
    for (size_t i = 0; i < ents.size(); ++i) {
        std::string name = GetSubsystemName(ents[i].first);
        nameToIdx[name] = i;
        // Also map by typeInfo name for backward compatibility
        std::string typeName = TypeInfoToName(ents[i].first.typeInfo);
        if (name != typeName)
            nameToIdx[typeName] = i;
    }

    std::vector<size_t> order;
    std::vector<std::string> cycleInfo;
    if (!TopoSort(ents, nameToIdx, order, cycleInfo)) {
        std::stringstream ss;
        ss << "Circular dependency detected involving: ";
        for (size_t i = 0; i < cycleInfo.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << cycleInfo[i];
        }
        std::string errorMsg = ss.str();
        te::core::Log(te::core::LogLevel::Error, errorMsg.c_str());
        return LifecycleResult(false, errorMsg.c_str());
    }

    std::vector<char const*> failed;
    for (size_t idx : order) {
        if (!ents[idx].second)
            continue;
        
        std::string name = GetSubsystemName(ents[idx].first);
        std::string logMsg = "Initializing subsystem: " + name;
        te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
        
        if (!ents[idx].second->Initialize()) {
            failed.push_back(ents[idx].first.name ? ents[idx].first.name : "Unknown");
            detail::SetSubsystemState(ents[idx].first.typeInfo, detail::SubsystemState::Uninitialized);
            std::string errorMsg = "Failed to initialize subsystem: " + name;
            te::core::Log(te::core::LogLevel::Error, errorMsg.c_str());
        } else {
            detail::SetSubsystemState(ents[idx].first.typeInfo, detail::SubsystemState::Initialized);
        }
    }

    if (!failed.empty()) {
        LifecycleResult result(false, "One or more subsystems failed to initialize");
        result.failedSubsystems = failed;
        return result;
    }

    return LifecycleResult(true, nullptr);
}

LifecycleResult Lifecycle::StartAll(Registry& reg) {
    auto ents = detail::GetEntriesForLifecycle();
    std::vector<size_t> indices(ents.size());
    for (size_t i = 0; i < ents.size(); ++i)
        indices[i] = i;
    std::sort(indices.begin(), indices.end(), [&ents](size_t a, size_t b) {
        return ents[a].first.priority < ents[b].first.priority;
    });
    
    std::vector<char const*> failed;
    for (size_t idx : indices) {
        if (!ents[idx].second)
            continue;
        
        // Only start if initialized
        auto state = detail::GetSubsystemState(ents[idx].first.typeInfo);
        if (state != detail::SubsystemState::Initialized && 
            state != detail::SubsystemState::Stopped) {
            continue;
        }
        
        std::string name = GetSubsystemName(ents[idx].first);
        std::string logMsg = "Starting subsystem: " + name;
        te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
        
        try {
            ents[idx].second->Start();
            detail::SetSubsystemState(ents[idx].first.typeInfo, detail::SubsystemState::Started);
        } catch (...) {
            failed.push_back(ents[idx].first.name ? ents[idx].first.name : "Unknown");
            std::string errorMsg = "Failed to start subsystem: " + name;
            te::core::Log(te::core::LogLevel::Error, errorMsg.c_str());
        }
    }
    
    if (!failed.empty()) {
        LifecycleResult result(false, "One or more subsystems failed to start");
        result.failedSubsystems = failed;
        return result;
    }
    
    return LifecycleResult(true, nullptr);
}

LifecycleResult Lifecycle::StopAll(Registry& reg) {
    auto ents = detail::GetEntriesForLifecycle();
    std::vector<size_t> indices(ents.size());
    for (size_t i = 0; i < ents.size(); ++i)
        indices[i] = i;
    // Sort in reverse priority order (higher priority first)
    std::sort(indices.begin(), indices.end(), [&ents](size_t a, size_t b) {
        return ents[a].first.priority > ents[b].first.priority;
    });
    
    std::vector<char const*> failed;
    for (size_t idx : indices) {
        if (!ents[idx].second)
            continue;
        
        // Only stop if started
        auto state = detail::GetSubsystemState(ents[idx].first.typeInfo);
        if (state != detail::SubsystemState::Started) {
            continue;
        }
        
        std::string name = GetSubsystemName(ents[idx].first);
        std::string logMsg = "Stopping subsystem: " + name;
        te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
        
        try {
            ents[idx].second->Stop();
            detail::SetSubsystemState(ents[idx].first.typeInfo, detail::SubsystemState::Stopped);
        } catch (...) {
            failed.push_back(ents[idx].first.name ? ents[idx].first.name : "Unknown");
            std::string errorMsg = "Failed to stop subsystem: " + name;
            te::core::Log(te::core::LogLevel::Error, errorMsg.c_str());
        }
    }
    
    if (!failed.empty()) {
        LifecycleResult result(false, "One or more subsystems failed to stop");
        result.failedSubsystems = failed;
        return result;
    }
    
    return LifecycleResult(true, nullptr);
}

void Lifecycle::ShutdownAll(Registry& reg) {
    auto ents = detail::GetEntriesForLifecycle();
    
    // Build reverse dependency order
    std::map<std::string, size_t> nameToIdx;
    for (size_t i = 0; i < ents.size(); ++i) {
        std::string name = GetSubsystemName(ents[i].first);
        nameToIdx[name] = i;
    }
    
    std::vector<size_t> order;
    std::vector<std::string> cycleInfo;
    TopoSort(ents, nameToIdx, order, cycleInfo);  // Get dependency order
    
    // Reverse for shutdown order
    std::reverse(order.begin(), order.end());
    
    for (size_t idx : order) {
        if (!ents[idx].second)
            continue;
        
        std::string name = GetSubsystemName(ents[idx].first);
        te::core::Log(te::core::LogLevel::Info, ("Shutting down subsystem: " + name).c_str());
        
        try {
            ents[idx].second->Shutdown();
            detail::SetSubsystemState(ents[idx].first.typeInfo, detail::SubsystemState::Shutdown);
        } catch (...) {
            std::string errorMsg = "Exception during shutdown of subsystem: " + name;
            te::core::Log(te::core::LogLevel::Error, errorMsg.c_str());
        }
    }
    
    detail::SetShutdown(true);
    detail::SetRegistryShutdown(true);
}

LifecycleResult Lifecycle::InitializeSubsystem(Registry& reg, char const* name) {
    if (!name || name[0] == '\0') {
        return LifecycleResult(false, "Subsystem name is null or empty");
    }
    
    ISubsystem* subsystem = reg.GetSubsystemByName(name);
    if (!subsystem) {
        std::string errorMsg = "Subsystem not found: " + std::string(name);
        return LifecycleResult(false, errorMsg.c_str());
    }
    
    auto state = reg.GetSubsystemStateByName(name);
    if (state != SubsystemState::Uninitialized) {
        std::string errorMsg = "Subsystem already initialized: " + std::string(name);
        return LifecycleResult(false, errorMsg.c_str());
    }
    
    // Check dependencies
    if (!CheckDependencies(reg, name)) {
        std::string errorMsg = "Dependencies not satisfied for subsystem: " + std::string(name);
        return LifecycleResult(false, errorMsg.c_str());
    }
    
    SubsystemDescriptor const& desc = subsystem->GetDescriptor();
    std::string subsystemName = GetSubsystemName(desc);
    std::string logMsg = "Initializing subsystem: " + subsystemName;
    te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
    
    if (!subsystem->Initialize()) {
        detail::SetSubsystemState(desc.typeInfo, detail::SubsystemState::Uninitialized);
        std::string errorMsg = "Failed to initialize subsystem: " + subsystemName;
        te::core::Log(te::core::LogLevel::Error, errorMsg.c_str());
        return LifecycleResult(false, errorMsg.c_str());
    }
    
    detail::SetSubsystemState(desc.typeInfo, detail::SubsystemState::Initialized);
    return LifecycleResult(true, nullptr);
}

LifecycleResult Lifecycle::StartSubsystem(Registry& reg, char const* name) {
    if (!name || name[0] == '\0') {
        return LifecycleResult(false, "Subsystem name is null or empty");
    }
    
    ISubsystem* subsystem = reg.GetSubsystemByName(name);
    if (!subsystem) {
        std::string errorMsg = "Subsystem not found: " + std::string(name);
        return LifecycleResult(false, errorMsg.c_str());
    }
    
    auto state = reg.GetSubsystemStateByName(name);
    if (state != SubsystemState::Initialized && state != SubsystemState::Stopped) {
        std::string errorMsg = "Subsystem not in valid state for start: " + std::string(name);
        return LifecycleResult(false, errorMsg.c_str());
    }
    
    SubsystemDescriptor const& desc = subsystem->GetDescriptor();
    std::string subsystemName = GetSubsystemName(desc);
    std::string logMsg = "Starting subsystem: " + subsystemName;
    te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
    
    try {
        subsystem->Start();
        detail::SetSubsystemState(desc.typeInfo, detail::SubsystemState::Started);
        return LifecycleResult(true, nullptr);
    } catch (...) {
        std::string errorMsg = "Exception during start of subsystem: " + subsystemName;
        te::core::Log(te::core::LogLevel::Error, errorMsg.c_str());
        return LifecycleResult(false, errorMsg.c_str());
    }
}

LifecycleResult Lifecycle::StopSubsystem(Registry& reg, char const* name) {
    if (!name || name[0] == '\0') {
        return LifecycleResult(false, "Subsystem name is null or empty");
    }
    
    ISubsystem* subsystem = reg.GetSubsystemByName(name);
    if (!subsystem) {
        std::string errorMsg = "Subsystem not found: " + std::string(name);
        return LifecycleResult(false, errorMsg.c_str());
    }
    
    auto state = reg.GetSubsystemStateByName(name);
    if (state != SubsystemState::Started) {
        std::string errorMsg = "Subsystem not started: " + std::string(name);
        return LifecycleResult(false, errorMsg.c_str());
    }
    
    SubsystemDescriptor const& desc = subsystem->GetDescriptor();
    std::string subsystemName = GetSubsystemName(desc);
    std::string logMsg = "Stopping subsystem: " + subsystemName;
    te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
    
    try {
        subsystem->Stop();
        detail::SetSubsystemState(desc.typeInfo, detail::SubsystemState::Stopped);
        return LifecycleResult(true, nullptr);
    } catch (...) {
        std::string errorMsg = "Exception during stop of subsystem: " + subsystemName;
        te::core::Log(te::core::LogLevel::Error, errorMsg.c_str());
        return LifecycleResult(false, errorMsg.c_str());
    }
}

void Lifecycle::ShutdownSubsystem(Registry& reg, char const* name) {
    if (!name || name[0] == '\0')
        return;
    
    ISubsystem* subsystem = reg.GetSubsystemByName(name);
    if (!subsystem)
        return;
    
    SubsystemDescriptor const& desc = subsystem->GetDescriptor();
    std::string subsystemName = GetSubsystemName(desc);
    std::string logMsg = "Shutting down subsystem: " + subsystemName;
    te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
    
    try {
        subsystem->Shutdown();
        detail::SetSubsystemState(desc.typeInfo, detail::SubsystemState::Shutdown);
    } catch (...) {
        std::string errorMsg = "Exception during shutdown of subsystem: " + subsystemName;
        te::core::Log(te::core::LogLevel::Error, errorMsg.c_str());
    }
}

bool Lifecycle::CheckDependencies(Registry& reg, char const* subsystemName) {
    if (!subsystemName || subsystemName[0] == '\0')
        return false;
    
    ISubsystem* subsystem = reg.GetSubsystemByName(subsystemName);
    if (!subsystem)
        return false;
    
    SubsystemDescriptor const& desc = subsystem->GetDescriptor();
    
    // Check if all dependencies are registered and initialized
    for (size_t i = 0; i < desc.dependencyCount && desc.dependencies && desc.dependencies[i]; ++i) {
        std::string depName(desc.dependencies[i]);
        ISubsystem* depSubsystem = reg.GetSubsystemByName(depName.c_str());
        
        if (!depSubsystem) {
            std::string errorMsg = "Dependency not found: " + depName + " (required by " + std::string(subsystemName) + ")";
            te::core::Log(te::core::LogLevel::Warn, errorMsg.c_str());
            return false;
        }
        
        auto depState = reg.GetSubsystemStateByName(depName.c_str());
        if (depState != SubsystemState::Initialized && depState != SubsystemState::Started) {
            std::string errorMsg = "Dependency not initialized: " + depName + " (required by " + std::string(subsystemName) + ")";
            te::core::Log(te::core::LogLevel::Warn, errorMsg.c_str());
            return false;
        }
    }
    
    return true;
}

}  // namespace subsystems
}  // namespace te
