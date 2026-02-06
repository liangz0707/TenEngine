/**
 * @file Octree.h
 * @brief Octree spatial index implementation for 3D scenes
 */

#ifndef TE_SCENE_OCTREE_H
#define TE_SCENE_OCTREE_H

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
 * @brief Octree node
 */
struct OctreeNode {
    te::core::AABB bounds;
    std::vector<ISceneNode*> nodes;
    std::unique_ptr<OctreeNode> children[8];
    bool isLeaf = true;
    
    OctreeNode(te::core::AABB const& b) : bounds(b) {}
};

/**
 * @brief Octree spatial index
 * 
 * 3D spatial index using octree structure.
 * Each node divides space into 8 octants.
 */
class Octree : public ISpatialIndex {
public:
    /**
     * @brief Constructor
     * @param bounds World bounds
     * @param maxDepth Maximum tree depth
     * @param maxNodesPerLeaf Maximum nodes per leaf before splitting
     */
    Octree(te::core::AABB const& bounds, 
           int maxDepth = 10, 
           int maxNodesPerLeaf = 10);
    
    ~Octree() override = default;
    
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
    std::unique_ptr<OctreeNode> m_root;
    int m_maxDepth;
    int m_maxNodesPerLeaf;
    size_t m_nodeCount = 0;
    
    void InsertRecursive(OctreeNode* node, ISceneNode* sceneNode, int depth);
    void RemoveRecursive(OctreeNode* node, ISceneNode* sceneNode);
    void UpdateRecursive(OctreeNode* node, ISceneNode* sceneNode);
    void SplitNode(OctreeNode* node);
    void QueryFrustumRecursive(OctreeNode const* node,
                               Frustum const& frustum,
                               std::function<void(ISceneNode*)> const& callback,
                               std::unordered_set<ISceneNode*>& visited) const;
    void QueryAABBRecursive(OctreeNode const* node,
                           te::core::AABB const& aabb,
                           std::function<void(ISceneNode*)> const& callback,
                           std::unordered_set<ISceneNode*>& visited) const;
    int GetOctant(te::core::AABB const& bounds, te::core::Vector3 const& point) const;
    te::core::AABB GetOctantBounds(te::core::AABB const& parent, int octant) const;
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_OCTREE_H
