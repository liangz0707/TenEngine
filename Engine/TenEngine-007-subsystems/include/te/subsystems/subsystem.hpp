/**
 * @file subsystem.hpp
 * @brief ISubsystem interface (contract: 007-subsystems-public-api.md).
 * Only contract-declared types and API.
 */
#ifndef TE_SUBSYSTEMS_SUBSYSTEM_HPP
#define TE_SUBSYSTEMS_SUBSYSTEM_HPP

#include "te/subsystems/descriptor.hpp"

namespace te {
namespace subsystems {

/**
 * Subsystem instance interface; lifecycle hooks called by main loop only.
 * Per-type singleton; GetSubsystem<T> returns the unique instance.
 */
class ISubsystem {
public:
    virtual ~ISubsystem() = default;
    
    /**
     * Initialize subsystem. Returns true on success, false on failure.
     * Called before Start() in dependency order.
     */
    virtual bool Initialize() = 0;
    
    /**
     * Start subsystem. Called after Initialize() in dependency order.
     */
    virtual void Start() = 0;
    
    /**
     * Stop subsystem. Called before Shutdown() in reverse dependency order.
     */
    virtual void Stop() = 0;
    
    /**
     * Shutdown subsystem. Called after Stop() in reverse dependency order.
     */
    virtual void Shutdown() = 0;
    
    /**
     * Get subsystem descriptor. Must return a valid reference.
     */
    virtual SubsystemDescriptor const& GetDescriptor() const = 0;
    
    /**
     * Get subsystem name. Must return a non-null string.
     */
    virtual char const* GetName() const = 0;
};

}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_SUBSYSTEM_HPP
