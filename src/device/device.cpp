/**
 * @file device.cpp
 * @brief RHI device implementation (stub backend for testing when Vulkan/D3D12/Metal unavailable).
 */
#include "te/rhi/device.hpp"
#include "te/rhi/command_list.hpp"
#include "te/rhi/resources.hpp"
#include "te/rhi/pso.hpp"
#include "te/rhi/sync.hpp"
#include "te/rhi/swapchain.hpp"
#include "te/rhi/types.hpp"
#include "te/core/alloc.h"
#include <cstddef>
#include <new>
#if defined(TE_RHI_VULKAN) && TE_RHI_VULKAN
#include "te/rhi/backend_vulkan.hpp"
#endif
#if defined(TE_RHI_D3D12) && TE_RHI_D3D12
#include "te/rhi/backend_d3d12.hpp"
#endif
#if defined(TE_RHI_D3D11) && TE_RHI_D3D11
#include "te/rhi/backend_d3d11.hpp"
#endif
#if defined(TE_RHI_METAL) && TE_RHI_METAL
#include "te/rhi/backend_metal.hpp"
#endif

namespace te {
namespace rhi {

namespace {

Backend g_selected_backend{Backend::Vulkan};
constexpr uint32_t kMaxQueuesPerType = 1u;

struct SwapChainStub : ISwapChain {
  uint32_t w{0}, h{0}, idx{0};
  bool Present() override { return true; }
  ITexture* GetCurrentBackBuffer() override { return nullptr; }
  uint32_t GetCurrentBackBufferIndex() const override { return idx; }
  void Resize(uint32_t width, uint32_t height) override { w = width; h = height; }
  uint32_t GetWidth() const override { return w; }
  uint32_t GetHeight() const override { return h; }
};

struct QueueStub : IQueue {
  void Submit(ICommandList*, IFence*, ISemaphore*, ISemaphore*) override {}
  void WaitIdle() override {}
};

struct CommandListStub : ICommandList {
  void Begin() override {}
  void End() override {}
  void Draw(uint32_t vc, uint32_t ic, uint32_t fv, uint32_t fi) override { (void)vc;(void)ic;(void)fv;(void)fi; }
  void DrawIndexed(uint32_t ic, uint32_t inst, uint32_t fi, int32_t vo, uint32_t finst) override { (void)ic;(void)inst;(void)fi;(void)vo;(void)finst; }
  void Dispatch(uint32_t x, uint32_t y, uint32_t z) override { (void)x;(void)y;(void)z; }
  void Copy(void const* s, void* d, size_t sz) override { (void)s;(void)d;(void)sz; }
  void ResourceBarrier(uint32_t, BufferBarrier const*, uint32_t, TextureBarrier const*) override {}
  void SetViewport(uint32_t, uint32_t, Viewport const*) override {}
  void SetScissor(uint32_t, uint32_t, ScissorRect const*) override {}
  void BeginRenderPass(RenderPassDesc const&) override {}
  void EndRenderPass() override {}
  void CopyBuffer(IBuffer*, size_t, IBuffer*, size_t, size_t) override {}
  void CopyBufferToTexture(IBuffer*, size_t, ITexture*, TextureRegion const&) override {}
  void CopyTextureToBuffer(ITexture*, TextureRegion const&, IBuffer*, size_t) override {}
  void BuildAccelerationStructure(RaytracingAccelerationStructureDesc const&, IBuffer*, IBuffer*) override {}
  void DispatchRays(DispatchRaysDesc const&) override {}
};

struct BufferStub : IBuffer {};
struct TextureStub : ITexture {};
struct SamplerStub : ISampler {};
struct PSOStub : IPSO {};
struct FenceStub : IFence {
  void Wait() override {}
  void Signal() override {}
  void Reset() override {}
};
struct SemaphoreStub : ISemaphore {};

struct DeviceStub : IDevice {
  DeviceFeatures features{4096, 256};
  DeviceLimits limits{};
  QueueStub graphics_queue;

  Backend GetBackend() const override { return Backend::Vulkan; }
  IQueue* GetQueue(QueueType type, uint32_t index) override {
    if (index >= kMaxQueuesPerType) return nullptr;
    if (type == QueueType::Graphics) return &graphics_queue;
    return nullptr;
  }
  DeviceFeatures const& GetFeatures() const override { return features; }
  DeviceLimits const& GetLimits() const override { return limits; }
  ICommandList* CreateCommandList() override {
    auto* p = static_cast<CommandListStub*>(core::Alloc(sizeof(CommandListStub), alignof(CommandListStub)));
    if (!p) return nullptr;
    new (p) CommandListStub();
    return p;
  }
  void DestroyCommandList(ICommandList* cmd) override {
    if (cmd) {
      static_cast<CommandListStub*>(cmd)->~CommandListStub();
      core::Free(cmd);
    }
  }

  IBuffer* CreateBuffer(BufferDesc const& desc) override {
    if (desc.size == 0) return nullptr;
    auto* p = static_cast<BufferStub*>(core::Alloc(sizeof(BufferStub), alignof(BufferStub)));
    if (!p) return nullptr;
    new (p) BufferStub();
    return p;
  }
  ITexture* CreateTexture(TextureDesc const& desc) override {
    if (desc.width == 0 || desc.height == 0) return nullptr;
    auto* p = static_cast<TextureStub*>(core::Alloc(sizeof(TextureStub), alignof(TextureStub)));
    if (!p) return nullptr;
    new (p) TextureStub();
    return p;
  }
  ISampler* CreateSampler(SamplerDesc const&) override {
    auto* p = static_cast<SamplerStub*>(core::Alloc(sizeof(SamplerStub), alignof(SamplerStub)));
    if (!p) return nullptr;
    new (p) SamplerStub();
    return p;
  }
  ViewHandle CreateView(ViewDesc const& desc) override {
    return desc.resource ? reinterpret_cast<ViewHandle>(desc.resource) : 0;
  }
  void DestroyBuffer(IBuffer* b) override {
    if (b) { static_cast<BufferStub*>(b)->~BufferStub(); core::Free(b); }
  }
  void DestroyTexture(ITexture* t) override {
    if (t) { static_cast<TextureStub*>(t)->~TextureStub(); core::Free(t); }
  }
  void DestroySampler(ISampler* s) override {
    if (s) { static_cast<SamplerStub*>(s)->~SamplerStub(); core::Free(s); }
  }

  IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc) override {
    if (!desc.vertex_shader || desc.vertex_shader_size == 0) return nullptr;
    auto* p = static_cast<PSOStub*>(core::Alloc(sizeof(PSOStub), alignof(PSOStub)));
    if (!p) return nullptr;
    new (p) PSOStub();
    return p;
  }
  IPSO* CreateComputePSO(ComputePSODesc const& desc) override {
    if (!desc.compute_shader || desc.compute_shader_size == 0) return nullptr;
    auto* p = static_cast<PSOStub*>(core::Alloc(sizeof(PSOStub), alignof(PSOStub)));
    if (!p) return nullptr;
    new (p) PSOStub();
    return p;
  }
  void SetShader(IPSO* pso, void const*, size_t) override { (void)pso; }
  void Cache(IPSO* pso) override { (void)pso; }
  void DestroyPSO(IPSO* pso) override {
    if (pso) { static_cast<PSOStub*>(pso)->~PSOStub(); core::Free(pso); }
  }

  IFence* CreateFence(bool initialSignaled = false) override {
    (void)initialSignaled;
    auto* p = static_cast<FenceStub*>(core::Alloc(sizeof(FenceStub), alignof(FenceStub)));
    if (!p) return nullptr;
    new (p) FenceStub();
    return p;
  }
  ISemaphore* CreateSemaphore() override {
    auto* p = static_cast<SemaphoreStub*>(core::Alloc(sizeof(SemaphoreStub), alignof(SemaphoreStub)));
    if (!p) return nullptr;
    new (p) SemaphoreStub();
    return p;
  }
  void DestroyFence(IFence* f) override {
    if (f) { static_cast<FenceStub*>(f)->~FenceStub(); core::Free(f); }
  }
  void DestroySemaphore(ISemaphore* s) override {
    if (s) { static_cast<SemaphoreStub*>(s)->~SemaphoreStub(); core::Free(s); }
  }

  ISwapChain* CreateSwapChain(SwapChainDesc const& desc) override {
    if (desc.width == 0 || desc.height == 0) return nullptr;
    auto* p = static_cast<SwapChainStub*>(core::Alloc(sizeof(SwapChainStub), alignof(SwapChainStub)));
    if (!p) return nullptr;
    new (p) SwapChainStub();
    p->w = desc.width;
    p->h = desc.height;
    return p;
  }

  IDescriptorSetLayout* CreateDescriptorSetLayout(DescriptorSetLayoutDesc const&) override { return nullptr; }
  IDescriptorSet* AllocateDescriptorSet(IDescriptorSetLayout*) override { return nullptr; }
  void UpdateDescriptorSet(IDescriptorSet*, DescriptorWrite const*, uint32_t) override {}
  void DestroyDescriptorSetLayout(IDescriptorSetLayout*) override {}
  void DestroyDescriptorSet(IDescriptorSet*) override {}
};

}  // namespace

void SelectBackend(Backend b) {
  g_selected_backend = b;
}

Backend GetSelectedBackend() {
  return g_selected_backend;
}

IDevice* CreateDevice(Backend backend) {
#if defined(TE_RHI_VULKAN) && TE_RHI_VULKAN
  if (backend == Backend::Vulkan)
    return CreateDeviceVulkan();
#endif
#if defined(TE_RHI_D3D12) && TE_RHI_D3D12
  if (backend == Backend::D3D12)
    return CreateDeviceD3D12();
#endif
#if defined(TE_RHI_D3D11) && TE_RHI_D3D11
  if (backend == Backend::D3D11)
    return CreateDeviceD3D11();
#endif
#if defined(TE_RHI_METAL) && TE_RHI_METAL
  if (backend == Backend::Metal)
    return CreateDeviceMetal();
#endif
  (void)backend;
  return nullptr;
}

IDevice* CreateDevice() {
  return CreateDevice(GetSelectedBackend());
}

void DestroyDevice(IDevice* device) {
  if (!device) return;
#if defined(TE_RHI_VULKAN) && TE_RHI_VULKAN
  if (device->GetBackend() == Backend::Vulkan) {
    DestroyDeviceVulkan(device);
    return;
  }
#endif
#if defined(TE_RHI_D3D12) && TE_RHI_D3D12
  if (device->GetBackend() == Backend::D3D12) {
    DestroyDeviceD3D12(device);
    return;
  }
#endif
#if defined(TE_RHI_D3D11) && TE_RHI_D3D11
  if (device->GetBackend() == Backend::D3D11) {
    DestroyDeviceD3D11(device);
    return;
  }
#endif
#if defined(TE_RHI_METAL) && TE_RHI_METAL
  if (device->GetBackend() == Backend::Metal) {
    DestroyDeviceMetal(device);
    return;
  }
#endif
  static_cast<DeviceStub*>(device)->~DeviceStub();
  core::Free(device);
}

}  // namespace rhi
}  // namespace te
