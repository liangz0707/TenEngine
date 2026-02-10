/**
 * @file SceneDesc.h
 * @brief Scene description for CreateSceneFromDesc - 004 only, Core types, no Object/Resource.
 * Contract: specs/_contracts/004-scene-public-api.md
 */

#ifndef TE_SCENE_SCENE_DESC_H
#define TE_SCENE_SCENE_DESC_H

#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <vector>

namespace te {
namespace scene {

/**
 * @brief Single node description for scene graph construction.
 * 004 does not interpret opaqueUserData; caller (e.g. 029) may use it to pass per-node data.
 */
struct SceneNodeDesc {
    /** Node name (null-terminated; may be nullptr) */
    char const* name = nullptr;
    /** Local transform */
    Transform localTransform;
    /** Child node descriptions (recursive) */
    std::vector<SceneNodeDesc> children;
    /** Opaque pointer for caller (e.g. 029) to bind model/entity data; 004 does not use it */
    void* opaqueUserData = nullptr;
};

/**
 * @brief Scene description: root nodes only. Used by CreateSceneFromDesc.
 */
struct SceneDesc {
    /** Root node descriptions */
    std::vector<SceneNodeDesc> roots;
};

}  // namespace scene
}  // namespace te

#endif  // TE_SCENE_SCENE_DESC_H
