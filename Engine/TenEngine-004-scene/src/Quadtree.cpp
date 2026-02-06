/**
 * @file Quadtree.cpp
 * @brief Quadtree implementation
 */

#include <te/scene/Quadtree.h>
#include <te/scene/SpatialQuery.h>
#include <te/scene/SceneTypes.h>
#include <algorithm>
#include <unordered_set>
#include <functional>

namespace te {
namespace scene {

Quadtree::Quadtree(te::core::AABB const& bounds, int maxDepth, int maxNodesPerLeaf)
    : m_maxDepth(maxDepth)
    , m_maxNodesPerLeaf(maxNodesPerLeaf)
{
    m_root = std::make_unique<QuadTreeNode>(bounds);
}

void Quadtree::Insert(ISceneNode* node) {
    if (!node || !node->HasAABB()) {
        return;
    }
    
    InsertRecursive(m_root.get(), node, 0);
    m_nodeCount++;
}

void Quadtree::InsertRecursive(QuadTreeNode* node, ISceneNode* sceneNode, int depth) {
    if (!node || !sceneNode) {
        return;
    }
    
    te::core::AABB nodeAABB = sceneNode->GetAABB();
    
    // Check if node AABB intersects this quadtree node (2D check, ignore Z)
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
        center.z = 0.0f;  // Z ignored for 2D
        
        int quadrant = GetQuadrant(node->bounds, center);
        if (node->children[quadrant]) {
            InsertRecursive(node->children[quadrant].get(), sceneNode, depth + 1);
        }
    }
}

void Quadtree::Remove(ISceneNode* node) {
    if (!node) {
        return;
    }
    
    RemoveRecursive(m_root.get(), node);
    m_nodeCount--;
}

void Quadtree::RemoveRecursive(QuadTreeNode* node, ISceneNode* sceneNode) {
    if (!node) {
        return;
    }
    
    if (node->isLeaf) {
        auto it = std::find(node->nodes.begin(), node->nodes.end(), sceneNode);
        if (it != node->nodes.end()) {
            node->nodes.erase(it);
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            if (node->children[i]) {
                RemoveRecursive(node->children[i].get(), sceneNode);
            }
        }
    }
}

void Quadtree::Update(ISceneNode* node) {
    // Remove and reinsert
    Remove(node);
    Insert(node);
}

void Quadtree::Clear() {
    m_root = std::make_unique<QuadTreeNode>(m_root->bounds);
    m_nodeCount = 0;
}

void Quadtree::QueryFrustum(Frustum const& frustum,
                           std::function<void(ISceneNode*)> const& callback) const {
    std::unordered_set<ISceneNode*> visited;
    QueryFrustumRecursive(m_root.get(), frustum, callback, visited);
}

void Quadtree::QueryFrustumRecursive(QuadTreeNode const* node,
                                     Frustum const& frustum,
                                     std::function<void(ISceneNode*)> const& callback,
                                     std::unordered_set<ISceneNode*>& visited) const {
    if (!node) {
        return;
    }
    
    // Test if quadtree node intersects frustum (2D check)
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
        for (int i = 0; i < 4; ++i) {
            if (node->children[i]) {
                QueryFrustumRecursive(node->children[i].get(), frustum, callback, visited);
            }
        }
    }
}

void Quadtree::QueryAABB(te::core::AABB const& aabb,
                         std::function<void(ISceneNode*)> const& callback) const {
    std::unordered_set<ISceneNode*> visited;
    QueryAABBRecursive(m_root.get(), aabb, callback, visited);
}

void Quadtree::QueryAABBRecursive(QuadTreeNode const* node,
                                  te::core::AABB const& aabb,
                                  std::function<void(ISceneNode*)> const& callback,
                                  std::unordered_set<ISceneNode*>& visited) const {
    if (!node) {
        return;
    }
    
    // Test if quadtree node intersects query AABB (2D check)
    if (!SpatialQuery::AABBIntersects(node->bounds, aabb)) {
        return;
    }
    
    if (node->isLeaf) {
        for (ISceneNode* sceneNode : node->nodes) {
            if (visited.find(sceneNode) == visited.end()) {
                visited.insert(sceneNode);
                if (sceneNode->IsActive() && sceneNode->HasAABB()) {
                    te::core::AABB nodeAABB = sceneNode->GetAABB();
                    if (!(nodeAABB.max.x < aabb.min.x || nodeAABB.min.x > aabb.max.x ||
                          nodeAABB.max.y < aabb.min.y || nodeAABB.min.y > aabb.max.y)) {
                        callback(sceneNode);
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            if (node->children[i]) {
                QueryAABBRecursive(node->children[i].get(), aabb, callback, visited);
            }
        }
    }
}

size_t Quadtree::GetNodeCount() const {
    return m_nodeCount;
}

void Quadtree::SplitNode(QuadTreeNode* node) {
    if (!node || !node->isLeaf) {
        return;
    }
    
    node->isLeaf = false;
    
    te::core::Vector3 center;
    center.x = (node->bounds.min.x + node->bounds.max.x) * 0.5f;
    center.y = (node->bounds.min.y + node->bounds.max.y) * 0.5f;
    center.z = 0.0f;  // Z ignored for 2D
    
    // Create 4 children
    for (int i = 0; i < 4; ++i) {
        node->children[i] = std::make_unique<QuadTreeNode>(GetQuadrantBounds(node->bounds, i));
    }
    
    // Redistribute nodes
    std::vector<ISceneNode*> nodesToRedistribute = node->nodes;
    node->nodes.clear();
    
    for (ISceneNode* sceneNode : nodesToRedistribute) {
        te::core::AABB nodeAABB = sceneNode->GetAABB();
        te::core::Vector3 nodeCenter;
        nodeCenter.x = (nodeAABB.min.x + nodeAABB.max.x) * 0.5f;
        nodeCenter.y = (nodeAABB.min.y + nodeAABB.max.y) * 0.5f;
        nodeCenter.z = 0.0f;
        
        int quadrant = GetQuadrant(node->bounds, nodeCenter);
        if (node->children[quadrant]) {
            node->children[quadrant]->nodes.push_back(sceneNode);
        }
    }
}


int Quadtree::GetQuadrant(te::core::AABB const& bounds, te::core::Vector3 const& point) const {
    te::core::Vector3 center;
    center.x = (bounds.min.x + bounds.max.x) * 0.5f;
    center.y = (bounds.min.y + bounds.max.y) * 0.5f;
    
    int quadrant = 0;
    if (point.x >= center.x) quadrant |= 1;
    if (point.y >= center.y) quadrant |= 2;
    
    return quadrant;
}

te::core::AABB Quadtree::GetQuadrantBounds(te::core::AABB const& parent, int quadrant) const {
    te::core::Vector3 center;
    center.x = (parent.min.x + parent.max.x) * 0.5f;
    center.y = (parent.min.y + parent.max.y) * 0.5f;
    
    te::core::AABB bounds;
    
    if (quadrant & 1) {
        bounds.min.x = center.x;
        bounds.max.x = parent.max.x;
    } else {
        bounds.min.x = parent.min.x;
        bounds.max.x = center.x;
    }
    
    if (quadrant & 2) {
        bounds.min.y = center.y;
        bounds.max.y = parent.max.y;
    } else {
        bounds.min.y = parent.min.y;
        bounds.max.y = center.y;
    }
    
    // Keep Z bounds from parent
    bounds.min.z = parent.min.z;
    bounds.max.z = parent.max.z;
    
    return bounds;
}

}  // namespace scene
}  // namespace te
