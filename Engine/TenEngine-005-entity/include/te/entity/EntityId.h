/**
 * @file EntityId.h
 * @brief Entity ID type definition
 * Contract: specs/_contracts/005-entity-public-api.md
 */

#ifndef TE_ENTITY_ENTITY_ID_H
#define TE_ENTITY_ENTITY_ID_H

#include <cstdint>
#include <cstddef>
#include <functional>

namespace te {
namespace entity {

// Forward declaration
class Entity;

/**
 * @brief Entity unique identifier
 * 
 * EntityId is an opaque handle that uniquely identifies an Entity.
 * It corresponds to a Scene node (1:1 mapping).
 */
struct EntityId {
    void* value = nullptr;
    
    EntityId() = default;
    explicit EntityId(void* v) : value(v) {}
    
    bool IsValid() const { return value != nullptr; }
    bool operator==(EntityId const& other) const { return value == other.value; }
    bool operator!=(EntityId const& other) const { return value != other.value; }
    
    // Hash support for unordered containers
    struct Hash {
        std::size_t operator()(EntityId const& id) const {
            return std::hash<void*>{}(id.value);
        }
    };
};

}  // namespace entity
}  // namespace te

#endif  // TE_ENTITY_ENTITY_ID_H
