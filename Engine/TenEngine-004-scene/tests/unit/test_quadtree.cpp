/**
 * @file test_quadtree.cpp
 * @brief Unit tests for Quadtree
 */

#include <te/scene/Quadtree.h>
#include <te/scene/ISceneNode.h>
#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <te/core/engine.h>
#include <cassert>
#include <functional>
#include <vector>
#include <string>

namespace te {
namespace scene {

// Simplified mock node with AABB
class MockNodeWithAABB : public ISceneNode {
public:
    MockNodeWithAABB(te::core::AABB const& aabb) : m_aabb(aabb), m_active(true), m_dirty(false) {}
    
    ISceneNode* GetParent() const override { return nullptr; }
    void SetParent(ISceneNode*) override {}
    void GetChildren(std::vector<ISceneNode*>&) const override {}
    size_t GetChildCount() const override { return 0; }
    Transform const& GetLocalTransform() const override { return m_transform; }
    void SetLocalTransform(Transform const&) override {}
    Transform const& GetWorldTransform() const override { return m_transform; }
    te::core::Matrix4 const& GetWorldMatrix() const override { return m_matrix; }
    NodeId GetNodeId() const override { return NodeId(const_cast<void*>(static_cast<const void*>(this))); }
    char const* GetName() const override { return "MockNode"; }
    bool IsActive() const override { return m_active; }
    void SetActive(bool) override {}
    NodeType GetNodeType() const override { return NodeType::Static; }
    bool HasAABB() const override { return true; }
    te::core::AABB GetAABB() const override { return m_aabb; }
    bool IsDirty() const override { return m_dirty; }
    void SetDirty(bool d) override { m_dirty = d; }
    
private:
    te::core::AABB m_aabb;
    Transform m_transform;
    te::core::Matrix4 m_matrix;
    bool m_active;
    bool m_dirty;
};

void TestQuadtreeInsert() {
    te::core::AABB bounds;
    bounds.min = {0, 0, 0};
    bounds.max = {100, 100, 100};
    
    Quadtree quadtree(bounds);
    
    te::core::AABB nodeAABB;
    nodeAABB.min = {10, 10, 0};
    nodeAABB.max = {20, 20, 0};
    
    MockNodeWithAABB node(nodeAABB);
    quadtree.Insert(&node);
    
    assert(quadtree.GetNodeCount() == 1);
}

void TestQuadtreeQuery() {
    te::core::AABB bounds;
    bounds.min = {0, 0, 0};
    bounds.max = {100, 100, 100};
    
    Quadtree quadtree(bounds);
    
    te::core::AABB nodeAABB;
    nodeAABB.min = {10, 10, 0};
    nodeAABB.max = {20, 20, 0};
    
    MockNodeWithAABB node(nodeAABB);
    quadtree.Insert(&node);
    
    // Query AABB (2D)
    te::core::AABB queryAABB;
    queryAABB.min = {5, 5, 0};
    queryAABB.max = {25, 25, 0};
    
    int count = 0;
    quadtree.QueryAABB(queryAABB, [&](ISceneNode* n) {
        count++;
    });
    
    assert(count == 1);
}

void RunTestQuadtree() {
    TestQuadtreeInsert();
    TestQuadtreeQuery();
}

}  // namespace scene
}  // namespace te
