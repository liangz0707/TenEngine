/**
 * @file MaterialRuntime.cpp
 * @brief 020-Pipeline: Material runtime encapsulation (uses MaterialResource internally).
 */

#include <te/pipeline/detail/MaterialRuntime.h>
#include <te/material/MaterialResource.h>
#include <te/rhi/device.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/descriptor_set.hpp>

namespace te {
namespace pipeline {
namespace detail {

void MaterialRuntimeBindTexture(pipelinecore::IMaterialHandle const* handle, uint32_t binding, te::rhi::ITexture* texture) {
  if (!handle) return;
  auto* matRes = const_cast<te::material::MaterialResource*>(
      reinterpret_cast<te::material::MaterialResource const*>(handle));
  matRes->SetRuntimeTexture(binding, texture);
}

void MaterialRuntimeCommit(pipelinecore::IMaterialHandle const* handle, te::rhi::IDevice* device, uint32_t frameSlot) {
  if (!handle || !device) return;
  auto* matRes = const_cast<te::material::MaterialResource*>(
      reinterpret_cast<te::material::MaterialResource const*>(handle));
  matRes->UpdateDescriptorSetForFrame(device, frameSlot);
}

te::rhi::IPSO* MaterialRuntimeGetPSO(pipelinecore::IMaterialHandle const* handle, uint32_t subpassIndex) {
  if (!handle) return nullptr;
  auto* matRes = reinterpret_cast<te::material::MaterialResource const*>(handle);
  return matRes->GetGraphicsPSO(subpassIndex);
}

te::rhi::IDescriptorSet* MaterialRuntimeGetDescriptorSet(pipelinecore::IMaterialHandle const* handle) {
  if (!handle) return nullptr;
  auto* matRes = reinterpret_cast<te::material::MaterialResource const*>(handle);
  return matRes->GetDescriptorSet();
}

}  // namespace detail
}  // namespace pipeline
}  // namespace te
