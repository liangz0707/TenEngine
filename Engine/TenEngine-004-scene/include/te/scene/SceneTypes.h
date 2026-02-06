/**
 * @file SceneTypes.h
 * @brief Scene module base types: NodeId, WorldRef, NodeType, Transform, etc.
 * Contract: specs/_contracts/004-scene-public-api.md
 */

#ifndef TE_SCENE_SCENE_TYPES_H
#define TE_SCENE_SCENE_TYPES_H

#include <te/core/math.h>
#include <cstdint>

namespace te {
namespace scene {

// Forward declaration
class ISceneNode;

/**
 * @brief Scene node ID (opaque handle)
 */
struct NodeId {
    void* value = nullptr;
    
    NodeId() = default;
    explicit NodeId(void* v) : value(v) {}
    
    bool IsValid() const { return value != nullptr; }
    bool operator==(NodeId const& other) const { return value == other.value; }
    bool operator!=(NodeId const& other) const { return value != other.value; }
};

/**
 * @brief Scene world reference (opaque handle)
 */
struct WorldRef {
    void* value = nullptr;
    
    WorldRef() = default;
    explicit WorldRef(void* v) : value(v) {}
    
    bool IsValid() const { return value != nullptr; }
    bool operator==(WorldRef const& other) const { return value == other.value; }
    bool operator!=(WorldRef const& other) const { return value != other.value; }
};

/**
 * @brief Scene reference (alias for WorldRef)
 */
using SceneRef = WorldRef;

/**
 * @brief Node type: Static uses spatial index, Dynamic uses linear list
 */
enum class NodeType {
    Static,   // Static node, stored in spatial index (octree/quadtree)
    Dynamic   // Dynamic node, stored in linear list
};

/**
 * @brief Spatial index type
 */
enum class SpatialIndexType {
    None,      // No spatial index
    Octree,    // Octree for 3D scenes
    Quadtree   // Quadtree for 2D scenes
};

/**
 * @brief Transform (position, rotation, scale)
 * Uses Core math types
 */
struct Transform {
    te::core::Vector3 position{0, 0, 0};
    te::core::Quaternion rotation{0, 0, 0, 1};  // x, y, z, w
    te::core::Vector3 scale{1, 1, 1};
    
    Transform() = default;
    Transform(te::core::Vector3 const& pos, 
              te::core::Quaternion const& rot, 
              te::core::Vector3 const& scl)
        : position(pos), rotation(rot), scale(scl) {}
};

/**
 * @brief Frustum (6 planes for frustum culling)
 * Each plane is represented as: ax + by + cz + d = 0
 */
struct Frustum {
    float planes[6][4];  // [left, right, bottom, top, near, far][a, b, c, d]
    
    Frustum() {
        // Initialize to zero
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 4; ++j) {
                planes[i][j] = 0.0f;
            }
        }
    }
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_SCENE_TYPES_H
