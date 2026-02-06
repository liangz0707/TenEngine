/**
 * @file SceneWorld.h
 * @brief Scene world container - manages a scene graph
 * Contract: specs/_contracts/004-scene-public-api.md
 */

#ifndef TE_SCENE_SCENE_WORLD_H
#define TE_SCENE_SCENE_WORLD_H

#include <te/scene/SceneTypes.h>
#include <te/scene/ISceneNode.h>
#include <te/scene/DynamicNodeManager.h>
#include <te/scene/StaticNodeManager.h>
#include <functional>
#include <vector>
#include <memory>

namespace te {
namespace scene {

/**
 * @brief Scene world - container for a scene graph
 * 
 * A SceneWorld contains a scene graph (tree of nodes) and manages
 * spatial indexing, transform updates, and queries.
 */
class SceneWorld {
public:
    /**
     * @brief Constructor
     * @param indexType Spatial index type (Octree, Quadtree, or None)
     * @param bounds World bounds (AABB) for spatial index
     */
    SceneWorld(SpatialIndexType indexType, te::core::AABB const& bounds);
    
    /**
     * @brief Destructor
     */
    ~SceneWorld();
    
    /**
     * @brief Get world reference
     * @return World reference
     */
    WorldRef GetWorldRef() const { return m_worldRef; }
    
    /**
     * @brief Register a node (does not take ownership)
     * @param node Node to register
     */
    void RegisterNode(ISceneNode* node);
    
    /**
     * @brief Unregister a node
     * @param node Node to unregister
     */
    void UnregisterNode(ISceneNode* node);
    
    /**
     * @brief Update transforms for all dirty nodes
     * 
     * Updates world transforms based on parent chain.
     * Uses iterative algorithm to avoid stack overflow.
     */
    void UpdateTransforms();
    
    /**
     * @brief Get root nodes (nodes without parent)
     * @param out Output vector to store root nodes
     */
    void GetRootNodes(std::vector<ISceneNode*>& out) const;
    
    /**
     * @brief Traverse scene graph (hierarchy traversal)
     * @param callback Callback function for each node
     */
    void Traverse(std::function<void(ISceneNode*)> const& callback) const;
    
    /**
     * @brief Find node by name
     * @param name Node name
     * @return Found node, or nullptr if not found
     */
    ISceneNode* FindNodeByName(char const* name) const;
    
    /**
     * @brief Find node by ID
     * @param id Node ID
     * @return Found node, or nullptr if not found
     */
    ISceneNode* FindNodeById(NodeId id) const;
    
    /**
     * @brief Get spatial index type
     * @return Spatial index type
     */
    SpatialIndexType GetSpatialIndexType() const { return m_indexType; }
    
private:
    WorldRef m_worldRef;
    SpatialIndexType m_indexType;
    te::core::AABB m_bounds;
    
    // Node managers
    std::unique_ptr<DynamicNodeManager> m_dynamicManager;
    std::unique_ptr<StaticNodeManager> m_staticManager;
    
    // All registered nodes (for fast lookup)
    std::vector<ISceneNode*> m_allNodes;
    
    // Root nodes cache
    mutable std::vector<ISceneNode*> m_rootNodesCache;
    mutable bool m_rootNodesCacheValid = false;
    
    void InvalidateRootNodesCache() { m_rootNodesCacheValid = false; }
    void UpdateRootNodesCache() const;
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_SCENE_WORLD_H
