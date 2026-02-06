/**
 * @file registry.inl
 * @brief GetSubsystem<T> template implementation.
 * This file is included inside the te::subsystems namespace in registry.hpp.
 * Note: registry.hpp already includes registry_detail.hpp, so detail::GetSubsystemByTypeInfo is visible.
 */
#ifndef TE_SUBSYSTEMS_REGISTRY_INL
#define TE_SUBSYSTEMS_REGISTRY_INL

#include "te/subsystems/subsystem.hpp"
#include <typeinfo>

// Note: This file is included inside te::subsystems namespace, so no namespace declaration here
// detail::GetSubsystemByTypeInfo is already declared in registry_detail.hpp which is included by registry.hpp
// ISubsystem is needed for dynamic_cast, so we include subsystem.hpp here

// Instance method implementation
template <typename T>
T* Registry::GetSubsystemImpl() {
    // GetSubsystemByTypeInfo already checks shutdown state and holds lock
    void const* key = &typeid(T);
    ISubsystem* subsystem = detail::GetSubsystemByTypeInfo(key);
    return dynamic_cast<T*>(subsystem);
}

// Static method implementation
template <typename T>
T* Registry::GetSubsystem() {
    return Registry::GetInstance().GetSubsystemImpl<T>();
}

#endif  // TE_SUBSYSTEMS_REGISTRY_INL
