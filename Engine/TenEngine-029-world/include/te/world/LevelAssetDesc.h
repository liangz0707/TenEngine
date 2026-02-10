/**
 * @file LevelAssetDesc.h
 * @brief 029-World: .level asset description (contract: specs/_contracts/029-world-public-api.md).
 * 029 owns; registered with 002 for serialization. 013 deserializes .level to this.
 */

#ifndef TE_WORLD_LEVEL_ASSET_DESC_H
#define TE_WORLD_LEVEL_ASSET_DESC_H

#include <te/scene/SceneTypes.h>
#include <te/resource/ResourceId.h>
#include <vector>
#include <string>

namespace te {
namespace world {

/**
 * @brief Single node description for level graph (029 side).
 * Used to build 004 SceneDesc; opaqueUserData will point to this for NodeFactoryFn.
 */
struct SceneNodeDesc {
    std::string name;
    te::scene::Transform localTransform;
    te::resource::ResourceId modelGuid;
    std::vector<SceneNodeDesc> children;

    SceneNodeDesc() = default;
};

/**
 * @brief .level asset description: root nodes only.
 * 029 owns; registered with 002 for serialization.
 */
struct LevelAssetDesc {
    std::vector<SceneNodeDesc> roots;

    LevelAssetDesc() = default;
};

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_LEVEL_ASSET_DESC_H
