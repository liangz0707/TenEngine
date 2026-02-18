/**
 * @file WorldTypes.h
 * @brief 029-World types: LevelHandle, RenderableItem, etc.
 * Contract: specs/_contracts/029-world-public-api.md
 */

#ifndef TE_WORLD_WORLD_TYPES_H
#define TE_WORLD_WORLD_TYPES_H

#include <te/scene/SceneTypes.h>
#include <te/core/math.h>

namespace te {
namespace rendercore {
struct IRenderElement;
}
namespace world {

/** Opaque level handle; 029 holds and binds to SceneRef. */
struct LevelHandle {
    void* value = nullptr;
    LevelHandle() = default;
    explicit LevelHandle(void* v) : value(v) {}
    bool IsValid() const { return value != nullptr; }
    bool operator==(LevelHandle const& o) const { return value == o.value; }
    bool operator!=(LevelHandle const& o) const { return value != o.value; }
};

/**
 * @brief Renderable item for pipeline collection.
 * Contains world matrix, render element, and submesh index.
 * Used by CollectRenderables callback.
 */
struct RenderableItem {
    /// World transformation matrix (column-major 4x4)
    float worldMatrix[16];

    /// Render element (contains mesh + material)
    te::rendercore::IRenderElement* element{nullptr};

    /// Submesh index for multi-submesh models
    uint32_t submeshIndex{0};

    /// Model resource ID for LOD selection
    uint64_t modelResourceId{0};

    /// Bounds for culling (AABB in world space)
    float boundsMin[3]{0.f, 0.f, 0.f};
    float boundsMax[3]{0.f, 0.f, 0.f};

    /// User data (e.g., Entity pointer)
    void* userData{nullptr};
};

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_WORLD_TYPES_H
