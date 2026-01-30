/**
 * @file device.cpp
 * @brief RHI device implementation (stub backend for testing when Vulkan/D3D12/Metal unavailable).
 */
#include "te/rhi/device.hpp"
#include "te/rhi/command_list.hpp"
#include "te/rhi/resources.hpp"
#include "te/rhi/pso.hpp"
#include "te/rhi/sync.hpp"
#include "te/rhi/types.hpp"
#include "te/core/alloc.h"
#include <cstddef>
#include <new>

namespace te {
namespace rhi {

namespace {

Backend g_selected_backend{Backend::Vulkan};
constexpr uint32_t kMaxQueuesPerType = 1u;

struct QueueStub : IQueue {};

struct CommandListStub : ICommandList {
  void Draw(uint32_t vc, uint32_t ic, uint32_t fv, uint32_t fi) override { (void)vc;(void)ic;(void)fv;(void)fi; }
  void Dispatch(uint32_t x, uint32_t y, uint32_t z) override { (void)x;(void)y;(void)z; }
  void Copy(void const* s, void* d, size_t sz) override { (void)s;(void)d;(void)sz; }
  void ResourceBarrier() override {}
};

struct BufferStub : IBuffer {};
struct TextureStub : ITexture {};
struct SamplerStub : ISampler {};
struct PSOStub : IPSO {};
struct FenceStub : IFence {};
struct SemaphoreStub : ISemaphore {};

struct DeviceStub : IDevice {
  DeviceFeatures features{4096, 256};
  QueueStub graphics_queue;

  IQueue* GetQueue(QueueType type, uint32_t index) override {
    if (index >= kMaxQueuesPerType) return nullptr;
    if (type == QueueType::Graphics) return &graphics_queue;
    return nullptr;
  }
  DeviceFeatures const& GetFeatures() const override { return features; }
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

  IFence* CreateFence() override {
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
};

}  // namespace

void SelectBackend(Backend b) {
  g_selected_backend = b;
}

Backend GetSelectedBackend() {
  return g_selected_backend;
}

IDevice* CreateDevice(Backend backend) {
  (void)backend;
  auto* dev = static_cast<DeviceStub*>(core::Alloc(sizeof(DeviceStub), alignof(DeviceStub)));
  if (!dev) return nullptr;
  new (dev) DeviceStub();
  return dev;
}

IDevice* CreateDevice() {
  return CreateDevice(GetSelectedBackend());
}

void DestroyDevice(IDevice* device) {
  if (device) {
    static_cast<DeviceStub*>(device)->~DeviceStub();
    core::Free(device);
  }
}

}  // namespace rhi
}  // namespace te
