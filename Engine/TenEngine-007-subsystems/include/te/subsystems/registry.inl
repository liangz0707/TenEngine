/**
 * @file registry.inl
 * @brief GetSubsystem<T> template implementation.
 */
#ifndef TE_SUBSYSTEMS_REGISTRY_INL
#define TE_SUBSYSTEMS_REGISTRY_INL

#include "te/subsystems/detail/registry_state.hpp"
#include <typeinfo>

namespace te {
namespace subsystems {

namespace detail {
ISubsystem* GetSubsystemByTypeInfo(void const* typeInfo);
}  // namespace detail

template <typename T>
T* Registry::GetSubsystem() {
    if (detail::IsShutdown())  // from registry_state.hpp
        return nullptr;
    void const* key = &typeid(T);
    return dynamic_cast<T*>(detail::GetSubsystemByTypeInfo(key));
}

}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_REGISTRY_INL
