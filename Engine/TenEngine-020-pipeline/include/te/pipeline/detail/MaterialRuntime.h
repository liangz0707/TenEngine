/**
 * @file MaterialRuntime.h
 * @brief 020-Pipeline: Encapsulated material runtime (bind texture, commit, PSO/DS).
 * Callers use IMaterialHandle only; MaterialResource is not exposed.
 */

#ifndef TE_PIPELINE_DETAIL_MATERIAL_RUNTIME_H
#define TE_PIPELINE_DETAIL_MATERIAL_RUNTIME_H

#include <te/pipelinecore/FrameGraph.h>
#include <cstdint>

namespace te {
namespace rhi {
struct IDevice;
struct ITexture;
struct IPSO;
struct IDescriptorSet;
}
namespace pipeline {
namespace detail {

/// Bind runtime texture at binding (e.g. post-process input). Use before Commit.
void MaterialRuntimeBindTexture(pipelinecore::IMaterialHandle const* handle, uint32_t binding, te::rhi::ITexture* texture);

/// Commit current frame descriptor (UB + runtime textures). Call after all BindTexture for this frame.
void MaterialRuntimeCommit(pipelinecore::IMaterialHandle const* handle, te::rhi::IDevice* device, uint32_t frameSlot);

/// PSO for the given subpass.
te::rhi::IPSO* MaterialRuntimeGetPSO(pipelinecore::IMaterialHandle const* handle, uint32_t subpassIndex);

/// Descriptor set (valid after Commit).
te::rhi::IDescriptorSet* MaterialRuntimeGetDescriptorSet(pipelinecore::IMaterialHandle const* handle);

}  // namespace detail
}  // namespace pipeline
}  // namespace te

#endif
