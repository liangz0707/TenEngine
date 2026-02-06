/**
 * @file DynamicNodeManager.cpp
 * @brief DynamicNodeManager implementation
 */

#include <te/scene/DynamicNodeManager.h>
#include <algorithm>

namespace te {
namespace scene {

void DynamicNodeManager::AddNode(ISceneNode* node) {
    if (!node) {
        return;
    }
    
    if (m_nodeSet.find(node) != m_nodeSet.end()) {
        return;  // Already added
    }
    
    m_nodes.push_back(node);
    m_nodeSet.insert(node);
}

void DynamicNodeManager::RemoveNode(ISceneNode* node) {
    if (!node) {
        return;
    }
    
    auto it = m_nodeSet.find(node);
    if (it == m_nodeSet.end()) {
        return;  // Not found
    }
    
    m_nodeSet.erase(it);
    
    auto vecIt = std::find(m_nodes.begin(), m_nodes.end(), node);
    if (vecIt != m_nodes.end()) {
        m_nodes.erase(vecIt);
    }
}

void DynamicNodeManager::UpdateNode(ISceneNode* node) {
    // Dynamic nodes don't need spatial index update
    // Transform changes are handled immediately
    (void)node;
}

void DynamicNodeManager::Clear() {
    m_nodes.clear();
    m_nodeSet.clear();
}

size_t DynamicNodeManager::GetNodeCount() const {
    return m_nodes.size();
}

void DynamicNodeManager::Traverse(std::function<void(ISceneNode*)> const& callback) const {
    for (ISceneNode* node : m_nodes) {
        if (node && node->IsActive()) {
            callback(node);
        }
    }
}

}  // namespace scene
}  // namespace te
