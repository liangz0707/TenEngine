/**
 * @file test_world_manager.cpp
 * @brief Unit tests for WorldManager: CreateLevelFromDesc, UnloadLevel, GetSceneRef, Traverse
 */

#include <te/world/WorldManager.h>
#include <te/world/WorldTypes.h>
#include <te/world/LevelAssetDesc.h>
#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <cassert>
#include <vector>
#include <string>

namespace te {
namespace world {

static int test_create_level_from_desc() {
    WorldManager& wm = WorldManager::GetInstance();
    te::core::AABB bounds;
    bounds.min.x = bounds.min.y = bounds.min.z = 0.f;
    bounds.max.x = bounds.max.y = bounds.max.z = 100.f;

    LevelAssetDesc desc;
    desc.roots.resize(1);
    desc.roots[0].name = "Root";
    desc.roots[0].modelGuid = te::resource::ResourceId();

    LevelHandle h = wm.CreateLevelFromDesc(te::scene::SpatialIndexType::None, bounds, desc);
    assert(h.IsValid());
    te::scene::SceneRef sr = wm.GetSceneRef(h);
    assert(sr.IsValid());
    te::scene::SceneRef current = wm.GetCurrentLevelScene();
    assert(current.IsValid() && current.value == sr.value);
    wm.UnloadLevel(h);
    assert(!wm.GetSceneRef(h).IsValid());
    return 0;
}

static int test_get_scene_ref_and_current() {
    WorldManager& wm = WorldManager::GetInstance();
    te::core::AABB bounds;
    bounds.min.x = bounds.min.y = bounds.min.z = 0.f;
    bounds.max.x = bounds.max.y = bounds.max.z = 100.f;

    LevelAssetDesc desc;
    desc.roots.resize(1);
    desc.roots[0].name = "A";

    LevelHandle h = wm.CreateLevelFromDesc(te::scene::SpatialIndexType::None, bounds, desc);
    assert(h.IsValid());
    te::scene::SceneRef sr = wm.GetSceneRef(h);
    assert(sr.IsValid());
    assert(wm.GetCurrentLevelScene().IsValid());
    wm.UnloadLevel(h);
    assert(!wm.GetSceneRef(h).IsValid());
    assert(!wm.GetCurrentLevelScene().IsValid());
    return 0;
}

static int test_traverse() {
    WorldManager& wm = WorldManager::GetInstance();
    te::core::AABB bounds;
    bounds.min.x = bounds.min.y = bounds.min.z = 0.f;
    bounds.max.x = bounds.max.y = bounds.max.z = 100.f;

    LevelAssetDesc desc;
    desc.roots.resize(1);
    desc.roots[0].name = "Single";
    desc.roots[0].children.resize(1);
    desc.roots[0].children[0].name = "Child";

    LevelHandle h = wm.CreateLevelFromDesc(te::scene::SpatialIndexType::None, bounds, desc);
    assert(h.IsValid());

    int count = 0;
    wm.Traverse(h, [&count](te::scene::ISceneNode* n) {
        (void)n;
        ++count;
    });
    assert(count == 2);

    std::vector<te::scene::ISceneNode*> roots;
    wm.GetRootNodes(h, roots);
    assert(roots.size() == 1);

    wm.UnloadLevel(h);
    return 0;
}

static int test_unload_order() {
    WorldManager& wm = WorldManager::GetInstance();
    te::core::AABB bounds;
    bounds.min.x = bounds.min.y = bounds.min.z = 0.f;
    bounds.max.x = bounds.max.y = bounds.max.z = 100.f;

    LevelAssetDesc desc;
    desc.roots.resize(1);
    desc.roots[0].name = "ToUnload";

    LevelHandle h = wm.CreateLevelFromDesc(te::scene::SpatialIndexType::None, bounds, desc);
    assert(h.IsValid());
    wm.UnloadLevel(h);
    assert(!wm.GetSceneRef(h).IsValid());
    wm.UnloadLevel(h);
    return 0;
}

int test_world_manager() {
    int r = 0;
    r |= test_create_level_from_desc();
    r |= test_get_scene_ref_and_current();
    r |= test_traverse();
    r |= test_unload_order();
    return r;
}

}  // namespace world
}  // namespace te
