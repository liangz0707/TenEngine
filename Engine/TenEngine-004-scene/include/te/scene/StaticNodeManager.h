/**
 * @file StaticNodeManager.h
 * @brief Static node manager - uses spatial index for efficient queries
 */

#ifndef TE_SCENE_STATIC_NODE_MANAGER_H
#define TE_SCENE_STATIC_NODE_MANAGER_H

#include <te/scene/NodeManager.h>
#include <te/scene/ISceneNode.h>
#include <te/scene/SpatialIndex.h>
#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <vector>
#include <unordered_set>
#include <functional>

namespace te {
namespace scene {

/**
 * @brief Static node manager
 * 
 * Uses spatial index (octree/quadtree) for O(log n) queries.
 * Suitable for static objects (terrain, buildings, etc.).
 */
class StaticNodeManager : public INodeManager {
public:
    /**
     * @brief Constructor
     * @param indexType Spatial index type (Octree or Quadtree)
     * @param bounds World bounds for spatial index
     */
    StaticNodeManager(SpatialIndexType indexType, te::core::AABB const& bounds);
    ~StaticNodeManager() override;
    
    void AddNode(ISceneNode* node) override;
    void RemoveNode(ISceneNode* node) override;
    void UpdateNode(ISceneNode* node) override;
    void Clear() override;
    size_t GetNodeCount() const override;
    void Traverse(std::function<void(ISceneNode*)> const& callback) const override;
    
    /**
     * @brief Rebuild spatial index (for dirty nodes)
     */
    void RebuildIndex();
    
    /**
     * @brief Query nodes in frustum
     * @param frustum Frustum
     * @param callback Callback function
     */
    void QueryFrustum(Frustum const& frustum,
                      std::function<void(ISceneNode*)> const& callback) const;
    
    /**
     * @brief Query nodes intersecting AABB
     * @param aabb Query AABB
     * @param callback Callback function
     */
    void QueryAABB(te::core::AABB const& aabb,
                   std::function<void(ISceneNode*)> const& callback) const;
    
private:
    SpatialIndexType m_indexType;
    te::core::AABB m_bounds;
    ISpatialIndex* m_spatialIndex;
    std::vector<ISceneNode*> m_nodes;
    std::unordered_set<ISceneNode*> m_dirtyNodes;  // Nodes that need spatial index update
    std::unordered_set<ISceneNode*> m_nodeSet;     // For fast lookup
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_STATIC_NODE_MANAGER_H
