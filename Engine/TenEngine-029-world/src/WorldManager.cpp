/**
 * @file WorldManager.cpp
 * @brief WorldManager implementation (029-World).
 */

#include <te/world/WorldManager.h>
#include <te/world/WorldModuleInit.h>
#include <te/world/LevelAssetDesc.h>
#include <te/world/LevelResource.h>
#include <te/world/ModelComponent.h>
#include <te/entity/Entity.h>
#include <te/entity/EntityManager.h>
#include <te/scene/SceneWorld.h>
#include <te/scene/ISceneNode.h>
#include <te/scene/SceneDesc.h>
#include <te/resource/Resource.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceTypes.h>
#include <algorithm>
#include <cstring>

namespace te {
namespace world {

namespace {
    WorldManager* g_instance = nullptr;
}

WorldManager& WorldManager::GetInstance() {
    if (!g_instance) {
        RegisterWorldModule();
        g_instance = new WorldManager();
    }
    return *g_instance;
}

WorldManager::LevelState* WorldManager::FindLevel(LevelHandle handle) const {
    if (!handle.IsValid()) return nullptr;
    for (auto& s : m_levels) {
        if (s.handle.value == handle.value) return const_cast<LevelState*>(&s);
    }
    return nullptr;
}

namespace {
void ConvertNodeToSceneDesc(SceneNodeDesc const& w, te::scene::SceneNodeDesc& out) {
    out.name = w.name.empty() ? nullptr : w.name.c_str();
    out.localTransform = w.localTransform;
    out.opaqueUserData = const_cast<SceneNodeDesc*>(&w);
    out.children.resize(w.children.size());
    for (size_t i = 0; i < w.children.size(); ++i) {
        ConvertNodeToSceneDesc(w.children[i], out.children[i]);
    }
}

void ConvertLevelToSceneDesc(LevelAssetDesc const& levelDesc, te::scene::SceneDesc& out) {
    out.roots.resize(levelDesc.roots.size());
    for (size_t i = 0; i < levelDesc.roots.size(); ++i) {
        ConvertNodeToSceneDesc(levelDesc.roots[i], out.roots[i]);
    }
}
}  // namespace

LevelHandle WorldManager::CreateLevelFromDesc(te::scene::SpatialIndexType indexType,
                                              te::core::AABB const& bounds,
                                              LevelAssetDesc const& desc) {
    te::scene::SceneDesc sceneDesc;
    ConvertLevelToSceneDesc(desc, sceneDesc);

    auto& sceneMgr = te::scene::SceneManager::GetInstance();
    auto& entityMgr = te::entity::EntityManager::GetInstance();

    te::scene::NodeFactoryFn factory = [&entityMgr](te::scene::SceneNodeDesc const& nodeDesc,
                                                    te::scene::WorldRef worldRef) -> te::scene::ISceneNode* {
        world::SceneNodeDesc const* w = static_cast<world::SceneNodeDesc const*>(nodeDesc.opaqueUserData);
        char const* name = (w && !w->name.empty()) ? w->name.c_str() : nullptr;
        te::entity::Entity* e = entityMgr.CreateEntity(worldRef, name);
        if (!e) return nullptr;
        e->SetLocalTransform(nodeDesc.localTransform);
        if (w && !w->modelGuid.IsNull()) {
            te::world::ModelComponent* comp = e->AddComponent<te::world::ModelComponent>();
            if (comp) comp->modelResourceId = w->modelGuid;
        }
        return e->GetSceneNode();
    };

    te::scene::WorldRef worldRef = sceneMgr.CreateSceneFromDesc(indexType, bounds, sceneDesc, factory);
    if (!worldRef.IsValid()) return LevelHandle();

    LevelState state;
    state.handle = LevelHandle(worldRef.value);
    state.sceneRef = worldRef;
    m_levels.push_back(state);
    m_currentLevel = state.handle;
    return state.handle;
}

LevelHandle WorldManager::CreateLevelFromDesc(te::scene::SpatialIndexType indexType,
                                              te::core::AABB const& bounds,
                                              te::resource::ResourceId levelResourceId) {
    te::resource::IResourceManager* mgr = te::resource::GetResourceManager();
    if (!mgr) return LevelHandle();
    char const* path = mgr->ResolvePath(levelResourceId);
    if (!path) return LevelHandle();
    te::resource::IResource* r = mgr->LoadSync(path, te::resource::ResourceType::Level);
    if (!r) return LevelHandle();
    ILevelResource* lr = dynamic_cast<ILevelResource*>(r);
    if (!lr) {
        r->Release();
        return LevelHandle();
    }
    LevelAssetDesc const& desc = lr->GetLevelAssetDesc();
    LevelHandle h = CreateLevelFromDesc(indexType, bounds, desc);
    r->Release();
    return h;
}

void WorldManager::UnloadLevel(LevelHandle handle) {
    LevelState* s = FindLevel(handle);
    if (!s) return;

    te::scene::SceneRef sceneRef = s->sceneRef;
    te::scene::SceneManager& sceneMgr = te::scene::SceneManager::GetInstance();
    te::scene::SceneWorld* world = sceneMgr.GetWorld(sceneRef);
    if (world) {
        std::vector<te::scene::ISceneNode*> all;
        world->Traverse([&all](te::scene::ISceneNode* n) { all.push_back(n); });
        for (te::scene::ISceneNode* n : all) {
            te::entity::Entity* e = dynamic_cast<te::entity::Entity*>(n);
            if (e) e->Destroy();
        }
    }
    sceneMgr.UnloadScene(sceneRef);
    m_levels.erase(std::remove_if(m_levels.begin(), m_levels.end(),
        [handle](LevelState const& x) { return x.handle.value == handle.value; }), m_levels.end());
    if (m_currentLevel.value == handle.value) m_currentLevel = LevelHandle();
}

te::scene::SceneRef WorldManager::GetSceneRef(LevelHandle handle) const {
    LevelState* s = FindLevel(handle);
    return s ? s->sceneRef : te::scene::SceneRef();
}

te::scene::SceneRef WorldManager::GetCurrentLevelScene() const {
    LevelState* s = FindLevel(m_currentLevel);
    return s ? s->sceneRef : te::scene::SceneRef();
}

void WorldManager::GetRootNodes(LevelHandle handle, std::vector<te::scene::ISceneNode*>& out) const {
    out.clear();
    LevelState* s = FindLevel(handle);
    if (!s || !s->sceneRef.IsValid()) return;
    te::scene::SceneWorld* world = te::scene::SceneManager::GetInstance().GetWorld(s->sceneRef);
    if (world) world->GetRootNodes(out);
}

void WorldManager::Traverse(LevelHandle handle, std::function<void(te::scene::ISceneNode*)> const& callback) const {
    LevelState* s = FindLevel(handle);
    if (!s || !s->sceneRef.IsValid()) return;
    te::scene::SceneManager::GetInstance().Traverse(s->sceneRef, callback);
}

namespace {
void ExportNodeToDesc(te::scene::ISceneNode* node, world::SceneNodeDesc& out) {
    if (!node) return;
    char const* name = node->GetName();
    out.name = name ? name : "";
    out.localTransform = node->GetLocalTransform();
    out.modelGuid = te::resource::ResourceId();
    te::entity::Entity* e = dynamic_cast<te::entity::Entity*>(node);
    if (e) {
        te::world::ModelComponent* comp = e->GetComponent<te::world::ModelComponent>();
        if (comp) out.modelGuid = comp->modelResourceId;
    }
    std::vector<te::scene::ISceneNode*> children;
    node->GetChildren(children);
    out.children.resize(children.size());
    for (size_t i = 0; i < children.size(); ++i) {
        ExportNodeToDesc(children[i], out.children[i]);
    }
}
}  // namespace

bool WorldManager::ExportLevelToDesc(LevelHandle handle, LevelAssetDesc& out) const {
    out.roots.clear();
    std::vector<te::scene::ISceneNode*> roots;
    GetRootNodes(handle, roots);
    out.roots.resize(roots.size());
    for (size_t i = 0; i < roots.size(); ++i) {
        ExportNodeToDesc(roots[i], out.roots[i]);
    }
    return true;
}

bool WorldManager::SaveLevel(LevelHandle handle, char const* path) const {
    if (!path || !handle.IsValid()) return false;
    te::resource::IResourceManager* mgr = te::resource::GetResourceManager();
    if (!mgr) return false;
    LevelAssetDesc desc;
    if (!ExportLevelToDesc(handle, desc)) return false;
    te::resource::IResource* r = CreateLevelResourceFromDesc(desc);
    if (!r) return false;
    bool ok = mgr->Save(r, path);
    r->Release();
    return ok;
}

}  // namespace world
}  // namespace te
