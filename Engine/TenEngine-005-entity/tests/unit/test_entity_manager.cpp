/**
 * @file test_entity_manager.cpp
 * @brief Unit tests for EntityManager
 */

#include <te/entity/Entity.h>
#include <te/entity/EntityManager.h>
#include <te/scene/SceneManager.h>
#include <te/scene/SceneTypes.h>
#include <cassert>
#include <vector>

namespace te {
namespace entity {

void TestEntityManagerCreate() {
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    EntityManager& mgr = EntityManager::GetInstance();
    
    // Create entity through manager
    Entity* entity = mgr.CreateEntity(world, "ManagerTest");
    assert(entity != nullptr);
    assert(std::string(entity->GetName()) == "ManagerTest");
    
    // Get entity by ID
    EntityId id = entity->GetEntityId();
    Entity* retrieved = mgr.GetEntity(id);
    assert(retrieved == entity);
    
    // Find entity by name
    Entity* found = mgr.FindEntityByName(world, "ManagerTest");
    assert(found == entity);
    
    // Cleanup
    mgr.DestroyEntity(entity);
    sceneMgr.DestroyWorld(world);
}

void TestEntityManagerDestroy() {
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    EntityManager& mgr = EntityManager::GetInstance();
    
    Entity* entity = mgr.CreateEntity(world, "DestroyTest");
    assert(entity != nullptr);
    
    EntityId id = entity->GetEntityId();
    
    // Destroy by ID
    mgr.DestroyEntity(id);
    
    // Verify destroyed
    Entity* retrieved = mgr.GetEntity(id);
    assert(retrieved == nullptr);
    
    sceneMgr.DestroyWorld(world);
}

void TestEntityManagerGetEntitiesInWorld() {
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world1 = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    te::scene::WorldRef world2 = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    EntityManager& mgr = EntityManager::GetInstance();
    
    Entity* e1 = mgr.CreateEntity(world1, "World1Entity1");
    Entity* e2 = mgr.CreateEntity(world1, "World1Entity2");
    Entity* e3 = mgr.CreateEntity(world2, "World2Entity1");
    
    // Get entities in world1
    std::vector<Entity*> world1Entities;
    mgr.GetEntitiesInWorld(world1, world1Entities);
    assert(world1Entities.size() == 2);
    
    // Get entities in world2
    std::vector<Entity*> world2Entities;
    mgr.GetEntitiesInWorld(world2, world2Entities);
    assert(world2Entities.size() == 1);
    assert(world2Entities[0] == e3);
    
    // Cleanup
    mgr.DestroyEntity(e1);
    mgr.DestroyEntity(e2);
    mgr.DestroyEntity(e3);
    sceneMgr.DestroyWorld(world1);
    sceneMgr.DestroyWorld(world2);
}

}  // namespace entity
}  // namespace te

int test_entity_manager() {
    te::entity::TestEntityManagerCreate();
    te::entity::TestEntityManagerDestroy();
    te::entity::TestEntityManagerGetEntitiesInWorld();
    return 0;
}
