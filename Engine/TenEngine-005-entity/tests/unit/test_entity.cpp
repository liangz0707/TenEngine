/**
 * @file test_entity.cpp
 * @brief Unit tests for Entity class
 */

#include <te/entity/Entity.h>
#include <te/entity/EntityId.h>
#include <te/scene/SceneManager.h>
#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <cassert>
#include <string>

namespace te {
namespace entity {

void TestEntityCreation() {
    // Create a world first
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    assert(world.IsValid());
    
    // Create an Entity
    Entity* entity = Entity::Create(world, "TestEntity");
    assert(entity != nullptr);
    assert(entity->GetEntityId().IsValid());
    
    // Check Entity ID
    EntityId id = entity->GetEntityId();
    assert(id.IsValid());
    
    // Check name
    assert(std::string(entity->GetName()) == "TestEntity");
    
    // Check initial state
    assert(entity->IsEnabled());
    assert(entity->IsActive());
    
    // Cleanup
    entity->Destroy();
    sceneMgr.DestroyWorld(world);
}

void TestEntityTransform() {
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    Entity* entity = Entity::Create(world, "TransformTest");
    assert(entity != nullptr);
    
    // Test local transform
    te::scene::Transform localTransform;
    localTransform.position = te::core::Vector3{1.0f, 2.0f, 3.0f};
    localTransform.rotation = te::core::Quaternion{0, 0, 0, 1};
    localTransform.scale = te::core::Vector3{1.0f, 1.0f, 1.0f};
    
    entity->SetLocalTransform(localTransform);
    te::scene::Transform const& retrieved = entity->GetLocalTransform();
    assert(retrieved.position.x == 1.0f);
    assert(retrieved.position.y == 2.0f);
    assert(retrieved.position.z == 3.0f);
    
    // Test dirty flag
    assert(entity->IsDirty());
    
    // Cleanup
    entity->Destroy();
    sceneMgr.DestroyWorld(world);
}

void TestEntityHierarchy() {
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    Entity* parent = Entity::Create(world, "Parent");
    Entity* child = Entity::Create(world, "Child");
    
    assert(parent != nullptr);
    assert(child != nullptr);
    
    // Set parent-child relationship
    child->SetParent(parent);
    assert(child->GetParent() == parent);
    assert(parent->GetChildCount() == 1);
    
    // Check children list
    std::vector<te::scene::ISceneNode*> children;
    parent->GetChildren(children);
    assert(children.size() == 1);
    assert(children[0] == child);
    
    // Cleanup
    child->Destroy();
    parent->Destroy();
    sceneMgr.DestroyWorld(world);
}

void TestEntityEnabled() {
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    Entity* entity = Entity::Create(world, "EnabledTest");
    assert(entity != nullptr);
    
    // Test SetEnabled
    entity->SetEnabled(false);
    assert(!entity->IsEnabled());
    assert(!entity->IsActive());
    
    entity->SetEnabled(true);
    assert(entity->IsEnabled());
    assert(entity->IsActive());
    
    // Cleanup
    entity->Destroy();
    sceneMgr.DestroyWorld(world);
}

}  // namespace entity
}  // namespace te

int test_entity() {
    te::entity::TestEntityCreation();
    te::entity::TestEntityTransform();
    te::entity::TestEntityHierarchy();
    te::entity::TestEntityEnabled();
    return 0;
}
