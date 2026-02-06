/**
 * @file TypeRegistry.h
 * @brief Type registration and query system (contract: specs/_contracts/002-object-public-api.md).
 * Thread-safe type registry for runtime type information.
 */
#ifndef TE_OBJECT_TYPE_REGISTRY_H
#define TE_OBJECT_TYPE_REGISTRY_H

#include "te/object/TypeId.h"

namespace te {
namespace object {

/**
 * Type registry: thread-safe type registration and query system.
 */
class TypeRegistry {
public:
    /**
     * Register a type descriptor (thread-safe).
     * Returns true on success, false if TypeId already registered.
     */
    static bool RegisterType(TypeDescriptor const& desc);
    
    /**
     * Get type descriptor by name (thread-safe).
     * Returns nullptr if not found.
     */
    static TypeDescriptor const* GetTypeByName(char const* name);
    
    /**
     * Get type descriptor by ID (thread-safe).
     * Returns nullptr if not found.
     */
    static TypeDescriptor const* GetTypeById(TypeId id);
    
    /**
     * Create an instance of the type (uses Core Alloc).
     * Returns nullptr on failure.
     */
    static void* CreateInstance(TypeId id);
    
    /**
     * Create an instance of the type by name (uses Core Alloc).
     * Returns nullptr on failure.
     */
    static void* CreateInstance(char const* typeName);
    
    /**
     * Check if type is registered by ID.
     */
    static bool IsTypeRegistered(TypeId id);
    
    /**
     * Check if type is registered by name.
     */
    static bool IsTypeRegistered(char const* name);
    
    /**
     * Enumerate all registered types (for debugging/tools).
     * Calls callback for each registered type.
     */
    static void EnumerateTypes(void (*callback)(TypeDescriptor const*, void*), void* userData);
};

} // namespace object
} // namespace te

#endif // TE_OBJECT_TYPE_REGISTRY_H
