/**
 * @file ComponentQuery.h
 * @brief Component query system for Entity queries
 * Contract: specs/_contracts/005-entity-public-api.md
 */

#ifndef TE_ENTITY_COMPONENT_QUERY_H
#define TE_ENTITY_COMPONENT_QUERY_H

#include <te/entity/Entity.h>
#include <te/entity/EntityManager.h>
#include <vector>
#include <functional>
#include <tuple>
#include <type_traits>

namespace te {
namespace entity {

/**
 * @brief Component query system
 * 
 * Provides query and iteration interfaces for Entities based on component types.
 * Supports single component queries and multi-component AND queries.
 */
class ComponentQuery {
public:
    /**
     * @brief Query Entities with one or more component types (AND query)
     * @tparam Components Component types (single or multiple)
     * @param out Output vector to store matching Entities
     */
    template<typename... Components>
    static void Query(std::vector<Entity*>& out);
    
    /**
     * @brief Iterate over Entities with one component type
     * @tparam T Component type
     * @param callback Callback function called for each matching Entity
     */
    template<typename T>
    static void ForEach(std::function<void(Entity*, T*)> const& callback);

    /**
     * @brief Iterate over Entities with multiple component types
     * @tparam Components Component types (two or more)
     * @param callback Callback function called for each matching Entity
     */
    template<typename... Components>
    static void ForEach(std::function<void(Entity*, Components*...)> const& callback);
};

// Template implementations (single and multi-component use the same variadic path)
template<typename... Components>
void ComponentQuery::Query(std::vector<Entity*>& out) {
    EntityManager& mgr = EntityManager::GetInstance();
    mgr.QueryEntitiesWithComponents<Components...>(out);
}

template<typename T>
void ComponentQuery::ForEach(std::function<void(Entity*, T*)> const& callback) {
    std::vector<Entity*> entities;
    Query<T>(entities);
    
    for (Entity* entity : entities) {
        T* comp = entity->GetComponent<T>();
        if (comp) {
            callback(entity, comp);
        }
    }
}

template<typename... Components>
void ComponentQuery::ForEach(std::function<void(Entity*, Components*...)> const& callback) {
    std::vector<Entity*> entities;
    Query<Components...>(entities);
    
    for (Entity* entity : entities) {
        auto comps = std::make_tuple(entity->GetComponent<Components>()...);
        if ((entity->GetComponent<Components>() != nullptr && ...)) {
            std::apply([&](Components*... compPtrs) {
                callback(entity, compPtrs...);
            }, comps);
        }
    }
}

}  // namespace entity
}  // namespace te

#endif  // TE_ENTITY_COMPONENT_QUERY_H
