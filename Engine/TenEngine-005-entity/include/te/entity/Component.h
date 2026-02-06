/**
 * @file Component.h
 * @brief Component base class and ComponentHandle
 * Contract: specs/_contracts/005-entity-public-api.md
 */

#ifndef TE_ENTITY_COMPONENT_H
#define TE_ENTITY_COMPONENT_H

#include <te/entity/EntityId.h>
#include <te/object/TypeId.h>
#include <cstddef>

namespace te {
namespace entity {

// Forward declaration
class Entity;

/**
 * @brief Base class for all components
 * 
 * All components inherit from Component.
 * Components can override OnAttached/OnDetached for lifecycle callbacks.
 */
class Component {
public:
    virtual ~Component() = default;
    
    /**
     * @brief Called when component is attached to an entity
     * @param entity The entity this component is attached to
     */
    virtual void OnAttached(Entity* entity) {}
    
    /**
     * @brief Called when component is detached from an entity
     * @param entity The entity this component was attached to
     */
    virtual void OnDetached(Entity* entity) {}
};

/**
 * @brief Component handle for cross-boundary access
 * 
 * ComponentHandle provides a way to reference a component
 * without holding a direct pointer, useful for serialization
 * and cross-module boundaries.
 */
struct ComponentHandle {
    EntityId entityId;
    te::object::TypeId componentTypeId;
    void* componentPtr;
    
    ComponentHandle() 
        : entityId(), componentTypeId(0), componentPtr(nullptr) {}
    
    ComponentHandle(EntityId eid, te::object::TypeId tid, void* ptr)
        : entityId(eid), componentTypeId(tid), componentPtr(ptr) {}
    
    bool IsValid() const {
        return entityId.IsValid() && componentTypeId != 0 && componentPtr != nullptr;
    }
};

}  // namespace entity
}  // namespace te

#endif  // TE_ENTITY_COMPONENT_H
