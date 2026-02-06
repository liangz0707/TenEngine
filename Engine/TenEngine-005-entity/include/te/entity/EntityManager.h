/**
 * @file EntityManager.h
 * @brief Entity manager for Entity lifecycle and queries
 * Contract: specs/_contracts/005-entity-public-api.md
 */

#ifndef TE_ENTITY_ENTITY_MANAGER_H
#define TE_ENTITY_ENTITY_MANAGER_H

#include <te/entity/EntityId.h>
#include <te/entity/Entity.h>
#include <te/scene/SceneTypes.h>
#include <te/object/TypeId.h>
#include <vector>
#include <unordered_map>
#include <string>

namespace te {
namespace entity {

/**
 * @brief Entity manager
 * 
 * Manages Entity lifecycle, EntityId allocation, and Entity queries.
 * Integrates with SceneManager for node registration/unregistration.
 */
class EntityManager {
public:
    /**
     * @brief Get singleton instance
     * @return Entity manager instance
     */
    static EntityManager& GetInstance();
    
    /**
     * @brief Create a new Entity
     * @param world World reference
     * @param name Entity name (optional)
     * @return Created Entity, or nullptr on failure
     */
    Entity* CreateEntity(te::scene::WorldRef world, char const* name = nullptr);
    
    /**
     * @brief Create Entity from existing Scene node
     * @param nodeId Scene node ID
     * @param world World reference
     * @return Created Entity, or nullptr on failure
     * 
     * Note: Resource-related operations should be handled by World module.
     */
    Entity* CreateEntityFromNode(te::scene::NodeId nodeId, te::scene::WorldRef world);
    
    /**
     * @brief Destroy an Entity
     * @param entityId Entity ID
     */
    void DestroyEntity(EntityId entityId);
    
    /**
     * @brief Destroy an Entity
     * @param entity Entity pointer
     */
    void DestroyEntity(Entity* entity);
    
    /**
     * @brief Get Entity by ID
     * @param entityId Entity ID
     * @return Entity pointer, or nullptr if not found
     */
    Entity* GetEntity(EntityId entityId);
    
    /**
     * @brief Find Entity by name in a world
     * @param world World reference
     * @param name Entity name
     * @return Entity pointer, or nullptr if not found
     */
    Entity* FindEntityByName(te::scene::WorldRef world, char const* name);
    
    /**
     * @brief Query Entities with a specific component type
     * @tparam T Component type
     * @param out Output vector to store matching Entities
     */
    template<typename T>
    void QueryEntitiesWithComponent(std::vector<Entity*>& out) {
        out.clear();
        for (auto& pair : m_entities) {
            Entity* entity = pair.second;
            if (entity && entity->HasComponent<T>()) {
                out.push_back(entity);
            }
        }
    }
    
    /**
     * @brief Query Entities with multiple component types (AND query)
     * @tparam Components Component types
     * @param out Output vector to store matching Entities
     */
    template<typename... Components>
    void QueryEntitiesWithComponents(std::vector<Entity*>& out) {
        out.clear();
        for (auto& pair : m_entities) {
            Entity* entity = pair.second;
            if (entity && (entity->HasComponent<Components>() && ...)) {
                out.push_back(entity);
            }
        }
    }
    
    /**
     * @brief Get all Entities in a world
     * @param world World reference
     * @param out Output vector to store Entities
     */
    void GetEntitiesInWorld(te::scene::WorldRef world, std::vector<Entity*>& out);
    
private:
    EntityManager() = default;
    ~EntityManager() = default;
    EntityManager(EntityManager const&) = delete;
    EntityManager& operator=(EntityManager const&) = delete;
    
    // Entity storage
    std::unordered_map<EntityId, Entity*, EntityId::Hash> m_entities;
    
    // Name to Entity mapping (per world)
    // WorldRef hash support
    struct WorldRefHash {
        std::size_t operator()(te::scene::WorldRef const& ref) const {
            return std::hash<void*>{}(ref.value);
        }
    };
    
    std::unordered_map<te::scene::WorldRef, 
                      std::unordered_map<std::string, Entity*>,
                      WorldRefHash> m_nameToEntity;
    
    // Next Entity ID counter
    uintptr_t m_nextEntityId = 1;
};

/**
 * @brief Get Entity manager instance
 * @return Entity manager pointer
 */
EntityManager* GetEntityManager();

}  // namespace entity
}  // namespace te

#endif  // TE_ENTITY_ENTITY_MANAGER_H
