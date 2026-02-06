/**
 * @file ComponentRegistry.h
 * @brief Component type registry (integration with 002-Object)
 * Contract: specs/_contracts/005-entity-public-api.md
 */

#ifndef TE_ENTITY_COMPONENT_REGISTRY_H
#define TE_ENTITY_COMPONENT_REGISTRY_H

#include <te/object/TypeId.h>
#include <te/object/TypeRegistry.h>
#include <unordered_map>
#include <string>

namespace te {
namespace entity {

// Forward declaration
struct IComponentTypeInfo;

/**
 * @brief Component registry interface
 * 
 * Provides component type registration and query capabilities.
 * Integrates with 002-Object TypeRegistry for reflection support.
 */
class IComponentRegistry {
public:
    virtual ~IComponentRegistry() = default;
    
    /**
     * @brief Register a component type
     * @tparam T Component type
     * @param name Component type name
     * 
     * Registers the component type with both Entity module
     * and 002-Object TypeRegistry for reflection support.
     */
    template<typename T>
    void RegisterComponentType(char const* name);
    
    /**
     * @brief Get component type info by type ID
     * @param id Type ID
     * @return Component type info, or nullptr if not found
     */
    virtual IComponentTypeInfo const* GetComponentTypeInfo(te::object::TypeId id) const = 0;
    
    /**
     * @brief Get component type info by type name
     * @param name Type name
     * @return Component type info, or nullptr if not found
     */
    virtual IComponentTypeInfo const* GetComponentTypeInfo(char const* name) const = 0;
    
    /**
     * @brief Check if a component type is registered
     * @param id Type ID
     * @return true if registered
     */
    virtual bool IsComponentTypeRegistered(te::object::TypeId id) const = 0;
};

/**
 * @brief Component type info
 */
struct IComponentTypeInfo {
    te::object::TypeId typeId;
    char const* name;
    std::size_t size;
    
    IComponentTypeInfo(te::object::TypeId id, char const* n, std::size_t s)
        : typeId(id), name(n), size(s) {}
};

/**
 * @brief Get component registry singleton
 * @return Component registry instance
 */
IComponentRegistry* GetComponentRegistry();

// Forward declaration of helper function (implemented in ComponentRegistry.cpp)
template<typename T>
void RegisterComponentTypeHelper(char const* name);

// Template implementation (must be in header)
template<typename T>
void IComponentRegistry::RegisterComponentType(char const* name) {
    // Call helper function which accesses the implementation
    RegisterComponentTypeHelper<T>(name);
}

}  // namespace entity
}  // namespace te

#endif  // TE_ENTITY_COMPONENT_REGISTRY_H
