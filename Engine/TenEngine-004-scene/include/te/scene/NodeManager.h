/**
 * @file NodeManager.h
 * @brief Node manager interface - base class for dynamic/static node managers
 */

#ifndef TE_SCENE_NODE_MANAGER_H
#define TE_SCENE_NODE_MANAGER_H

#include <te/scene/ISceneNode.h>
#include <te/scene/SceneTypes.h>
#include <vector>
#include <functional>

namespace te {
namespace scene {

/**
 * @brief Node manager interface
 * 
 * Base interface for managing nodes with different strategies.
 */
class INodeManager {
public:
    virtual ~INodeManager() = default;
    
    /**
     * @brief Add a node
     * @param node Node to add
     */
    virtual void AddNode(ISceneNode* node) = 0;
    
    /**
     * @brief Remove a node
     * @param node Node to remove
     */
    virtual void RemoveNode(ISceneNode* node) = 0;
    
    /**
     * @brief Update node (for spatial index rebuild)
     * @param node Node to update
     */
    virtual void UpdateNode(ISceneNode* node) = 0;
    
    /**
     * @brief Clear all nodes
     */
    virtual void Clear() = 0;
    
    /**
     * @brief Get node count
     * @return Number of nodes
     */
    virtual size_t GetNodeCount() const = 0;
    
    /**
     * @brief Traverse all nodes
     * @param callback Callback function for each node
     */
    virtual void Traverse(std::function<void(ISceneNode*)> const& callback) const = 0;
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_NODE_MANAGER_H
