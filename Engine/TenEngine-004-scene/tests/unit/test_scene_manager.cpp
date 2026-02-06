/**
 * @file test_scene_manager.cpp
 * @brief Unit tests for SceneManager
 */

#include <te/scene/SceneManager.h>
#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <te/core/engine.h>
#include <cassert>
#include <string>

namespace te {
namespace scene {

// Mock node for testing
class MockNode : public ISceneNode {
public:
    MockNode(NodeId id, NodeType type = NodeType::Dynamic)
        : m_nodeId(id)
        , m_nodeType(type)
        , m_active(true)
        , m_dirty(false)
    {}
    
    ISceneNode* GetParent() const override { return m_parent; }
    void SetParent(ISceneNode* parent) override { 
        m_parent = parent;
        SetDirty(true);
    }
    void GetChildren(std::vector<ISceneNode*>& out) const override {
        out = m_children;
    }
    size_t GetChildCount() const override { return m_children.size(); }
    Transform const& GetLocalTransform() const override { return m_localTransform; }
    void SetLocalTransform(Transform const& t) override {
        m_localTransform = t;
        SetDirty(true);
    }
    Transform const& GetWorldTransform() const override { return m_worldTransform; }
    te::core::Matrix4 const& GetWorldMatrix() const override { return m_worldMatrix; }
    NodeId GetNodeId() const override { return m_nodeId; }
    char const* GetName() const override { return m_name.c_str(); }
    bool IsActive() const override {
        if (m_parent && !m_parent->IsActive()) return false;
        return m_active;
    }
    void SetActive(bool active) override { m_active = active; }
    NodeType GetNodeType() const override { return m_nodeType; }
    bool IsDirty() const override { return m_dirty; }
    void SetDirty(bool dirty) override { m_dirty = dirty; }
    
    void SetName(char const* name) { m_name = name; }
    
private:
    NodeId m_nodeId;
    NodeType m_nodeType;
    ISceneNode* m_parent = nullptr;
    std::vector<ISceneNode*> m_children;
    Transform m_localTransform;
    Transform m_worldTransform;
    te::core::Matrix4 m_worldMatrix;
    bool m_active;
    bool m_dirty;
    std::string m_name;
};

void TestSceneManagerBasic() {
    SceneManager& manager = SceneManager::GetInstance();
    
    // Create world
    te::core::AABB bounds;
    bounds.min = {0, 0, 0};
    bounds.max = {100, 100, 100};
    
    WorldRef world = manager.CreateWorld(SpatialIndexType::Octree, bounds);
    assert(world.IsValid());
    
    // Get active world
    WorldRef activeWorld = manager.GetActiveWorld();
    assert(activeWorld == world);
    
    // Set active world
    manager.SetActiveWorld(world);
    assert(manager.GetActiveWorld() == world);
    
    // Destroy world
    manager.DestroyWorld(world);
    assert(!manager.GetActiveWorld().IsValid());
}

void TestNodeRegistration() {
    SceneManager& manager = SceneManager::GetInstance();
    
    te::core::AABB bounds;
    bounds.min = {0, 0, 0};
    bounds.max = {100, 100, 100};
    WorldRef world = manager.CreateWorld(SpatialIndexType::None, bounds);
    
    // Create mock nodes
    MockNode node1(NodeId((void*)1), NodeType::Dynamic);
    MockNode node2(NodeId((void*)2), NodeType::Static);
    
    // Register nodes
    manager.RegisterNode(&node1);
    manager.RegisterNode(&node2);
    
    // Find nodes
    ISceneNode* found1 = manager.FindNodeById(world, node1.GetNodeId());
    assert(found1 == &node1);
    
    // Unregister
    manager.UnregisterNode(&node1);
    manager.UnregisterNode(&node2);
    
    manager.DestroyWorld(world);
}

void RunTestSceneManager() {
    TestSceneManagerBasic();
    TestNodeRegistration();
}

}  // namespace scene
}  // namespace te
