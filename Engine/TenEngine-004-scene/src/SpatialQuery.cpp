/**
 * @file SpatialQuery.cpp
 * @brief Spatial query algorithms implementation
 */

#include <te/scene/SpatialQuery.h>
#include <te/scene/SceneManager.h>
#include <te/scene/SceneWorld.h>
#include <te/scene/ISceneNode.h>
#include <te/core/math.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace te {
namespace scene {

void SpatialQuery::QueryFrustum(
    WorldRef world,
    Frustum const& frustum,
    std::function<void(ISceneNode*)> const& callback) {
    
    SceneManager& manager = SceneManager::GetInstance();
    
    // Traverse all nodes in the world
    manager.Traverse(world, [&](ISceneNode* node) {
        if (!node->IsActive() || !node->HasAABB()) {
            return;
        }
        
        te::core::AABB aabb = node->GetAABB();
        if (FrustumIntersectsAABB(frustum, aabb)) {
            callback(node);
        }
    });
}

void SpatialQuery::QueryAABB(
    WorldRef world,
    te::core::AABB const& aabb,
    std::function<void(ISceneNode*)> const& callback) {
    
    QueryIntersecting(world, aabb, callback);
}

void SpatialQuery::QueryContained(
    WorldRef world,
    te::core::AABB const& aabb,
    std::function<void(ISceneNode*)> const& callback) {
    
    SceneManager& manager = SceneManager::GetInstance();
    
    manager.Traverse(world, [&](ISceneNode* node) {
        if (!node->IsActive() || !node->HasAABB()) {
            return;
        }
        
        te::core::AABB nodeAABB = node->GetAABB();
        if (AABBContains(nodeAABB, aabb)) {
            callback(node);
        }
    });
}

void SpatialQuery::QueryIntersecting(
    WorldRef world,
    te::core::AABB const& aabb,
    std::function<void(ISceneNode*)> const& callback) {
    
    QueryAABB(world, aabb, callback);
}

bool SpatialQuery::Raycast(
    WorldRef world,
    te::core::Ray const& ray,
    ISceneNode*& outHitNode,
    float& outDistance) {
    
    SceneManager& manager = SceneManager::GetInstance();
    
    ISceneNode* closestNode = nullptr;
    float closestDistance = std::numeric_limits<float>::max();
    
    manager.Traverse(world, [&](ISceneNode* node) {
        if (!node->IsActive() || !node->HasAABB()) {
            return;
        }
        
        te::core::AABB aabb = node->GetAABB();
        float distance;
        if (RayIntersectsAABB(ray, aabb, distance)) {
            if (distance < closestDistance) {
                closestDistance = distance;
                closestNode = node;
            }
        }
    });
    
    if (closestNode) {
        outHitNode = closestNode;
        outDistance = closestDistance;
        return true;
    }
    
    outHitNode = nullptr;
    outDistance = 0.0f;
    return false;
}

ISceneNode* SpatialQuery::FindNearest(
    WorldRef world,
    te::core::Vector3 const& point,
    float maxDistance) {
    
    SceneManager& manager = SceneManager::GetInstance();
    
    ISceneNode* nearestNode = nullptr;
    float nearestDistance = maxDistance;
    
    manager.Traverse(world, [&](ISceneNode* node) {
        if (!node->IsActive() || !node->HasAABB()) {
            return;
        }
        
        te::core::AABB aabb = node->GetAABB();
        float distance = DistanceToAABB(point, aabb);
        if (distance < nearestDistance) {
            nearestDistance = distance;
            nearestNode = node;
        }
    });
    
    return nearestNode;
}

// Helper function implementations

bool SpatialQuery::FrustumIntersectsAABB(
    Frustum const& frustum,
    te::core::AABB const& aabb) {
    
    // Test AABB against each frustum plane
    for (int i = 0; i < 6; ++i) {
        float const* plane = frustum.planes[i];
        float a = plane[0];
        float b = plane[1];
        float c = plane[2];
        float d = plane[3];
        
        // Find the AABB vertex that is farthest in the positive direction of the plane normal
        te::core::Vector3 positiveVertex;
        positiveVertex.x = (a > 0) ? aabb.max.x : aabb.min.x;
        positiveVertex.y = (b > 0) ? aabb.max.y : aabb.min.y;
        positiveVertex.z = (c > 0) ? aabb.max.z : aabb.min.z;
        
        // Calculate distance from positive vertex to plane
        float distance = a * positiveVertex.x + b * positiveVertex.y + c * positiveVertex.z + d;
        
        // If positive vertex is behind the plane, AABB is outside frustum
        if (distance < 0) {
            return false;
        }
    }
    
    return true;
}

bool SpatialQuery::AABBContains(
    te::core::AABB const& inner,
    te::core::AABB const& outer) {
    
    return (inner.min.x >= outer.min.x && inner.min.y >= outer.min.y && inner.min.z >= outer.min.z &&
            inner.max.x <= outer.max.x && inner.max.y <= outer.max.y && inner.max.z <= outer.max.z);
}

bool SpatialQuery::AABBIntersects(
    te::core::AABB const& a,
    te::core::AABB const& b) {
    
    // 3D intersection check
    return !(a.max.x < b.min.x || a.min.x > b.max.x ||
             a.max.y < b.min.y || a.min.y > b.max.y ||
             a.max.z < b.min.z || a.min.z > b.max.z);
}

bool SpatialQuery::RayIntersectsAABB(
    te::core::Ray const& ray,
    te::core::AABB const& aabb,
    float& outDistance) {
    
    // Slab method for ray-AABB intersection
    float tmin = 0.0f;
    float tmax = std::numeric_limits<float>::max();
    
    for (int i = 0; i < 3; ++i) {
        float invD = 1.0f / ray.direction[i];
        float t0 = (aabb.min[i] - ray.origin[i]) * invD;
        float t1 = (aabb.max[i] - ray.origin[i]) * invD;
        
        if (invD < 0.0f) {
            std::swap(t0, t1);
        }
        
        tmin = std::max(tmin, t0);
        tmax = std::min(tmax, t1);
        
        if (tmax < tmin) {
            return false;
        }
    }
    
    outDistance = tmin;
    return true;
}

float SpatialQuery::DistanceToAABB(
    te::core::Vector3 const& point,
    te::core::AABB const& aabb) {
    
    // Clamp point to AABB
    te::core::Vector3 clamped;
    clamped.x = std::max(aabb.min.x, std::min(point.x, aabb.max.x));
    clamped.y = std::max(aabb.min.y, std::min(point.y, aabb.max.y));
    clamped.z = std::max(aabb.min.z, std::min(point.z, aabb.max.z));
    
    // Calculate distance from point to clamped point
    te::core::Vector3 diff;
    diff.x = point.x - clamped.x;
    diff.y = point.y - clamped.y;
    diff.z = point.z - clamped.z;
    
    return std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
}

}  // namespace scene
}  // namespace te
