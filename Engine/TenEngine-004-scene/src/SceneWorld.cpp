/**
 * @file SceneWorld.cpp
 * @brief SceneWorld implementation
 */

#include <te/scene/SceneWorld.h>
#include <te/scene/SceneManager.h>
#include <te/scene/DynamicNodeManager.h>
#include <te/scene/StaticNodeManager.h>
#include <algorithm>
#include <cstring>
#include <cmath>

namespace te {
namespace scene {

// Helper functions for transform math

/**
 * @brief Convert quaternion to rotation matrix (3x3)
 */
static void QuaternionToMatrix3(te::core::Quaternion const& q, te::core::Matrix3& out) {
    float x = q.x, y = q.y, z = q.z, w = q.w;
    
    // Normalize quaternion
    float len = std::sqrt(x*x + y*y + z*z + w*w);
    if (len > 0.0f) {
        x /= len; y /= len; z /= len; w /= len;
    }
    
    // Convert to rotation matrix
    out[0][0] = 1.0f - 2.0f * (y*y + z*z);
    out[0][1] = 2.0f * (x*y - z*w);
    out[0][2] = 2.0f * (x*z + y*w);
    
    out[1][0] = 2.0f * (x*y + z*w);
    out[1][1] = 1.0f - 2.0f * (x*x + z*z);
    out[1][2] = 2.0f * (y*z - x*w);
    
    out[2][0] = 2.0f * (x*z - y*w);
    out[2][1] = 2.0f * (y*z + x*w);
    out[2][2] = 1.0f - 2.0f * (x*x + y*y);
}

/**
 * @brief Convert Transform to Matrix4
 */
static te::core::Matrix4 TransformToMatrix4(Transform const& t) {
    te::core::Matrix4 result;
    
    // Initialize to identity
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
    
    // Convert rotation quaternion to 3x3 matrix
    te::core::Matrix3 rotMat;
    QuaternionToMatrix3(t.rotation, rotMat);
    
    // Apply scale
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result[i][j] = rotMat[i][j];
            if (i == 0) result[i][j] *= t.scale.x;
            else if (i == 1) result[i][j] *= t.scale.y;
            else if (i == 2) result[i][j] *= t.scale.z;
        }
    }
    
    // Apply translation
    result[0][3] = t.position.x;
    result[1][3] = t.position.y;
    result[2][3] = t.position.z;
    
    return result;
}

/**
 * @brief Multiply two Matrix4 matrices: result = a * b
 */
static te::core::Matrix4 MultiplyMatrix4(te::core::Matrix4 const& a, te::core::Matrix4 const& b) {
    te::core::Matrix4 result;
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    
    return result;
}

/**
 * @brief Compose two transforms: result = parent * local
 * Note: This is a simplified implementation. For full transform composition,
 * use matrix multiplication and extract transform from the result matrix.
 */
static Transform ComposeTransforms(Transform const& parent, Transform const& local) {
    // Simplified composition: combine position, rotation, and scale
    // For accurate composition, convert to matrices, multiply, then extract transform
    Transform result;
    
    // Compose position: parent.position + parent.rotation * (parent.scale * local.position)
    // Use matrix multiplication for accurate position composition
    te::core::Matrix4 parentMat = TransformToMatrix4(parent);
    te::core::Vector4 localPos4{local.position.x, local.position.y, local.position.z, 1.0f};
    te::core::Vector4 worldPos4;
    worldPos4.x = parentMat[0][0]*localPos4.x + parentMat[0][1]*localPos4.y + parentMat[0][2]*localPos4.z + parentMat[0][3]*localPos4.w;
    worldPos4.y = parentMat[1][0]*localPos4.x + parentMat[1][1]*localPos4.y + parentMat[1][2]*localPos4.z + parentMat[1][3]*localPos4.w;
    worldPos4.z = parentMat[2][0]*localPos4.x + parentMat[2][1]*localPos4.y + parentMat[2][2]*localPos4.z + parentMat[2][3]*localPos4.w;
    result.position.x = worldPos4.x;
    result.position.y = worldPos4.y;
    result.position.z = worldPos4.z;
    
    // Compose rotation: parent.rotation * local.rotation
    // Simplified quaternion multiplication
    te::core::Quaternion const& p = parent.rotation;
    te::core::Quaternion const& l = local.rotation;
    result.rotation.w = p.w * l.w - p.x * l.x - p.y * l.y - p.z * l.z;
    result.rotation.x = p.w * l.x + p.x * l.w + p.y * l.z - p.z * l.y;
    result.rotation.y = p.w * l.y - p.x * l.z + p.y * l.w + p.z * l.x;
    result.rotation.z = p.w * l.z + p.x * l.y - p.y * l.x + p.z * l.w;
    
    // Normalize quaternion
    float len = std::sqrt(result.rotation.w*result.rotation.w + 
                         result.rotation.x*result.rotation.x + 
                         result.rotation.y*result.rotation.y + 
                         result.rotation.z*result.rotation.z);
    if (len > 0.0f) {
        result.rotation.w /= len;
        result.rotation.x /= len;
        result.rotation.y /= len;
        result.rotation.z /= len;
    }
    
    // Compose scale: parent.scale * local.scale
    result.scale.x = parent.scale.x * local.scale.x;
    result.scale.y = parent.scale.y * local.scale.y;
    result.scale.z = parent.scale.z * local.scale.z;
    
    return result;
}

SceneWorld::SceneWorld(SpatialIndexType indexType, te::core::AABB const& bounds)
    : m_indexType(indexType)
    , m_bounds(bounds)
    , m_worldRef(WorldRef(this))  // Use this pointer as unique identifier
{
    // Create node managers
    m_dynamicManager = std::make_unique<DynamicNodeManager>();
    
    if (indexType != SpatialIndexType::None) {
        m_staticManager = std::make_unique<StaticNodeManager>(indexType, bounds);
    }
}

SceneWorld::~SceneWorld() {
    // Clear all nodes (but don't delete them - we don't own them)
    m_allNodes.clear();
    m_rootNodesCache.clear();
    m_dynamicManager.reset();
    m_staticManager.reset();
}

void SceneWorld::RegisterNode(ISceneNode* node) {
    if (!node) {
        return;
    }
    
    // Check if already registered
    auto it = std::find(m_allNodes.begin(), m_allNodes.end(), node);
    if (it != m_allNodes.end()) {
        return;  // Already registered
    }
    
    m_allNodes.push_back(node);
    
    // Add to appropriate manager based on node type
    NodeType nodeType = node->GetNodeType();
    if (nodeType == NodeType::Dynamic) {
        if (m_dynamicManager) {
            m_dynamicManager->AddNode(node);
        }
    } else if (nodeType == NodeType::Static) {
        if (m_staticManager) {
            m_staticManager->AddNode(node);
        }
    }
    
    InvalidateRootNodesCache();
}

void SceneWorld::UnregisterNode(ISceneNode* node) {
    if (!node) {
        return;
    }
    
    auto it = std::find(m_allNodes.begin(), m_allNodes.end(), node);
    if (it == m_allNodes.end()) {
        return;  // Not registered
    }
    
    m_allNodes.erase(it);
    
    // Remove from appropriate manager
    NodeType nodeType = node->GetNodeType();
    if (nodeType == NodeType::Dynamic) {
        if (m_dynamicManager) {
            m_dynamicManager->RemoveNode(node);
        }
    } else if (nodeType == NodeType::Static) {
        if (m_staticManager) {
            m_staticManager->RemoveNode(node);
        }
    }
    
    InvalidateRootNodesCache();
}

void SceneWorld::UpdateTransforms() {
    // Collect all dirty nodes
    std::vector<ISceneNode*> dirtyNodes;
    for (ISceneNode* node : m_allNodes) {
        if (node->IsDirty()) {
            dirtyNodes.push_back(node);
        }
    }
    
    if (dirtyNodes.empty()) {
        return;
    }
    
    // Iterative transform update (avoid recursion)
    // Process nodes in topological order (parents before children)
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (ISceneNode* node : dirtyNodes) {
            if (!node->IsDirty()) {
                continue;  // Already processed
            }
            
            ISceneNode* parent = node->GetParent();
            
            // If parent is dirty, skip this node for now (process parent first)
            if (parent && parent->IsDirty()) {
                continue;
            }
            
            // Calculate world transform
            Transform worldTransform;
            te::core::Matrix4 worldMatrix;
            
            if (parent) {
                // World = ParentWorld * Local
                Transform parentWorldTransform = parent->GetWorldTransform();
                Transform localTransform = node->GetLocalTransform();
                
                // Compose transforms
                worldTransform = ComposeTransforms(parentWorldTransform, localTransform);
                
                // Calculate world matrix
                te::core::Matrix4 parentWorldMatrix = parent->GetWorldMatrix();
                te::core::Matrix4 localMatrix = TransformToMatrix4(localTransform);
                worldMatrix = MultiplyMatrix4(parentWorldMatrix, localMatrix);
            } else {
                // Root node: world = local
                worldTransform = node->GetLocalTransform();
                worldMatrix = TransformToMatrix4(worldTransform);
            }
            
            // Update node's world transform (node implementation should handle this)
            // Note: ISceneNode interface doesn't provide SetWorldTransform/SetWorldMatrix
            // The node implementation is responsible for storing and updating world transform
            // We just clear the dirty flag here
            node->SetDirty(false);
            changed = true;
        }
    }
}

void SceneWorld::GetRootNodes(std::vector<ISceneNode*>& out) const {
    if (!m_rootNodesCacheValid) {
        UpdateRootNodesCache();
    }
    
    out = m_rootNodesCache;
}

void SceneWorld::UpdateRootNodesCache() const {
    m_rootNodesCache.clear();
    
    for (ISceneNode* node : m_allNodes) {
        if (node->GetParent() == nullptr) {
            m_rootNodesCache.push_back(node);
        }
    }
    
    m_rootNodesCacheValid = true;
}

void SceneWorld::Traverse(std::function<void(ISceneNode*)> const& callback) const {
    // Get root nodes
    std::vector<ISceneNode*> rootNodes;
    GetRootNodes(rootNodes);
    
    // Recursive traversal helper
    std::function<void(ISceneNode*)> traverseNode = [&](ISceneNode* node) {
        if (!node || !node->IsActive()) {
            return;
        }
        
        callback(node);
        
        std::vector<ISceneNode*> children;
        node->GetChildren(children);
        for (ISceneNode* child : children) {
            traverseNode(child);
        }
    };
    
    // Traverse from each root
    for (ISceneNode* root : rootNodes) {
        traverseNode(root);
    }
}

ISceneNode* SceneWorld::FindNodeByName(char const* name) const {
    if (!name) {
        return nullptr;
    }
    
    // Traverse and find first matching node
    ISceneNode* found = nullptr;
    Traverse([&](ISceneNode* node) {
        if (!found && node->GetName() && std::strcmp(node->GetName(), name) == 0) {
            found = node;
        }
    });
    
    return found;
}

ISceneNode* SceneWorld::FindNodeById(NodeId id) const {
    if (!id.IsValid()) {
        return nullptr;
    }
    
    for (ISceneNode* node : m_allNodes) {
        if (node->GetNodeId() == id) {
            return node;
        }
    }
    
    return nullptr;
}

}  // namespace scene
}  // namespace te
