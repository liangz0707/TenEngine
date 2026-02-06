/**
 * @file SpatialIndex.h
 * @brief Spatial index interface - base for octree/quadtree
 */

#ifndef TE_SCENE_SPATIAL_INDEX_H
#define TE_SCENE_SPATIAL_INDEX_H

#include <te/scene/ISceneNode.h>
#include <te/core/math.h>
#include <functional>
#include <vector>

namespace te {
namespace scene {

// Forward declaration
struct Frustum;

/**
 * @brief Spatial index interface
 * 
 * Base interface for spatial indexing structures (octree, quadtree).
 */
class ISpatialIndex {
public:
    virtual ~ISpatialIndex() = default;
    
    /**
     * @brief Insert a node
     * @param node Node to insert
     */
    virtual void Insert(ISceneNode* node) = 0;
    
    /**
     * @brief Remove a node
     * @param node Node to remove
     */
    virtual void Remove(ISceneNode* node) = 0;
    
    /**
     * @brief Update a node (reinsert if moved)
     * @param node Node to update
     */
    virtual void Update(ISceneNode* node) = 0;
    
    /**
     * @brief Clear all nodes
     */
    virtual void Clear() = 0;
    
    /**
     * @brief Query nodes in frustum
     * @param frustum Frustum
     * @param callback Callback function
     */
    virtual void QueryFrustum(Frustum const& frustum,
                              std::function<void(ISceneNode*)> const& callback) const = 0;
    
    /**
     * @brief Query nodes intersecting AABB
     * @param aabb Query AABB
     * @param callback Callback function
     */
    virtual void QueryAABB(te::core::AABB const& aabb,
                           std::function<void(ISceneNode*)> const& callback) const = 0;
    
    /**
     * @brief Get node count
     * @return Number of nodes
     */
    virtual size_t GetNodeCount() const = 0;
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_SPATIAL_INDEX_H
