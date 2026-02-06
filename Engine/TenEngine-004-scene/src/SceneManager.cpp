/**
 * @file SceneManager.cpp
 * @brief SceneManager implementation
 */

#include <te/scene/SceneManager.h>
#include <te/scene/SceneWorld.h>
#include <algorithm>

namespace te {
namespace scene {

SceneManager& SceneManager::GetInstance() {
    static SceneManager instance;
    return instance;
}

WorldRef SceneManager::CreateWorld(SpatialIndexType indexType, 
                                   te::core::AABB const& bounds) {
    auto world = std::make_unique<SceneWorld>(indexType, bounds);
    WorldRef worldRef = world->GetWorldRef();
    
    m_worlds[worldRef.value] = std::move(world);
    
    // Set as active if it's the first world
    if (!m_activeWorld.IsValid()) {
        m_activeWorld = worldRef;
    }
    
    return worldRef;
}

void SceneManager::DestroyWorld(WorldRef world) {
    if (!world.IsValid()) {
        return;
    }
    
    auto it = m_worlds.find(world.value);
    if (it == m_worlds.end()) {
        return;
    }
    
    // Remove all nodes from this world from node-to-world mapping
    SceneWorld* worldPtr = it->second.get();
    std::vector<ISceneNode*> nodesToRemove;
    
    // Collect all nodes in this world
    worldPtr->Traverse([&](ISceneNode* node) {
        nodesToRemove.push_back(node);
    });
    
    // Remove from mapping
    for (ISceneNode* node : nodesToRemove) {
        m_nodeToWorld.erase(node);
    }
    
    // Clear active world if it's being destroyed
    if (m_activeWorld == world) {
        m_activeWorld = WorldRef();
    }
    
    // Destroy world
    m_worlds.erase(it);
}

WorldRef SceneManager::GetActiveWorld() const {
    return m_activeWorld;
}

void SceneManager::SetActiveWorld(WorldRef world) {
    if (world.IsValid() && m_worlds.find(world.value) != m_worlds.end()) {
        m_activeWorld = world;
    }
}

void SceneManager::RegisterNode(ISceneNode* node) {
    if (!node) {
        return;
    }
    
    // Find which world this node belongs to
    WorldRef worldRef = FindNodeWorld(node);
    if (!worldRef.IsValid()) {
        return;  // Node doesn't belong to any world
    }
    
    SceneWorld* world = GetWorld(worldRef);
    if (world) {
        world->RegisterNode(node);
        m_nodeToWorld[node] = worldRef;
    }
}

void SceneManager::UnregisterNode(ISceneNode* node) {
    if (!node) {
        return;
    }
    
    auto it = m_nodeToWorld.find(node);
    if (it != m_nodeToWorld.end()) {
        SceneWorld* world = GetWorld(it->second);
        if (world) {
            world->UnregisterNode(node);
        }
        m_nodeToWorld.erase(it);
    }
}

void SceneManager::UpdateTransforms(WorldRef world) {
    SceneWorld* worldPtr = GetWorld(world);
    if (worldPtr) {
        worldPtr->UpdateTransforms();
    }
}

void SceneManager::MoveNode(ISceneNode* node, te::core::Vector3 const& position) {
    if (!node) {
        return;
    }
    
    Transform localTransform = node->GetLocalTransform();
    localTransform.position = position;
    node->SetLocalTransform(localTransform);
    
    // Mark dirty for transform update
    node->SetDirty(true);
}

bool SceneManager::ConvertToStatic(ISceneNode* node) {
    if (!node) {
        return false;
    }
    
    // Check if already static
    if (node->GetNodeType() == NodeType::Static) {
        return true;
    }
    
    // Find the world this node belongs to
    WorldRef worldRef = FindNodeWorld(node);
    if (!worldRef.IsValid()) {
        return false;
    }
    
    SceneWorld* world = GetWorld(worldRef);
    if (!world) {
        return false;
    }
    
    // Unregister from current manager (dynamic)
    world->UnregisterNode(node);
    
    // Change node type (node implementation should handle this)
    // Note: ISceneNode doesn't provide SetNodeType, so the node implementation
    // must handle the type change internally. We assume the node will return
    // Static from GetNodeType() after this call.
    
    // Re-register to static manager
    world->RegisterNode(node);
    
    return true;
}

bool SceneManager::ConvertToDynamic(ISceneNode* node) {
    if (!node) {
        return false;
    }
    
    // Check if already dynamic
    if (node->GetNodeType() == NodeType::Dynamic) {
        return true;
    }
    
    // Find the world this node belongs to
    WorldRef worldRef = FindNodeWorld(node);
    if (!worldRef.IsValid()) {
        return false;
    }
    
    SceneWorld* world = GetWorld(worldRef);
    if (!world) {
        return false;
    }
    
    // Unregister from current manager (static)
    world->UnregisterNode(node);
    
    // Change node type (node implementation should handle this)
    // Note: ISceneNode doesn't provide SetNodeType, so the node implementation
    // must handle the type change internally. We assume the node will return
    // Dynamic from GetNodeType() after this call.
    
    // Re-register to dynamic manager
    world->RegisterNode(node);
    
    return true;
}

void SceneManager::Traverse(WorldRef world, 
                           std::function<void(ISceneNode*)> const& callback) {
    SceneWorld* worldPtr = GetWorld(world);
    if (worldPtr) {
        worldPtr->Traverse(callback);
    }
}

ISceneNode* SceneManager::FindNodeByName(WorldRef world, char const* name) {
    SceneWorld* worldPtr = GetWorld(world);
    if (worldPtr) {
        return worldPtr->FindNodeByName(name);
    }
    return nullptr;
}

ISceneNode* SceneManager::FindNodeById(WorldRef world, NodeId id) {
    SceneWorld* worldPtr = GetWorld(world);
    if (worldPtr) {
        return worldPtr->FindNodeById(id);
    }
    return nullptr;
}

SceneWorld* SceneManager::GetWorld(WorldRef world) const {
    if (!world.IsValid()) {
        return nullptr;
    }
    
    auto it = m_worlds.find(world.value);
    if (it != m_worlds.end()) {
        return it->second.get();
    }
    
    return nullptr;
}

WorldRef SceneManager::FindNodeWorld(ISceneNode* node) const {
    if (!node) {
        return WorldRef();
    }
    
    // Check if node is already mapped
    auto it = m_nodeToWorld.find(node);
    if (it != m_nodeToWorld.end()) {
        return it->second;
    }
    
    // Try to find world by traversing parent chain
    ISceneNode* current = node;
    while (current) {
        auto it2 = m_nodeToWorld.find(current);
        if (it2 != m_nodeToWorld.end()) {
            return it2->second;
        }
        current = current->GetParent();
    }
    
    // If not found, try active world
    if (m_activeWorld.IsValid()) {
        return m_activeWorld;
    }
    
    return WorldRef();
}

}  // namespace scene
}  // namespace te
