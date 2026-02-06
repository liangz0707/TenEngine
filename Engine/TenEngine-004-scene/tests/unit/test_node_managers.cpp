/**
 * @file test_node_managers.cpp
 * @brief Unit tests for DynamicNodeManager and StaticNodeManager
 */

#include <te/scene/DynamicNodeManager.h>
#include <te/scene/StaticNodeManager.h>
#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <te/core/engine.h>
#include <cassert>
#include <functional>

namespace te {
namespace scene {

// Simplified mock node for testing
class SimpleMockNode : public ISceneNode {
public:
    SimpleMockNode() : m_active(true), m_dirty(false), m_type(NodeType::Dynamic) {}
    
    ISceneNode* GetParent() const override { return nullptr; }
    void SetParent(ISceneNode*) override {}
    void GetChildren(std::vector<ISceneNode*>&) const override {}
    size_t GetChildCount() const override { return 0; }
    Transform const& GetLocalTransform() const override { return m_transform; }
    void SetLocalTransform(Transform const& t) override { m_transform = t; }
    Transform const& GetWorldTransform() const override { return m_transform; }
    te::core::Matrix4 const& GetWorldMatrix() const override { return m_matrix; }
    NodeId GetNodeId() const override { return NodeId(const_cast<void*>(static_cast<const void*>(this))); }
    char const* GetName() const override { return "MockNode"; }
    bool IsActive() const override { return m_active; }
    void SetActive(bool a) override { m_active = a; }
    NodeType GetNodeType() const override { return m_type; }
    bool IsDirty() const override { return m_dirty; }
    void SetDirty(bool d) override { m_dirty = d; }
    
    void SetType(NodeType type) { m_type = type; }
    
private:
    Transform m_transform;
    te::core::Matrix4 m_matrix;
    bool m_active;
    bool m_dirty;
    NodeType m_type;
};

void TestDynamicNodeManager() {
    DynamicNodeManager manager;
    
    SimpleMockNode node1, node2, node3;
    node1.SetType(NodeType::Dynamic);
    node2.SetType(NodeType::Dynamic);
    node3.SetType(NodeType::Dynamic);
    
    // Add nodes
    manager.AddNode(&node1);
    manager.AddNode(&node2);
    manager.AddNode(&node3);
    
    assert(manager.GetNodeCount() == 3);
    
    // Traverse
    int count = 0;
    manager.Traverse([&](ISceneNode* node) {
        count++;
    });
    assert(count == 3);
    
    // Remove node
    manager.RemoveNode(&node2);
    assert(manager.GetNodeCount() == 2);
    
    // Clear
    manager.Clear();
    assert(manager.GetNodeCount() == 0);
}

void TestStaticNodeManager() {
    te::core::AABB bounds;
    bounds.min = {0, 0, 0};
    bounds.max = {100, 100, 100};
    
    StaticNodeManager manager(SpatialIndexType::Octree, bounds);
    
    SimpleMockNode node1, node2;
    node1.SetType(NodeType::Static);
    node2.SetType(NodeType::Static);
    
    // Add nodes
    manager.AddNode(&node1);
    manager.AddNode(&node2);
    
    assert(manager.GetNodeCount() == 2);
    
    // Clear
    manager.Clear();
    assert(manager.GetNodeCount() == 0);
}

void RunTestNodeManagers() {
    TestDynamicNodeManager();
    TestStaticNodeManager();
}

}  // namespace scene
}  // namespace te
