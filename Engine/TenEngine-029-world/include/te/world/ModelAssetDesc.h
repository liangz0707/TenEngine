/**
 * @file ModelAssetDesc.h
 * @brief 029-World: .model asset description (contract: specs/_contracts/029-world-ABI.md).
 * Model asset describes mesh/material references and submesh-to-material mapping.
 */

#ifndef TE_WORLD_MODEL_ASSET_DESC_H
#define TE_WORLD_MODEL_ASSET_DESC_H

#include <te/resource/ResourceId.h>
#include <vector>
#include <cstdint>

namespace te {
namespace world {

/**
 * @brief .model asset description; owned by 029, registered with 002 for serialization.
 * 013 deserializes .model to this; 029 or 013 assembles RResource from it.
 */
struct ModelAssetDesc {
    /** Mesh resource GUIDs (e.g. one mesh with multiple submeshes) */
    std::vector<te::resource::ResourceId> meshGuids;
    /** Material resource GUIDs */
    std::vector<te::resource::ResourceId> materialGuids;
    /** Per-submesh material index into materialGuids */
    std::vector<std::uint32_t> submeshMaterialIndices;

    ModelAssetDesc() = default;
};

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_MODEL_ASSET_DESC_H
