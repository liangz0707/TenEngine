/**
 * @file registry.hpp
 * @brief Registry: Register, GetSubsystem<T>, Unregister (contract: 007-subsystems-public-api.md).
 * Only contract-declared types and API.
 */
#ifndef TE_SUBSYSTEMS_REGISTRY_HPP
#define TE_SUBSYSTEMS_REGISTRY_HPP

#include "te/subsystems/descriptor.hpp"
#include "te/subsystems/subsystem.hpp"
#include "te/subsystems/detail/registry_detail.hpp"

#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <map>

namespace te {
namespace subsystems {

/**
 * Subsystem state enumeration.
 */
enum class SubsystemState {
    Uninitialized,  // Registered but not initialized
    Initialized,    // Initialize() called successfully
    Started,        // Start() called successfully
    Stopped,        // Stop() called
    Shutdown        // Shutdown() called
};

/**
 * Registry result structure for error reporting.
 */
struct RegisterResult {
    bool success;
    char const* errorMessage;
    
    RegisterResult() : success(true), errorMessage(nullptr) {}
    RegisterResult(bool s, char const* msg) : success(s), errorMessage(msg) {}
};

/**
 * Subsystem registry: per-type singleton; duplicate Register returns false.
 * GetSubsystem returns nullptr when unregistered or after ShutdownAll.
 * Convention: desc.typeInfo = &typeid(ConcreteSubsystem) when registering.
 * Thread-safe: all operations are protected by mutex.
 */
class Registry {
public:
    /**
     * Get singleton instance.
     */
    static Registry& GetInstance();
    
    /**
     * Register subsystem. Returns RegisterResult with success status and error message.
     * Instance ownership is transferred to registry.
     */
    RegisterResult RegisterInstance(SubsystemDescriptor const& desc, std::unique_ptr<ISubsystem> instance);
    
    /**
     * Register subsystem (static convenience method).
     * Returns false on failure (for backward compatibility).
     */
    static bool Register(SubsystemDescriptor const& desc, std::unique_ptr<ISubsystem> instance);
    
    /**
     * Get subsystem by type (instance method); returns nullptr if unregistered or after ShutdownAll.
     */
    template <typename T>
    T* GetSubsystemImpl();
    
    /**
     * Get subsystem by type (static convenience method).
     */
    template <typename T>
    static T* GetSubsystem();
    
    /**
     * Get subsystem by name; returns nullptr if not found or after ShutdownAll.
     */
    ISubsystem* GetSubsystemByName(char const* name);
    
    /**
     * Get all registered subsystems.
     */
    std::vector<ISubsystem*> GetAllSubsystems();
    
    /**
     * Get subsystem state by typeInfo.
     */
    SubsystemState GetSubsystemState(void const* typeInfo) const;
    
    /**
     * Get subsystem state by name.
     */
    SubsystemState GetSubsystemStateByName(char const* name) const;
    
    /**
     * Unregister by typeInfo (instance method).
     */
    void UnregisterInstance(void const* typeInfo);
    
    /**
     * Unregister by typeInfo (static convenience method).
     */
    static void Unregister(void const* typeInfo);
    
    /**
     * Lock registry mutex (for external synchronization if needed).
     */
    void Lock();
    
    /**
     * Unlock registry mutex.
     */
    void Unlock();
    
    /**
     * Check if registry is in shutdown state.
     */
    bool IsShutdown() const;

private:
    Registry() = default;
    ~Registry() = default;
    Registry(Registry const&) = delete;
    Registry& operator=(Registry const&) = delete;
    
    struct Entry {
        SubsystemDescriptor desc;
        std::unique_ptr<ISubsystem> instance;
    };
    
    mutable std::mutex m_mutex;
    std::map<void const*, Entry> m_entries;
    std::map<std::string, void const*> m_nameToTypeInfo;  // name -> typeInfo mapping
    bool m_shutdown = false;
    
    static Registry* s_instance;
    static std::mutex s_instanceMutex;
    
    // Friend declarations for detail namespace functions
    // These functions need access to private members m_mutex, m_entries, m_shutdown
    friend ISubsystem* te::subsystems::detail::GetSubsystemByTypeInfo(void const*);
    friend std::vector<te::subsystems::detail::SubsystemEntry> te::subsystems::detail::GetEntriesForLifecycle();
    friend void te::subsystems::detail::SetRegistryShutdown(bool);
};

#include "te/subsystems/registry.inl"

}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_REGISTRY_HPP
