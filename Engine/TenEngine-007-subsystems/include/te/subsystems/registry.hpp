/**
 * @file registry.hpp
 * @brief Registry: Register, GetSubsystem<T>, Unregister (contract: 007-subsystems-public-api.md).
 * Only contract-declared types and API.
 */
#ifndef TE_SUBSYSTEMS_REGISTRY_HPP
#define TE_SUBSYSTEMS_REGISTRY_HPP

#include <memory>

#include "te/subsystems/descriptor.hpp"
#include "te/subsystems/subsystem.hpp"

namespace te {
namespace subsystems {

/**
 * Subsystem registry: per-type singleton; duplicate Register returns false.
 * GetSubsystem returns nullptr when unregistered or after ShutdownAll.
 * Convention: desc.typeInfo = &typeid(ConcreteSubsystem) when registering.
 */
class Registry {
public:
    /** Register; returns false on duplicate type, instance ownership transferred. */
    static bool Register(SubsystemDescriptor const& desc, std::unique_ptr<ISubsystem> instance);

    /** Get subsystem by type; returns nullptr if unregistered or after ShutdownAll. */
    template <typename T>
    static T* GetSubsystem();

    /** Unregister by typeInfo. */
    static void Unregister(void const* typeInfo);
};

}  // namespace subsystems
}  // namespace te

#include "te/subsystems/registry.inl"

#endif  // TE_SUBSYSTEMS_REGISTRY_HPP
