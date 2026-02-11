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

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_WORLD_TYPES_H
