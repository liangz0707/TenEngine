/**
 * @file test_scene_world.cpp
 * @brief Unit tests for SceneWorld
 */

#include <te/scene/SceneWorld.h>
#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <te/core/engine.h>
#include <cassert>
#include <vector>

namespace te {
namespace scene {

// Use MockNode from test_scene_manager.cpp
extern class MockNode;

void TestSceneWorldCreation() {
    te::core::AABB bounds;
    bounds.min = {0, 0, 0};
    bounds.max = {100, 100, 100};
    
    SceneWorld world(SpatialIndexType::Octree, bounds);
    assert(world.GetWorldRef().IsValid());
    assert(world.GetSpatialIndexType() == SpatialIndexType::Octree);
}

void TestSceneWorldNodeRegistration() {
    te::core::AABB bounds;
    bounds.min = {0, 0, 0};
    bounds.max = {100, 100, 100};
    
    SceneWorld world(SpatialIndexType::None, bounds);
    
    // Create mock nodes (simplified)
    // In real test, would use MockNode class
    
    std::vector<ISceneNode*> rootNodes;
    world.GetRootNodes(rootNodes);
    assert(rootNodes.empty());
}

void RunTestSceneWorld() {
    TestSceneWorldCreation();
    TestSceneWorldNodeRegistration();
}

}  // namespace scene
}  // namespace te
