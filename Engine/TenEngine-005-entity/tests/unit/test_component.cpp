/**
 * @file test_component.cpp
 * @brief Unit tests for Component system
 * 
 * This test demonstrates how to implement and use custom components.
 * See docs/ComponentUsageGuide.md for detailed implementation guide.
 */

#include <te/entity/Entity.h>
#include <te/entity/Component.h>
#include <te/scene/SceneManager.h>
#include <te/scene/SceneTypes.h>
#include <cassert>

namespace te {
namespace entity {

// Example: Test component implementation
// This demonstrates how to create a custom component
struct TestComponent : public Component {
    int value = 0;
    bool attachedCalled = false;
    bool detachedCalled = false;
    
    void OnAttached(Entity* entity) override {
        attachedCalled = true;
    }
    
    void OnDetached(Entity* entity) override {
        detachedCalled = true;
    }
};

void TestAddComponent() {
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    Entity* entity = Entity::Create(world, "ComponentTest");
    assert(entity != nullptr);
    
    // Add component
    TestComponent* comp = entity->AddComponent<TestComponent>();
    assert(comp != nullptr);
    assert(comp->attachedCalled);
    
    // Set value
    comp->value = 42;
    
    // Get component
    TestComponent* retrieved = entity->GetComponent<TestComponent>();
    assert(retrieved != nullptr);
    assert(retrieved == comp);
    assert(retrieved->value == 42);
    
    // Check HasComponent
    assert(entity->HasComponent<TestComponent>());
    
    // Cleanup
    entity->Destroy();
    sceneMgr.DestroyWorld(world);
}

void TestRemoveComponent() {
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::WorldRef world = sceneMgr.CreateWorld(
        te::scene::SpatialIndexType::None,
        te::core::AABB{}
    );
    
    Entity* entity = Entity::Create(world, "RemoveComponentTest");
    assert(entity != nullptr);
    
    TestComponent* comp = entity->AddComponent<TestComponent>();
    assert(comp != nullptr);
    
    // Remove component
    entity->RemoveComponent<TestComponent>();
    assert(comp->detachedCalled);
    
    // Verify removed
    assert(!entity->HasComponent<TestComponent>());
    assert(entity->GetComponent<TestComponent>() == nullptr);
    
    // Cleanup
    entity->Destroy();
    sceneMgr.DestroyWorld(world);
}

}  // namespace entity
}  // namespace te

int test_component() {
    te::entity::TestAddComponent();
    te::entity::TestRemoveComponent();
    // Note: ModelComponent tests removed - ModelComponent should be implemented by World module
    return 0;
}
