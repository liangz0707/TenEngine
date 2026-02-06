/**
 * @file test_component_query.cpp
 * @brief Unit tests for ComponentQuery
 */

#include <te/entity/Entity.h>
#include <te/entity/Component.h>
#include <te/entity/ComponentQuery.h>
#include <te/scene/SceneManager.h>
#include <te/scene/SceneTypes.h>
#include <cassert>
#include <vector>

namespace te {
namespace entity {

// Test components
struct ComponentA : public Component {
    int a = 0;
};

struct ComponentB : public Component {
    int b = 0;
};

struct ComponentC : public Component {
    int c = 0;
};

void TestComponentQuerySingle() {
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    // Create entities with different components
    Entity* entity1 = Entity::Create(world, "Entity1");
    Entity* entity2 = Entity::Create(world, "Entity2");
    Entity* entity3 = Entity::Create(world, "Entity3");
    
    entity1->AddComponent<ComponentA>()->a = 1;
    entity2->AddComponent<ComponentA>()->a = 2;
    entity2->AddComponent<ComponentB>()->b = 20;
    entity3->AddComponent<ComponentB>()->b = 30;
    
    // Query entities with ComponentA
    std::vector<Entity*> results;
    ComponentQuery::Query<ComponentA>(results);
    assert(results.size() == 2);
    assert(results[0] == entity1 || results[1] == entity1);
    assert(results[0] == entity2 || results[1] == entity2);
    
    // Query entities with ComponentB
    results.clear();
    ComponentQuery::Query<ComponentB>(results);
    assert(results.size() == 2);
    assert(results[0] == entity2 || results[1] == entity2);
    assert(results[0] == entity3 || results[1] == entity3);
    
    // Cleanup
    entity1->Destroy();
    entity2->Destroy();
    entity3->Destroy();
    sceneMgr.DestroyWorld(world);
}

void TestComponentQueryMultiple() {
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    // Create entities
    Entity* entity1 = Entity::Create(world, "Entity1");
    Entity* entity2 = Entity::Create(world, "Entity2");
    Entity* entity3 = Entity::Create(world, "Entity3");
    
    entity1->AddComponent<ComponentA>()->a = 1;
    entity2->AddComponent<ComponentA>()->a = 2;
    entity2->AddComponent<ComponentB>()->b = 20;
    entity3->AddComponent<ComponentA>()->a = 3;
    entity3->AddComponent<ComponentB>()->b = 30;
    entity3->AddComponent<ComponentC>()->c = 300;
    
    // Query entities with both ComponentA and ComponentB
    std::vector<Entity*> results;
    ComponentQuery::Query<ComponentA, ComponentB>(results);
    assert(results.size() == 2);
    assert(results[0] == entity2 || results[1] == entity2);
    assert(results[0] == entity3 || results[1] == entity3);
    
    // Query entities with ComponentA, ComponentB, and ComponentC
    results.clear();
    ComponentQuery::Query<ComponentA, ComponentB, ComponentC>(results);
    assert(results.size() == 1);
    assert(results[0] == entity3);
    
    // Cleanup
    entity1->Destroy();
    entity2->Destroy();
    entity3->Destroy();
    sceneMgr.DestroyWorld(world);
}

void TestComponentQueryForEach() {
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    Entity* entity1 = Entity::Create(world, "Entity1");
    Entity* entity2 = Entity::Create(world, "Entity2");
    
    entity1->AddComponent<ComponentA>()->a = 10;
    entity2->AddComponent<ComponentA>()->a = 20;
    
    int sum = 0;
    ComponentQuery::ForEach<ComponentA>([&sum](Entity* e, ComponentA* comp) {
        sum += comp->a;
    });
    
    assert(sum == 30);
    
    // Cleanup
    entity1->Destroy();
    entity2->Destroy();
    sceneMgr.DestroyWorld(world);
}

}  // namespace entity
}  // namespace te

int test_component_query() {
    te::entity::TestComponentQuerySingle();
    te::entity::TestComponentQueryMultiple();
    te::entity::TestComponentQueryForEach();
    return 0;
}
