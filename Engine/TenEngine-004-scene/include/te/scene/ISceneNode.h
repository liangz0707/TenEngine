/**
 * @file ISceneNode.h
 * @brief Scene node interface - all scene-managed nodes must implement this
 * Contract: specs/_contracts/004-scene-public-api.md
 */

#ifndef TE_SCENE_ISCENE_NODE_H
#define TE_SCENE_ISCENE_NODE_H

#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <vector>

namespace te {
namespace scene {

/**
 * @brief Scene node interface
 * 
 * All nodes managed by Scene module must implement this interface.
 * Scene module does not own node objects - ownership is managed by World/Entity modules.
 */
class ISceneNode {
public:
    virtual ~ISceneNode() = default;
    
    // ========== Hierarchy ==========
    
    /**
     * @brief Get parent node
     * @return Parent node, or nullptr if root node
     */
    virtual ISceneNode* GetParent() const = 0;
    
    /**
     * @brief Set parent node
     * @param parent New parent node, or nullptr to make this a root node
     * 
     * After setting parent, the node should be marked dirty.
     * Circular references should be rejected.
     */
    virtual void SetParent(ISceneNode* parent) = 0;
    
    /**
     * @brief Get children nodes
     * @param out Output vector to store children
     */
    virtual void GetChildren(std::vector<ISceneNode*>& out) const = 0;
    
    /**
     * @brief Get child count
     * @return Number of children
     */
    virtual size_t GetChildCount() const = 0;
    
    // ========== Transform ==========
    
    /**
     * @brief Get local transform
     * @return Local transform (position, rotation, scale)
     */
    virtual Transform const& GetLocalTransform() const = 0;
    
    /**
     * @brief Set local transform
     * @param t Local transform
     * 
     * After setting transform, the node should be marked dirty.
     */
    virtual void SetLocalTransform(Transform const& t) = 0;
    
    /**
     * @brief Get world transform
     * @return World transform (calculated from parent chain)
     * 
     * World transform is updated by UpdateTransforms().
     */
    virtual Transform const& GetWorldTransform() const = 0;
    
    /**
     * @brief Get world matrix (4x4 transformation matrix)
     * @return World transformation matrix
     */
    virtual te::core::Matrix4 const& GetWorldMatrix() const = 0;
    
    // ========== Node Identity ==========
    
    /**
     * @brief Get node ID
     * @return Node ID
     */
    virtual NodeId GetNodeId() const = 0;
    
    /**
     * @brief Get node name
     * @return Node name (null-terminated string)
     */
    virtual char const* GetName() const = 0;
    
    // ========== Active State ==========
    
    /**
     * @brief Check if node is active
     * @return true if node and all parents are active
     * 
     * If parent is inactive, this node is considered inactive.
     */
    virtual bool IsActive() const = 0;
    
    /**
     * @brief Set active state
     * @param active Active state
     */
    virtual void SetActive(bool active) = 0;
    
    // ========== Node Type ==========
    
    /**
     * @brief Get node type (Static or Dynamic)
     * @return Node type
     */
    virtual NodeType GetNodeType() const = 0;
    
    // ========== AABB (Optional, for spatial queries) ==========
    
    /**
     * @brief Check if node has AABB
     * @return true if node provides AABB for spatial queries
     * 
     * Default implementation returns false.
     * Override this and GetAABB() to enable spatial queries.
     */
    virtual bool HasAABB() const { return false; }
    
    /**
     * @brief Get AABB (axis-aligned bounding box)
     * @return AABB in world space
     * 
     * Default implementation returns empty AABB.
     * Only called if HasAABB() returns true.
     */
    virtual te::core::AABB GetAABB() const { 
        return te::core::AABB{};
    }
    
    // ========== Dirty Flag (Internal use) ==========
    
    /**
     * @brief Check if node is dirty (needs transform update)
     * @return true if node needs transform update
     */
    virtual bool IsDirty() const = 0;
    
    /**
     * @brief Set dirty flag
     * @param dirty Dirty state
     */
    virtual void SetDirty(bool dirty) = 0;
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_ISCENE_NODE_H
