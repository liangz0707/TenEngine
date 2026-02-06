/**
 * @file registry.cpp
 * @brief Registry implementation (contract: 007-subsystems-public-api.md).
 */
#include "te/subsystems/registry.hpp"
#include "te/subsystems/detail/registry_detail.hpp"
#include "te/subsystems/detail/registry_state.hpp"
#include "te/core/platform.h"
#include "te/core/log.h"

#include <cstdint>
#include <map>
#include <string>
#include <typeinfo>

namespace {
std::uint32_t CurrentPlatformBits() {
    std::uint32_t bits = 0;
#if TE_PLATFORM_WINDOWS
    bits |= 1u;
#endif
#if TE_PLATFORM_LINUX
    bits |= 2u;
#endif
#if TE_PLATFORM_MACOS
    bits |= 4u;
#endif
#if TE_PLATFORM_ANDROID
    bits |= 8u;
#endif
#if TE_PLATFORM_IOS
    bits |= 16u;
#endif
    return bits;
}
}  // namespace

namespace te {
namespace subsystems {

Registry* Registry::s_instance = nullptr;
std::mutex Registry::s_instanceMutex;

Registry& Registry::GetInstance() {
    if (!s_instance) {
        std::lock_guard<std::mutex> lock(s_instanceMutex);
        if (!s_instance) {
            static Registry instance;
            s_instance = &instance;
        }
    }
    return *s_instance;
}

RegisterResult Registry::RegisterInstance(SubsystemDescriptor const& desc, std::unique_ptr<ISubsystem> instance) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_shutdown) {
        return RegisterResult(false, "Registry is shutdown");
    }
    
    if (!instance) {
        return RegisterResult(false, "Instance is null");
    }
    
    if (!desc.typeInfo) {
        return RegisterResult(false, "TypeInfo is null");
    }
    
    if (!desc.name || desc.name[0] == '\0') {
        return RegisterResult(false, "Subsystem name is null or empty");
    }
    
    // Check platform filter
    if (desc.platformFilter != 0 && (desc.platformFilter & CurrentPlatformBits()) == 0) {
        return RegisterResult(false, "Platform filter excludes current platform");
    }
    
    // Check for duplicate typeInfo
    if (m_entries.find(desc.typeInfo) != m_entries.end()) {
        return RegisterResult(false, "Subsystem with same typeInfo already registered");
    }
    
    // Check for duplicate name
    std::string nameStr(desc.name);
    if (m_nameToTypeInfo.find(nameStr) != m_nameToTypeInfo.end()) {
        return RegisterResult(false, "Subsystem with same name already registered");
    }
    
    // Register subsystem
    Entry entry;
    entry.desc = desc;
    entry.instance = std::move(instance);
    
    // Verify instance implements required methods
    if (entry.instance->GetName() == nullptr || std::string(entry.instance->GetName()) != nameStr) {
        return RegisterResult(false, "Subsystem GetName() does not match descriptor name");
    }
    
    m_entries[desc.typeInfo] = std::move(entry);
    m_nameToTypeInfo[nameStr] = desc.typeInfo;
    
    // Set initial state
    detail::SetSubsystemState(desc.typeInfo, detail::SubsystemState::Uninitialized);
    
    std::string logMsg = "Subsystem registered: " + nameStr;
    te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
    
    return RegisterResult(true, nullptr);
}

bool Registry::Register(SubsystemDescriptor const& desc, std::unique_ptr<ISubsystem> instance) {
    return GetInstance().RegisterInstance(desc, std::move(instance)).success;
}

ISubsystem* Registry::GetSubsystemByName(char const* name) {
    if (!name || name[0] == '\0')
        return nullptr;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_shutdown)
        return nullptr;
    
    auto it = m_nameToTypeInfo.find(std::string(name));
    if (it == m_nameToTypeInfo.end())
        return nullptr;
    
    auto entryIt = m_entries.find(it->second);
    if (entryIt == m_entries.end())
        return nullptr;
    
    return entryIt->second.instance.get();
}

std::vector<ISubsystem*> Registry::GetAllSubsystems() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<ISubsystem*> result;
    result.reserve(m_entries.size());
    
    for (auto& [typeInfo, entry] : m_entries) {
        (void)typeInfo;
        result.push_back(entry.instance.get());
    }
    
    return result;
}

SubsystemState Registry::GetSubsystemState(void const* typeInfo) const {
    if (!typeInfo)
        return SubsystemState::Uninitialized;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto state = detail::GetSubsystemState(typeInfo);
    return static_cast<SubsystemState>(state);
}

SubsystemState Registry::GetSubsystemStateByName(char const* name) const {
    if (!name || name[0] == '\0')
        return SubsystemState::Uninitialized;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_nameToTypeInfo.find(std::string(name));
    if (it == m_nameToTypeInfo.end())
        return SubsystemState::Uninitialized;
    
    return GetSubsystemState(it->second);
}

void Registry::UnregisterInstance(void const* typeInfo) {
    if (!typeInfo)
        return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_entries.find(typeInfo);
    if (it == m_entries.end())
        return;
    
    std::string name(it->second.desc.name);
    m_nameToTypeInfo.erase(name);
    m_entries.erase(it);
    
    detail::SetSubsystemState(typeInfo, detail::SubsystemState::Shutdown);
    
    std::string logMsg = "Subsystem unregistered: " + name;
    te::core::Log(te::core::LogLevel::Info, logMsg.c_str());
}

void Registry::Unregister(void const* typeInfo) {
    GetInstance().UnregisterInstance(typeInfo);
}

void Registry::Lock() {
    m_mutex.lock();
}

void Registry::Unlock() {
    m_mutex.unlock();
}

bool Registry::IsShutdown() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_shutdown;
}

namespace detail {
ISubsystem* GetSubsystemByTypeInfo(void const* typeInfo) {
    auto& reg = Registry::GetInstance();
    std::lock_guard<std::mutex> lock(reg.m_mutex);
    
    if (reg.m_shutdown)
        return nullptr;
    
    auto it = reg.m_entries.find(typeInfo);
    if (it == reg.m_entries.end())
        return nullptr;
    
    return it->second.instance.get();
}

std::vector<SubsystemEntry> GetEntriesForLifecycle() {
    auto& reg = Registry::GetInstance();
    std::lock_guard<std::mutex> lock(reg.m_mutex);
    
    std::vector<SubsystemEntry> out;
    out.reserve(reg.m_entries.size());
    
    for (auto& [typeInfo, entry] : reg.m_entries) {
        (void)typeInfo;
        out.emplace_back(entry.desc, entry.instance.get());
    }
    
    return out;
}

void SetRegistryShutdown(bool value) {
    auto& reg = Registry::GetInstance();
    std::lock_guard<std::mutex> lock(reg.m_mutex);
    reg.m_shutdown = value;
}
}  // namespace detail

}  // namespace subsystems
}  // namespace te
