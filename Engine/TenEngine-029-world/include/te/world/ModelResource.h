/**
 * @file ModelResource.h
 * @brief 029-World: IModelResource view (contract: specs/_contracts/029-world-ABI.md).
 * Model resource view aggregates mesh/material refs and submesh-to-material mapping.
 * 013 loads Model via RequestLoadAsync(..., Model, ...) and returns IResource*; caller casts to IModelResource*.
 */

#ifndef TE_WORLD_MODEL_RESOURCE_H
#define TE_WORLD_MODEL_RESOURCE_H

#include <te/resource/MeshResource.h>
#include <te/resource/MaterialResource.h>
#include <cstddef>
#include <cstdint>

namespace te {
namespace world {

/**
 * @brief Model resource view: mesh + materials + submeshMaterialIndices.
 * Implemented by 013 (or 029); 013 returns IResource* from LoadSync(..., Model), caller casts to IModelResource*.
 */
class IModelResource {
public:
    virtual ~IModelResource() = default;

    /** @return Mesh resource (single mesh with submeshes); may be nullptr if not loaded. */
    virtual te::resource::IMeshResource* GetMesh() = 0;

    /** @return Number of materials. */
    virtual std::size_t GetMaterialCount() const = 0;

    /** @return Material at index (0 .. GetMaterialCount()-1). */
    virtual te::resource::IMaterialResource* GetMaterial(std::size_t index) = 0;

    /** @return Material index for the given submesh (into GetMaterial(i)). */
    virtual std::uint32_t GetSubmeshMaterialIndex(std::uint32_t submeshIndex) const = 0;
};

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_MODEL_RESOURCE_H
