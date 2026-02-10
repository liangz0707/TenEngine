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

/** One renderable item for Pipeline: world matrix + model/mesh/material refs. */
struct RenderableItem {
    te::core::Matrix4 worldMatrix;
    void* modelResource = nullptr;  // IModelResource*; 029 owns type
    uint32_t submeshIndex = 0;
    RenderableItem() = default;
};

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_WORLD_TYPES_H
