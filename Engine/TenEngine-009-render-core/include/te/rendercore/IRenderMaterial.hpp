/** @file IRenderMaterial.hpp
 *  009-RenderCore: Render material = IShadingState + uniform buffer + bindings.
 */
#pragma once

#include <te/rendercore/IShadingState.hpp>
#include <cstdint>

namespace te {
namespace rhi {
struct IDevice;
struct IRenderPass;
struct IDescriptorSetLayout;
struct IPSO;
struct IDescriptorSet;
struct ITexture;
}
namespace rendercore {

struct IUniformBuffer;

/** Material: shading state, UB, descriptor set, PSO. Set CPU data via SetData*, CreateDeviceResource, UpdateDeviceResource. */
struct IRenderMaterial : IShadingState {
  ~IRenderMaterial() override = default;
  virtual IUniformBuffer* GetUniformBuffer() = 0;
  virtual IUniformBuffer const* GetUniformBuffer() const = 0;
  virtual rhi::IDescriptorSet* GetDescriptorSet() = 0;
  virtual rhi::IDescriptorSet const* GetDescriptorSet() const = 0;
  virtual rhi::IPSO* GetGraphicsPSO(std::uint32_t subpassIndex = 0) = 0;
  virtual rhi::IPSO const* GetGraphicsPSO(std::uint32_t subpassIndex = 0) const = 0;

  /** Create GPU resources (PSO, UB, descriptor set). Optional renderPass for subpass-specific PSO. */
  virtual void CreateDeviceResource() = 0;
  virtual void CreateDeviceResource(rhi::IRenderPass* renderPass, std::uint32_t subpassCount,
                                    rhi::IDescriptorSetLayout* skinLayout = nullptr) = 0;
  /** Upload CPU data to GPU (UB + descriptor set for frame). Call after SetData* before draw. */
  virtual void UpdateDeviceResource(rhi::IDevice* device, std::uint32_t frameSlot) = 0;
  /** Set CPU uniform parameter by name (std140). */
  virtual void SetDataParameter(char const* name, void const* data, std::size_t size) = 0;
  /** Set CPU texture at binding (RHI texture pointer). */
  virtual void SetDataTexture(std::uint32_t binding, rhi::ITexture* texture) = 0;
  /** Set CPU texture by binding name. */
  virtual void SetDataTextureByName(char const* name, rhi::ITexture* texture) = 0;
  virtual bool IsDeviceReady() const = 0;
};

}  // namespace rendercore
}  // namespace te
