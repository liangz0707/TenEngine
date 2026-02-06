/**
 * @file DynamicNodeManager.h
 * @brief Dynamic node manager - uses linear list for fast add/remove
 */

#ifndef TE_SCENE_DYNAMIC_NODE_MANAGER_H
#define TE_SCENE_DYNAMIC_NODE_MANAGER_H

#include <te/scene/NodeManager.h>
#include <te/scene/ISceneNode.h>
#include <vector>
#include <unordered_set>
#include <functional>

namespace te {
namespace scene {

/**
 * @brief Dynamic node manager
 * 
 * Uses linear list for O(1) add/remove operations.
 * Suitable for frequently moving objects (characters, vehicles, etc.).
 */
class DynamicNodeManager : public INodeManager {
public:
    DynamicNodeManager() = default;
    ~DynamicNodeManager() override = default;
    
    void AddNode(ISceneNode* node) override;
    void RemoveNode(ISceneNode* node) override;
    void UpdateNode(ISceneNode* node) override;
    void Clear() override;
    size_t GetNodeCount() const override;
    void Traverse(std::function<void(ISceneNode*)> const& callback) const override;
    
private:
    std::vector<ISceneNode*> m_nodes;
    std::unordered_set<ISceneNode*> m_nodeSet;  // For fast lookup
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_DYNAMIC_NODE_MANAGER_H
