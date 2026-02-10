/**
 * @file EntityManager.cpp
 * @brief Entity manager implementation
 */

#include <te/entity/EntityManager.h>
#include <te/entity/Entity.h>
#include <te/scene/SceneManager.h>
#include <algorithm>

namespace te {
namespace entity {

namespace {
    EntityManager* g_entityManager = nullptr;
}

EntityManager& EntityManager::GetInstance() {
    if (!g_entityManager) {
        g_entityManager = new EntityManager();
    }
    return *g_entityManager;
}

EntityManager* GetEntityManager() {
    return g_entityManager;
}

Entity* EntityManager::CreateEntity(te::scene::WorldRef world, char const* name) {
    Entity* entity = Entity::Create(world, name);
    if (!entity) {
        return nullptr;
    }
    
    // Register Entity
    m_entities[entity->GetEntityId()] = entity;
    
    // Register by name if provided
    if (name && strlen(name) > 0) {
        m_nameToEntity[world][std::string(name)] = entity;
    }
    
    return entity;
}

Entity* EntityManager::CreateEntityFromNode(te::scene::NodeId nodeId, te::scene::WorldRef world) {
    Entity* entity = Entity::CreateFromNode(nodeId, world);
    if (!entity) {
        return nullptr;
    }
    
    // Register Entity
    m_entities[entity->GetEntityId()] = entity;
    
    // Register by name if available
    char const* name = entity->GetName();
    if (name && strlen(name) > 0) {
        m_nameToEntity[world][std::string(name)] = entity;
    }
    
    return entity;
}

void EntityManager::DestroyEntity(EntityId entityId) {
    auto it = m_entities.find(entityId);
    if (it != m_entities.end()) {
        Entity* entity = it->second;
        DestroyEntity(entity);
    }
}

void EntityManager::DestroyEntity(Entity* entity) {
    if (!entity) {
        return;
    }

    EntityId entityId = entity->GetEntityId();
    te::scene::WorldRef world = entity->GetWorldRef();
    char const* name = entity->GetName();

    // Remove from entities map
    m_entities.erase(entityId);

    // Remove from name map (use entity's world so we find the correct bucket)
    if (world.IsValid() && name && strlen(name) > 0) {
        auto worldIt = m_nameToEntity.find(world);
        if (worldIt != m_nameToEntity.end()) {
            worldIt->second.erase(std::string(name));
        }
    }

    // Delete Entity (this will unregister from SceneManager)
    delete entity;
}

Entity* EntityManager::GetEntity(EntityId entityId) {
    auto it = m_entities.find(entityId);
    if (it != m_entities.end()) {
        return it->second;
    }
    return nullptr;
}

Entity* EntityManager::FindEntityByName(te::scene::WorldRef world, char const* name) {
    if (!name || strlen(name) == 0) {
        return nullptr;
    }
    
    auto worldIt = m_nameToEntity.find(world);
    if (worldIt != m_nameToEntity.end()) {
        auto nameIt = worldIt->second.find(std::string(name));
        if (nameIt != worldIt->second.end()) {
            return nameIt->second;
        }
    }
    return nullptr;
}

// Template implementations are in header file

void EntityManager::GetEntitiesInWorld(te::scene::WorldRef world, std::vector<Entity*>& out) {
    out.clear();
    for (auto& pair : m_entities) {
        Entity* entity = pair.second;
        if (entity && entity->GetWorldRef() == world) {
            out.push_back(entity);
        }
    }
}

}  // namespace entity
}  // namespace te
