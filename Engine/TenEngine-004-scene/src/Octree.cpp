/**
 * @file Octree.cpp
 * @brief Octree implementation
 */

#include <te/scene/Octree.h>
#include <te/scene/SpatialQuery.h>
#include <te/scene/SceneTypes.h>
#include <algorithm>
#include <unordered_set>
#include <functional>

namespace te {
namespace scene {

Octree::Octree(te::core::AABB const& bounds, int maxDepth, int maxNodesPerLeaf)
    : m_maxDepth(maxDepth)
    , m_maxNodesPerLeaf(maxNodesPerLeaf)
{
    m_root = std::make_unique<OctreeNode>(bounds);
}

void Octree::Insert(ISceneNode* node) {
    if (!node || !node->HasAABB()) {
        return;
    }
    
    InsertRecursive(m_root.get(), node, 0);
    m_nodeCount++;
}

void Octree::InsertRecursive(OctreeNode* node, ISceneNode* sceneNode, int depth) {
    if (!node || !sceneNode) {
        return;
    }
    
    te::core::AABB nodeAABB = sceneNode->GetAABB();
    
    // Check if node AABB intersects this octree node
    if (!SpatialQuery::AABBIntersects(node->bounds, nodeAABB)) {
        return;
    }
    
    if (node->isLeaf) {
        // Add to leaf
        node->nodes.push_back(sceneNode);
        
        // Split if needed
        if (node->nodes.size() > static_cast<size_t>(m_maxNodesPerLeaf) && depth < m_maxDepth) {
            SplitNode(node);
        }
    } else {
        // Insert into appropriate child
        te::core::Vector3 center;
        center.x = (nodeAABB.min.x + nodeAABB.max.x) * 0.5f;
        center.y = (nodeAABB.min.y + nodeAABB.max.y) * 0.5f;
        center.z = (nodeAABB.min.z + nodeAABB.max.z) * 0.5f;
        
        int octant = GetOctant(node->bounds, center);
        if (node->children[octant]) {
            InsertRecursive(node->children[octant].get(), sceneNode, depth + 1);
        }
    }
}

void Octree::Remove(ISceneNode* node) {
    if (!node) {
        return;
    }
    
    RemoveRecursive(m_root.get(), node);
    m_nodeCount--;
}

void Octree::RemoveRecursive(OctreeNode* node, ISceneNode* sceneNode) {
    if (!node) {
        return;
    }
    
    if (node->isLeaf) {
        auto it = std::find(node->nodes.begin(), node->nodes.end(), sceneNode);
        if (it != node->nodes.end()) {
            node->nodes.erase(it);
        }
    } else {
        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                RemoveRecursive(node->children[i].get(), sceneNode);
            }
        }
    }
}

void Octree::Update(ISceneNode* node) {
    // Remove and reinsert
    Remove(node);
    Insert(node);
}

void Octree::Clear() {
    m_root = std::make_unique<OctreeNode>(m_root->bounds);
    m_nodeCount = 0;
}

void Octree::QueryFrustum(Frustum const& frustum,
                         std::function<void(ISceneNode*)> const& callback) const {
    std::unordered_set<ISceneNode*> visited;
    QueryFrustumRecursive(m_root.get(), frustum, callback, visited);
}

void Octree::QueryFrustumRecursive(OctreeNode const* node,
                                   Frustum const& frustum,
                                   std::function<void(ISceneNode*)> const& callback,
                                   std::unordered_set<ISceneNode*>& visited) const {
    if (!node) {
        return;
    }
    
    // Test if octree node intersects frustum
    if (!SpatialQuery::FrustumIntersectsAABB(frustum, node->bounds)) {
        return;
    }
    
    if (node->isLeaf) {
        for (ISceneNode* sceneNode : node->nodes) {
            if (visited.find(sceneNode) == visited.end()) {
                visited.insert(sceneNode);
                if (sceneNode->IsActive() && sceneNode->HasAABB()) {
                    te::core::AABB aabb = sceneNode->GetAABB();
                    if (SpatialQuery::FrustumIntersectsAABB(frustum, aabb)) {
                        callback(sceneNode);
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                QueryFrustumRecursive(node->children[i].get(), frustum, callback, visited);
            }
        }
    }
}

void Octree::QueryAABB(te::core::AABB const& aabb,
                      std::function<void(ISceneNode*)> const& callback) const {
    std::unordered_set<ISceneNode*> visited;
    QueryAABBRecursive(m_root.get(), aabb, callback, visited);
}

void Octree::QueryAABBRecursive(OctreeNode const* node,
                                te::core::AABB const& aabb,
                                std::function<void(ISceneNode*)> const& callback,
                                std::unordered_set<ISceneNode*>& visited) const {
    if (!node) {
        return;
    }
    
    // Test if octree node intersects query AABB
    if (!SpatialQuery::AABBIntersects(node->bounds, aabb)) {
        return;
    }
    
    if (node->isLeaf) {
        for (ISceneNode* sceneNode : node->nodes) {
            if (visited.find(sceneNode) == visited.end()) {
                visited.insert(sceneNode);
                if (sceneNode->IsActive() && sceneNode->HasAABB()) {
                    te::core::AABB nodeAABB = sceneNode->GetAABB();
                    if (SpatialQuery::AABBIntersects(nodeAABB, aabb)) {
                        callback(sceneNode);
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                QueryAABBRecursive(node->children[i].get(), aabb, callback, visited);
            }
        }
    }
}

size_t Octree::GetNodeCount() const {
    return m_nodeCount;
}

void Octree::SplitNode(OctreeNode* node) {
    if (!node || !node->isLeaf) {
        return;
    }
    
    node->isLeaf = false;
    
    te::core::Vector3 center;
    center.x = (node->bounds.min.x + node->bounds.max.x) * 0.5f;
    center.y = (node->bounds.min.y + node->bounds.max.y) * 0.5f;
    center.z = (node->bounds.min.z + node->bounds.max.z) * 0.5f;
    
    // Create 8 children
    for (int i = 0; i < 8; ++i) {
        node->children[i] = std::make_unique<OctreeNode>(GetOctantBounds(node->bounds, i));
    }
    
    // Redistribute nodes
    std::vector<ISceneNode*> nodesToRedistribute = node->nodes;
    node->nodes.clear();
    
    for (ISceneNode* sceneNode : nodesToRedistribute) {
        te::core::AABB nodeAABB = sceneNode->GetAABB();
        te::core::Vector3 nodeCenter;
        nodeCenter.x = (nodeAABB.min.x + nodeAABB.max.x) * 0.5f;
        nodeCenter.y = (nodeAABB.min.y + nodeAABB.max.y) * 0.5f;
        nodeCenter.z = (nodeAABB.min.z + nodeAABB.max.z) * 0.5f;
        
        int octant = GetOctant(node->bounds, nodeCenter);
        if (node->children[octant]) {
            node->children[octant]->nodes.push_back(sceneNode);
        }
    }
}


int Octree::GetOctant(te::core::AABB const& bounds, te::core::Vector3 const& point) const {
    te::core::Vector3 center;
    center.x = (bounds.min.x + bounds.max.x) * 0.5f;
    center.y = (bounds.min.y + bounds.max.y) * 0.5f;
    center.z = (bounds.min.z + bounds.max.z) * 0.5f;
    
    int octant = 0;
    if (point.x >= center.x) octant |= 1;
    if (point.y >= center.y) octant |= 2;
    if (point.z >= center.z) octant |= 4;
    
    return octant;
}

te::core::AABB Octree::GetOctantBounds(te::core::AABB const& parent, int octant) const {
    te::core::Vector3 center;
    center.x = (parent.min.x + parent.max.x) * 0.5f;
    center.y = (parent.min.y + parent.max.y) * 0.5f;
    center.z = (parent.min.z + parent.max.z) * 0.5f;
    
    te::core::AABB bounds;
    
    if (octant & 1) {
        bounds.min.x = center.x;
        bounds.max.x = parent.max.x;
    } else {
        bounds.min.x = parent.min.x;
        bounds.max.x = center.x;
    }
    
    if (octant & 2) {
        bounds.min.y = center.y;
        bounds.max.y = parent.max.y;
    } else {
        bounds.min.y = parent.min.y;
        bounds.max.y = center.y;
    }
    
    if (octant & 4) {
        bounds.min.z = center.z;
        bounds.max.z = parent.max.z;
    } else {
        bounds.min.z = parent.min.z;
        bounds.max.z = center.z;
    }
    
    return bounds;
}

}  // namespace scene
}  // namespace te
