/**
 * @file Entity.cpp
 * @brief Entity implementation
 */

#include <te/entity/Entity.h>
#include <te/entity/EntityManager.h>
#include <te/scene/SceneManager.h>
#include <te/scene/ISceneNode.h>
#include <te/core/math.h>
#include <algorithm>
#include <cstring>
#include <cmath>

namespace te {
namespace entity {

// Forward declaration
extern EntityManager* GetEntityManager();

Entity::Entity(te::scene::WorldRef world, char const* name)
    : m_entityId(EntityId(this))
    , m_name(name ? name : "")
    , m_world(world)
    , m_parent(nullptr)
    , m_children()
    , m_localTransform()
    , m_worldTransform()
    , m_worldMatrix()
    , m_worldMatrixDirty(true)
    , m_active(true)
    , m_dirty(true)
    , m_nodeType(te::scene::NodeType::Dynamic)
    , m_hasAABB(false)
    , m_aabb()
{
    // Initialize world matrix to identity
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m_worldMatrix[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

Entity::~Entity() {
    // Clean up components
    for (auto& pair : m_components) {
        if (pair.second) {
            pair.second->OnDetached(this);
        }
    }
    m_components.clear();
    
    // Remove from parent's children list
    if (m_parent) {
        Entity* parentEntity = static_cast<Entity*>(m_parent);
        auto it = std::find(parentEntity->m_children.begin(), 
                           parentEntity->m_children.end(), this);
        if (it != parentEntity->m_children.end()) {
            parentEntity->m_children.erase(it);
        }
    }
    
    // Clear children's parent references
    for (Entity* child : m_children) {
        child->m_parent = nullptr;
    }
    m_children.clear();
}

Entity* Entity::Create(te::scene::WorldRef world, char const* name) {
    Entity* entity = new Entity(world, name);
    
    // Register with SceneManager
    te::scene::SceneManager::GetInstance().RegisterNode(entity);
    
    return entity;
}

Entity* Entity::CreateFromNode(te::scene::NodeId nodeId, te::scene::WorldRef world) {
    // Find the node
    te::scene::ISceneNode* node = te::scene::SceneManager::GetInstance().FindNodeById(world, nodeId);
    if (!node) {
        return nullptr;
    }
    
    // Create Entity from node
    Entity* entity = new Entity(world, node->GetName());
    
    // Copy transform from node
    entity->SetLocalTransform(node->GetLocalTransform());
    
    // Copy active state
    entity->SetActive(node->IsActive());
    
    // Copy node type
    entity->m_nodeType = node->GetNodeType();
    
    // Register with SceneManager
    te::scene::SceneManager::GetInstance().RegisterNode(entity);
    
    // Note: Resource-related operations (like ModelComponent) should be handled by World module
    // Entity module does not process resource content
    
    return entity;
}

void Entity::Destroy() {
    // Unregister from SceneManager
    te::scene::SceneManager::GetInstance().UnregisterNode(this);
    
    // Destroy will be handled by EntityManager
    // This is just a convenience method
    EntityManager* mgr = GetEntityManager();
    if (mgr) {
        mgr->DestroyEntity(this);
    } else {
        delete this;
    }
}

void Entity::SetEnabled(bool enabled) {
    SetActive(enabled);
}

bool Entity::IsEnabled() const {
    return IsActive();
}

// ========== ISceneNode Implementation ==========

te::scene::ISceneNode* Entity::GetParent() const {
    return m_parent;
}

void Entity::SetParent(te::scene::ISceneNode* parent) {
    // Remove from old parent
    if (m_parent) {
        Entity* oldParent = static_cast<Entity*>(m_parent);
        auto it = std::find(oldParent->m_children.begin(), 
                           oldParent->m_children.end(), this);
        if (it != oldParent->m_children.end()) {
            oldParent->m_children.erase(it);
        }
    }
    
    // Set new parent
    m_parent = parent;
    
    // Add to new parent's children
    if (parent) {
        Entity* parentEntity = static_cast<Entity*>(parent);
        parentEntity->m_children.push_back(this);
    }
    
    // Mark dirty
    SetDirty(true);
}

void Entity::GetChildren(std::vector<te::scene::ISceneNode*>& out) const {
    out.clear();
    out.reserve(m_children.size());
    for (Entity* child : m_children) {
        out.push_back(child);
    }
}

size_t Entity::GetChildCount() const {
    return m_children.size();
}

te::scene::Transform const& Entity::GetLocalTransform() const {
    return m_localTransform;
}

void Entity::SetLocalTransform(te::scene::Transform const& t) {
    m_localTransform = t;
    SetDirty(true);
}

te::scene::Transform const& Entity::GetWorldTransform() const {
    return m_worldTransform;
}

te::core::Matrix4 const& Entity::GetWorldMatrix() const {
    if (m_worldMatrixDirty) {
        // Calculate world matrix from transform
        te::core::Vector3 pos = m_worldTransform.position;
        te::core::Quaternion rot = m_worldTransform.rotation;
        te::core::Vector3 scale = m_worldTransform.scale;
        
        // Convert quaternion to rotation matrix
        // Quaternion: (x, y, z, w)
        float x = rot.x, y = rot.y, z = rot.z, w = rot.w;
        
        // Normalize quaternion
        float len = x*x + y*y + z*z + w*w;
        if (len > 0.0f) {
            float invLen = 1.0f / std::sqrt(len);
            x *= invLen;
            y *= invLen;
            z *= invLen;
            w *= invLen;
        }
        
        // Build rotation matrix from quaternion
        // R = [1-2(y²+z²)  2(xy-wz)    2(xz+wy)   ]
        //     [2(xy+wz)    1-2(x²+z²)  2(yz-wx)   ]
        //     [2(xz-wy)    2(yz+wx)    1-2(x²+y²) ]
        float xx = x * x;
        float yy = y * y;
        float zz = z * z;
        float xy = x * y;
        float xz = x * z;
        float yz = y * z;
        float wx = w * x;
        float wy = w * y;
        float wz = w * z;
        
        // Initialize to identity
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m_worldMatrix[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
        
        // Set rotation part (3x3)
        m_worldMatrix[0][0] = 1.0f - 2.0f * (yy + zz);
        m_worldMatrix[0][1] = 2.0f * (xy - wz);
        m_worldMatrix[0][2] = 2.0f * (xz + wy);
        
        m_worldMatrix[1][0] = 2.0f * (xy + wz);
        m_worldMatrix[1][1] = 1.0f - 2.0f * (xx + zz);
        m_worldMatrix[1][2] = 2.0f * (yz - wx);
        
        m_worldMatrix[2][0] = 2.0f * (xz - wy);
        m_worldMatrix[2][1] = 2.0f * (yz + wx);
        m_worldMatrix[2][2] = 1.0f - 2.0f * (xx + yy);
        
        // Apply scale
        m_worldMatrix[0][0] *= scale.x;
        m_worldMatrix[0][1] *= scale.x;
        m_worldMatrix[0][2] *= scale.x;
        
        m_worldMatrix[1][0] *= scale.y;
        m_worldMatrix[1][1] *= scale.y;
        m_worldMatrix[1][2] *= scale.y;
        
        m_worldMatrix[2][0] *= scale.z;
        m_worldMatrix[2][1] *= scale.z;
        m_worldMatrix[2][2] *= scale.z;
        
        // Set translation part
        m_worldMatrix[0][3] = pos.x;
        m_worldMatrix[1][3] = pos.y;
        m_worldMatrix[2][3] = pos.z;
        
        m_worldMatrixDirty = false;
    }
    return m_worldMatrix;
}

te::scene::NodeId Entity::GetNodeId() const {
    // const_cast is safe here because NodeId is just an opaque handle
    // and we're not modifying the Entity through it
    return te::scene::NodeId(const_cast<Entity*>(this));
}

char const* Entity::GetName() const {
    return m_name.c_str();
}

bool Entity::IsActive() const {
    if (!m_active) {
        return false;
    }
    // Check parent chain
    if (m_parent) {
        return m_parent->IsActive();
    }
    return true;
}

void Entity::SetActive(bool active) {
    m_active = active;
}

te::scene::NodeType Entity::GetNodeType() const {
    return m_nodeType;
}

bool Entity::HasAABB() const {
    return m_hasAABB;
}

te::core::AABB Entity::GetAABB() const {
    return m_aabb;
}

bool Entity::IsDirty() const {
    return m_dirty;
}

void Entity::SetDirty(bool dirty) {
    m_dirty = dirty;
    if (dirty) {
        m_worldMatrixDirty = true;
    }
}

// ========== Internal Component Management ==========

Component* Entity::AddComponentInternal(te::object::TypeId typeId) {
    auto it = m_components.find(typeId);
    if (it != m_components.end()) {
        return it->second.get();
    }
    
    // Create component instance using TypeRegistry
    void* instance = te::object::TypeRegistry::CreateInstance(typeId);
    if (!instance) {
        return nullptr;
    }
    
    Component* comp = static_cast<Component*>(instance);
    m_components[typeId] = std::unique_ptr<Component>(comp);
    
    return comp;
}

Component* Entity::GetComponentInternal(te::object::TypeId typeId) {
    auto it = m_components.find(typeId);
    if (it != m_components.end()) {
        return it->second.get();
    }
    return nullptr;
}

Component const* Entity::GetComponentInternal(te::object::TypeId typeId) const {
    auto it = m_components.find(typeId);
    if (it != m_components.end()) {
        return it->second.get();
    }
    return nullptr;
}

void Entity::RemoveComponentInternal(te::object::TypeId typeId) {
    auto it = m_components.find(typeId);
    if (it != m_components.end()) {
        m_components.erase(it);
    }
}

bool Entity::HasComponentInternal(te::object::TypeId typeId) const {
    return m_components.find(typeId) != m_components.end();
}

}  // namespace entity
}  // namespace te
