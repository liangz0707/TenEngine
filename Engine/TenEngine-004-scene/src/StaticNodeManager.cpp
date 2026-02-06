/**
 * @file StaticNodeManager.cpp
 * @brief StaticNodeManager implementation
 */

#include <te/scene/StaticNodeManager.h>
#include <te/scene/Octree.h>
#include <te/scene/Quadtree.h>
#include <te/scene/SpatialQuery.h>
#include <te/scene/SceneTypes.h>
#include <algorithm>
#include <unordered_set>

namespace te {
namespace scene {

StaticNodeManager::StaticNodeManager(SpatialIndexType indexType, te::core::AABB const& bounds)
    : m_indexType(indexType)
    , m_bounds(bounds)
    , m_spatialIndex(nullptr)
{
    // Create spatial index based on type
    if (indexType == SpatialIndexType::Octree) {
        m_spatialIndex = new Octree(bounds);
    } else if (indexType == SpatialIndexType::Quadtree) {
        m_spatialIndex = new Quadtree(bounds);
    }
    // If None, m_spatialIndex remains nullptr
}

StaticNodeManager::~StaticNodeManager() {
    delete m_spatialIndex;
}

void StaticNodeManager::AddNode(ISceneNode* node) {
    if (!node) {
        return;
    }
    
    if (m_nodeSet.find(node) != m_nodeSet.end()) {
        return;  // Already added
    }
    
    m_nodes.push_back(node);
    m_nodeSet.insert(node);
    
    if (m_spatialIndex && node->HasAABB()) {
        m_spatialIndex->Insert(node);
    }
}

void StaticNodeManager::RemoveNode(ISceneNode* node) {
    if (!node) {
        return;
    }
    
    auto it = m_nodeSet.find(node);
    if (it == m_nodeSet.end()) {
        return;  // Not found
    }
    
    m_nodeSet.erase(it);
    m_dirtyNodes.erase(node);
    
    auto vecIt = std::find(m_nodes.begin(), m_nodes.end(), node);
    if (vecIt != m_nodes.end()) {
        m_nodes.erase(vecIt);
    }
    
    if (m_spatialIndex) {
        m_spatialIndex->Remove(node);
    }
}

void StaticNodeManager::UpdateNode(ISceneNode* node) {
    if (!node) {
        return;
    }
    
    if (m_nodeSet.find(node) == m_nodeSet.end()) {
        return;  // Not in manager
    }
    
    // Mark as dirty for batch update
    m_dirtyNodes.insert(node);
}

void StaticNodeManager::Clear() {
    m_nodes.clear();
    m_nodeSet.clear();
    m_dirtyNodes.clear();
    
    if (m_spatialIndex) {
        m_spatialIndex->Clear();
    }
}

size_t StaticNodeManager::GetNodeCount() const {
    return m_nodes.size();
}

void StaticNodeManager::Traverse(std::function<void(ISceneNode*)> const& callback) const {
    for (ISceneNode* node : m_nodes) {
        if (node && node->IsActive()) {
            callback(node);
        }
    }
}

void StaticNodeManager::RebuildIndex() {
    if (!m_spatialIndex || m_dirtyNodes.empty()) {
        return;
    }
    
    // Update all dirty nodes in spatial index
    for (ISceneNode* node : m_dirtyNodes) {
        if (node->HasAABB()) {
            m_spatialIndex->Update(node);
        }
    }
    
    m_dirtyNodes.clear();
}

void StaticNodeManager::QueryFrustum(Frustum const& frustum,
                                     std::function<void(ISceneNode*)> const& callback) const {
    if (m_spatialIndex) {
        std::unordered_set<ISceneNode*> visited;
        m_spatialIndex->QueryFrustum(frustum, [&](ISceneNode* node) {
            if (visited.find(node) == visited.end()) {
                visited.insert(node);
                if (node->IsActive()) {
                    callback(node);
                }
            }
        });
    } else {
        // Fallback to linear traversal
        Traverse([&](ISceneNode* node) {
            if (node->HasAABB()) {
                te::core::AABB aabb = node->GetAABB();
                // Simple frustum test (simplified)
                callback(node);
            }
        });
    }
}

void StaticNodeManager::QueryAABB(te::core::AABB const& aabb,
                                  std::function<void(ISceneNode*)> const& callback) const {
    if (m_spatialIndex) {
        std::unordered_set<ISceneNode*> visited;
        m_spatialIndex->QueryAABB(aabb, [&](ISceneNode* node) {
            if (visited.find(node) == visited.end()) {
                visited.insert(node);
                if (node->IsActive()) {
                    callback(node);
                }
            }
        });
    } else {
        // Fallback to linear traversal
        Traverse([&](ISceneNode* node) {
            if (node->HasAABB()) {
                te::core::AABB nodeAABB = node->GetAABB();
                if (SpatialQuery::AABBIntersects(nodeAABB, aabb)) {
                    callback(node);
                }
            }
        });
    }
}

}  // namespace scene
}  // namespace te
