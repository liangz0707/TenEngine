/**
 * @file device.hpp
 * @brief RHI device abstraction (contract: specs/_contracts/008-rhi-public-api.md).
 */
#ifndef TE_RHI_DEVICE_HPP
#define TE_RHI_DEVICE_HPP

#include "te/rhi/queue.hpp"
#include "te/rhi/types.hpp"
#include "te/rhi/resources.hpp"
#include "te/rhi/pso.hpp"
#include "te/rhi/sync.hpp"

namespace te {
namespace rhi {

struct ICommandList;

/** Graphics device abstraction; creates queues, resources, PSO. Lifetime until DestroyDevice. */
struct IDevice {
  virtual ~IDevice() = default;

  virtual IQueue* GetQueue(QueueType type, uint32_t index) = 0;
  virtual DeviceFeatures const& GetFeatures() const = 0;
  virtual ICommandList* CreateCommandList() = 0;
  virtual void DestroyCommandList(ICommandList* cmd) = 0;

  virtual IBuffer* CreateBuffer(BufferDesc const& desc) = 0;
  virtual ITexture* CreateTexture(TextureDesc const& desc) = 0;
  virtual ISampler* CreateSampler(SamplerDesc const& desc) = 0;
  virtual ViewHandle CreateView(ViewDesc const& desc) = 0;
  virtual void DestroyBuffer(IBuffer* b) = 0;
  virtual void DestroyTexture(ITexture* t) = 0;
  virtual void DestroySampler(ISampler* s) = 0;

  virtual IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc) = 0;
  virtual IPSO* CreateComputePSO(ComputePSODesc const& desc) = 0;
  virtual void SetShader(IPSO* pso, void const* data, size_t size) = 0;
  virtual void Cache(IPSO* pso) = 0;
  virtual void DestroyPSO(IPSO* pso) = 0;

  virtual IFence* CreateFence() = 0;
  virtual ISemaphore* CreateSemaphore() = 0;
  virtual void DestroyFence(IFence* f) = 0;
  virtual void DestroySemaphore(ISemaphore* s) = 0;
};

/** Set default backend; used when CreateDevice() is called with no args. */
void SelectBackend(Backend b);

/** Get currently selected backend. */
Backend GetSelectedBackend();

/** Create device for given backend. Returns nullptr on failure. Does not depend on window/context. */
IDevice* CreateDevice(Backend backend);

/** Create device using GetSelectedBackend(). */
IDevice* CreateDevice();

/** Destroy device; nullptr is no-op. */
void DestroyDevice(IDevice* device);

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_DEVICE_HPP
