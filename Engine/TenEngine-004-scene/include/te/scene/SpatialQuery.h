/**
 * @file SpatialQuery.h
 * @brief Spatial query algorithms: frustum culling, AABB queries, etc.
 * Contract: specs/_contracts/004-scene-public-api.md
 */

#ifndef TE_SCENE_SPATIAL_QUERY_H
#define TE_SCENE_SPATIAL_QUERY_H

#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <functional>
#include <vector>

namespace te {
namespace scene {

// Forward declaration
class ISceneNode;

/**
 * @brief Spatial query algorithms
 * 
 * Provides frustum culling, AABB queries, containment tests,
 * intersection tests, raycasting, and nearest point queries.
 */
class SpatialQuery {
public:
    /**
     * @brief Query nodes within frustum (frustum culling)
     * @param world World reference
     * @param frustum Frustum (6 planes)
     * @param callback Callback function for each node in frustum
     * 
     * Each node is tested independently - parent culling does not
     * affect child nodes.
     */
    static void QueryFrustum(
        WorldRef world,
        Frustum const& frustum,
        std::function<void(ISceneNode*)> const& callback);
    
    /**
     * @brief Query nodes intersecting AABB
     * @param world World reference
     * @param aabb Query AABB
     * @param callback Callback function for each intersecting node
     */
    static void QueryAABB(
        WorldRef world,
        te::core::AABB const& aabb,
        std::function<void(ISceneNode*)> const& callback);
    
    /**
     * @brief Query nodes completely contained in AABB
     * @param world World reference
     * @param aabb Query AABB
     * @param callback Callback function for each contained node
     */
    static void QueryContained(
        WorldRef world,
        te::core::AABB const& aabb,
        std::function<void(ISceneNode*)> const& callback);
    
    /**
     * @brief Query nodes intersecting AABB
     * @param world World reference
     * @param aabb Query AABB
     * @param callback Callback function for each intersecting node
     * 
     * Same as QueryAABB, but with explicit name for clarity.
     */
    static void QueryIntersecting(
        WorldRef world,
        te::core::AABB const& aabb,
        std::function<void(ISceneNode*)> const& callback);
    
    /**
     * @brief Raycast - find closest node hit by ray
     * @param world World reference
     * @param ray Ray (origin and direction)
     * @param outHitNode Output: hit node (nullptr if no hit)
     * @param outDistance Output: distance to hit point
     * @return true if ray hit a node
     */
    static bool Raycast(
        WorldRef world,
        te::core::Ray const& ray,
        ISceneNode*& outHitNode,
        float& outDistance);
    
    /**
     * @brief Find nearest node to a point
     * @param world World reference
     * @param point Query point
     * @param maxDistance Maximum distance to search (FLT_MAX for unlimited)
     * @return Nearest node, or nullptr if none found
     */
    static ISceneNode* FindNearest(
        WorldRef world,
        te::core::Vector3 const& point,
        float maxDistance = 3.402823466e+38f);  // FLT_MAX
    
public:
    // Helper functions (public for use by Octree/Quadtree)
    
    /**
     * @brief Test if AABB intersects frustum
     * @param frustum Frustum
     * @param aabb AABB
     * @return true if AABB intersects frustum
     */
    static bool FrustumIntersectsAABB(
        Frustum const& frustum,
        te::core::AABB const& aabb);
    
    /**
     * @brief Test if AABB is completely contained in another AABB
     * @param inner Inner AABB
     * @param outer Outer AABB
     * @return true if inner is completely contained in outer
     */
    static bool AABBContains(te::core::AABB const& inner,
                             te::core::AABB const& outer);
    
    /**
     * @brief Test if two AABBs intersect
     * @param a First AABB
     * @param b Second AABB
     * @return true if AABBs intersect
     */
    static bool AABBIntersects(te::core::AABB const& a,
                               te::core::AABB const& b);
    
private:
    /**
     * @brief Test if ray intersects AABB
     * @param ray Ray
     * @param aabb AABB
     * @param outDistance Output: distance to intersection point
     * @return true if ray intersects AABB
     */
    static bool RayIntersectsAABB(te::core::Ray const& ray,
                                  te::core::AABB const& aabb,
                                  float& outDistance);
    
    /**
     * @brief Calculate distance from point to AABB
     * @param point Point
     * @param aabb AABB
     * @return Distance (0 if point is inside AABB)
     */
    static float DistanceToAABB(te::core::Vector3 const& point,
                                te::core::AABB const& aabb);
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_SPATIAL_QUERY_H
