/**
 * @file Quadtree.h
 * @brief Quadtree spatial index implementation for 2D scenes
 */

#ifndef TE_SCENE_QUADTREE_H
#define TE_SCENE_QUADTREE_H

#include <te/scene/SpatialIndex.h>
#include <te/scene/ISceneNode.h>
#include <te/core/math.h>
#include <vector>
#include <memory>
#include <unordered_set>
#include <functional>

namespace te {
namespace scene {

// Forward declaration
struct Frustum;

/**
 * @brief Quadtree node
 */
struct QuadTreeNode {
    te::core::AABB bounds;  // 2D bounds (z ignored or fixed)
    std::vector<ISceneNode*> nodes;
    std::unique_ptr<QuadTreeNode> children[4];
    bool isLeaf = true;
    
    QuadTreeNode(te::core::AABB const& b) : bounds(b) {}
};

/**
 * @brief Quadtree spatial index
 * 
 * 2D spatial index using quadtree structure.
 * Each node divides space into 4 quadrants.
 */
class Quadtree : public ISpatialIndex {
public:
    /**
     * @brief Constructor
     * @param bounds World bounds (2D, z ignored)
     * @param maxDepth Maximum tree depth
     * @param maxNodesPerLeaf Maximum nodes per leaf before splitting
     */
    Quadtree(te::core::AABB const& bounds,
             int maxDepth = 10,
             int maxNodesPerLeaf = 10);
    
    ~Quadtree() override = default;
    
    void Insert(ISceneNode* node) override;
    void Remove(ISceneNode* node) override;
    void Update(ISceneNode* node) override;
    void Clear() override;
    void QueryFrustum(Frustum const& frustum,
                     std::function<void(ISceneNode*)> const& callback) const override;
    void QueryAABB(te::core::AABB const& aabb,
                  std::function<void(ISceneNode*)> const& callback) const override;
    size_t GetNodeCount() const override;
    
private:
    std::unique_ptr<QuadTreeNode> m_root;
    int m_maxDepth;
    int m_maxNodesPerLeaf;
    size_t m_nodeCount = 0;
    
    void InsertRecursive(QuadTreeNode* node, ISceneNode* sceneNode, int depth);
    void RemoveRecursive(QuadTreeNode* node, ISceneNode* sceneNode);
    void UpdateRecursive(QuadTreeNode* node, ISceneNode* sceneNode);
    void SplitNode(QuadTreeNode* node);
    void QueryFrustumRecursive(QuadTreeNode const* node,
                              Frustum const& frustum,
                              std::function<void(ISceneNode*)> const& callback,
                              std::unordered_set<ISceneNode*>& visited) const;
    void QueryAABBRecursive(QuadTreeNode const* node,
                           te::core::AABB const& aabb,
                           std::function<void(ISceneNode*)> const& callback,
                           std::unordered_set<ISceneNode*>& visited) const;
    int GetQuadrant(te::core::AABB const& bounds, te::core::Vector3 const& point) const;
    te::core::AABB GetQuadrantBounds(te::core::AABB const& parent, int quadrant) const;
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_QUADTREE_H
