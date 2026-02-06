/**
 * @file SceneManager.h
 * @brief Scene manager singleton - manages all scene worlds
 * Contract: specs/_contracts/004-scene-public-api.md
 */

#ifndef TE_SCENE_SCENE_MANAGER_H
#define TE_SCENE_SCENE_MANAGER_H

#include <te/scene/SceneTypes.h>
#include <te/scene/ISceneNode.h>
#include <te/core/math.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace te {
namespace scene {

// Forward declaration
class SceneWorld;

/**
 * @brief Scene manager singleton
 * 
 * Manages all scene worlds, provides global scene management API.
 */
class SceneManager {
public:
    /**
     * @brief Get singleton instance
     * @return Scene manager instance
     */
    static SceneManager& GetInstance();
    
    /**
     * @brief Create a new scene world
     * @param indexType Spatial index type (Octree, Quadtree, or None)
     * @param bounds World bounds (AABB) for spatial index
     * @return World reference
     */
    WorldRef CreateWorld(SpatialIndexType indexType, 
                        te::core::AABB const& bounds);
    
    /**
     * @brief Destroy a scene world
     * @param world World reference to destroy
     */
    void DestroyWorld(WorldRef world);
    
    /**
     * @brief Get active world
     * @return Active world reference, or invalid WorldRef if none
     */
    WorldRef GetActiveWorld() const;
    
    /**
     * @brief Set active world
     * @param world World reference to set as active
     */
    void SetActiveWorld(WorldRef world);
    
    /**
     * @brief Register a node (does not take ownership)
     * @param node Node to register
     * 
     * Node must belong to a world. The node's world is determined
     * by finding which world contains it (via parent chain).
     */
    void RegisterNode(ISceneNode* node);
    
    /**
     * @brief Unregister a node
     * @param node Node to unregister
     */
    void UnregisterNode(ISceneNode* node);
    
    /**
     * @brief Update transforms for a world
     * @param world World reference
     */
    void UpdateTransforms(WorldRef world);
    
    /**
     * @brief Move a node (update its position)
     * @param node Node to move
     * @param position New position
     */
    void MoveNode(ISceneNode* node, te::core::Vector3 const& position);
    
    /**
     * @brief Convert node to static type
     * @param node Node to convert
     * @return true if conversion successful
     */
    bool ConvertToStatic(ISceneNode* node);
    
    /**
     * @brief Convert node to dynamic type
     * @param node Node to convert
     * @return true if conversion successful
     */
    bool ConvertToDynamic(ISceneNode* node);
    
    /**
     * @brief Traverse a world (hierarchy traversal)
     * @param world World reference
     * @param callback Callback function for each node
     */
    void Traverse(WorldRef world, 
                 std::function<void(ISceneNode*)> const& callback);
    
    /**
     * @brief Find node by name in a world
     * @param world World reference
     * @param name Node name
     * @return Found node, or nullptr if not found
     */
    ISceneNode* FindNodeByName(WorldRef world, char const* name);
    
    /**
     * @brief Find node by ID in a world
     * @param world World reference
     * @param id Node ID
     * @return Found node, or nullptr if not found
     */
    ISceneNode* FindNodeById(WorldRef world, NodeId id);
    
    /**
     * @brief Get world by reference
     * @param world World reference
     * @return SceneWorld pointer, or nullptr if invalid
     */
    SceneWorld* GetWorld(WorldRef world) const;
    
private:
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(SceneManager const&) = delete;
    SceneManager& operator=(SceneManager const&) = delete;
    
    // World storage
    std::unordered_map<void*, std::unique_ptr<SceneWorld>> m_worlds;
    WorldRef m_activeWorld;
    
    // Node to world mapping (for fast lookup)
    std::unordered_map<ISceneNode*, WorldRef> m_nodeToWorld;
    
    // Find which world a node belongs to
    WorldRef FindNodeWorld(ISceneNode* node) const;
    
    // Next world ID for unique references
    uintptr_t m_nextWorldId = 1;
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_SCENE_MANAGER_H
