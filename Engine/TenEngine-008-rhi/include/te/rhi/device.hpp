/** @file device.hpp
 *  008-RHI ABI: SelectBackend, GetSelectedBackend, CreateDevice, DestroyDevice, IDevice.
 */
#pragma once

#include <te/rhi/types.hpp>
#include <te/rhi/queue.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/sync.hpp>
#include <te/rhi/swapchain.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/descriptor_set.hpp>
#include <te/rhi/raytracing.hpp>
#include <cstddef>

namespace te {
namespace rhi {

void SelectBackend(Backend b);
Backend GetSelectedBackend();

IDevice* CreateDevice(Backend backend);
IDevice* CreateDevice();
void DestroyDevice(IDevice* device);

struct IDevice {
  virtual Backend GetBackend() const = 0;
  virtual IQueue* GetQueue(QueueType type, uint32_t index) = 0;
  virtual DeviceFeatures const& GetFeatures() const = 0;
  virtual DeviceLimits const& GetLimits() const = 0;
  virtual ICommandList* CreateCommandList() = 0;
  virtual void DestroyCommandList(ICommandList* cmd) = 0;
  virtual IBuffer* CreateBuffer(BufferDesc const& desc) = 0;
  virtual void UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size) = 0;
  virtual ITexture* CreateTexture(TextureDesc const& desc) = 0;
  virtual ISampler* CreateSampler(SamplerDesc const& desc) = 0;
  virtual ViewHandle CreateView(ViewDesc const& desc) = 0;
  virtual void DestroyBuffer(IBuffer* b) = 0;
  virtual void DestroyTexture(ITexture* t) = 0;
  virtual void DestroySampler(ISampler* s) = 0;
  virtual IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc) = 0;
  /** Create PSO with custom descriptor set layout (set 0). When null, uses device default layout. */
  virtual IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc, IDescriptorSetLayout* layout) = 0;
  /** Create PSO for a specific render pass and subpass. When pass is null, uses default single-subpass render pass and subpass 0. */
  virtual IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc, IDescriptorSetLayout* layout,
                                  IRenderPass* pass, uint32_t subpassIndex) = 0;
  virtual IRenderPass* CreateRenderPass(RenderPassDesc const& desc) = 0;
  virtual void DestroyRenderPass(IRenderPass* pass) = 0;
  virtual IPSO* CreateComputePSO(ComputePSODesc const& desc) = 0;
  virtual void SetShader(IPSO* pso, void const* data, size_t size) = 0;
  virtual void Cache(IPSO* pso) = 0;
  virtual void DestroyPSO(IPSO* pso) = 0;
  virtual IFence* CreateFence(bool initialSignaled = false) = 0;
  virtual ISemaphore* CreateSemaphore() = 0;
  virtual void DestroyFence(IFence* f) = 0;
  virtual void DestroySemaphore(ISemaphore* s) = 0;
  virtual ISwapChain* CreateSwapChain(SwapChainDesc const& desc) = 0;
  virtual IDescriptorSetLayout* CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) = 0;
  virtual IDescriptorSet* AllocateDescriptorSet(IDescriptorSetLayout* layout) = 0;
  virtual void UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes,
                                   uint32_t writeCount) = 0;
  virtual void DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) = 0;
  virtual void DestroyDescriptorSet(IDescriptorSet* set) = 0;
  virtual ~IDevice() = default;
};

}  // namespace rhi
}  // namespace te
