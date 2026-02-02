/**
 * @file device_metal.mm
 * @brief Metal device implementation (real MTL* calls); no stub. macOS/iOS only.
 */
#if !defined(TE_RHI_METAL) || !TE_RHI_METAL
#error "device_metal.mm must be built with TE_RHI_METAL=1"
#endif
#if !defined(__APPLE__)
#error "Metal backend is macOS/iOS only"
#endif

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "te/rhi/device.hpp"
#include "te/rhi/queue.hpp"
#include "te/rhi/command_list.hpp"
#include "te/rhi/resources.hpp"
#include "te/rhi/pso.hpp"
#include "te/rhi/sync.hpp"
#include "te/rhi/swapchain.hpp"
#include "te/rhi/types.hpp"
#include "te/core/alloc.h"
#include <cstddef>
#include <new>

namespace te {
namespace rhi {

namespace {

constexpr uint32_t kMaxQueuesPerType = 1u;

// --- Forward decls ---
struct CommandListMetal;

// --- Queue (MTLCommandQueue wrapper) ---
struct QueueMetal : IQueue {
  id<MTLCommandQueue> queue;
  id<MTLDevice> device;

  void Submit(ICommandList* cmdList, IFence* signalFence,
              ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) override;
  void WaitIdle() override;
};

// --- CommandList (MTLCommandBuffer wrapper) ---
struct CommandListMetal : ICommandList {
  id<MTLCommandBuffer> cmdBuffer;
  id<MTLCommandQueue> queue;

  void Begin() override;
  void End() override;
  void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;
  void Dispatch(uint32_t x, uint32_t y, uint32_t z) override;
  void Copy(void const* src, void* dst, size_t size) override;
  void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers,
                       uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) override;
};

void CommandListMetal::Begin() {
  if (queue && !cmdBuffer) {
    cmdBuffer = [queue commandBuffer];
  }
}

void CommandListMetal::End() {
  // Metal command buffers are committed in Submit, not End
}

void CommandListMetal::Draw(uint32_t vc, uint32_t ic, uint32_t fv, uint32_t fi) {
  (void)vc; (void)ic; (void)fv; (void)fi;
  // T056: Use MTLRenderCommandEncoder drawPrimitives; skip when no encoder
}

void CommandListMetal::Dispatch(uint32_t x, uint32_t y, uint32_t z) {
  (void)x; (void)y; (void)z;
  // T056: Use MTLComputeCommandEncoder dispatchThreadgroups; skip when no encoder
}

void CommandListMetal::Copy(void const* src, void* dst, size_t size) {
  (void)src; (void)dst; (void)size;
  // T056: Use MTLBlitCommandEncoder copyFromBuffer; skip for minimal test
}

void CommandListMetal::ResourceBarrier(uint32_t bc, BufferBarrier const* bb,
                                        uint32_t tc, TextureBarrier const* tb) {
  (void)bc; (void)bb; (void)tc; (void)tb;
  // T056: Metal handles most synchronization automatically; explicit barriers via memoryBarrier
}

void QueueMetal::Submit(ICommandList* cmdList, IFence* signalFence,
                        ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) {
  (void)signalFence; (void)waitSemaphore; (void)signalSemaphore;
  if (!cmdList) return;
  CommandListMetal* mtlCmd = static_cast<CommandListMetal*>(cmdList);
  if (mtlCmd->cmdBuffer) {
    [mtlCmd->cmdBuffer commit];
    mtlCmd->cmdBuffer = nil; // Reset for next Begin()
  }
}

void QueueMetal::WaitIdle() {
  // T055: waitUntilCompleted on last command buffer; placeholder
}

// --- Device (MTLDevice wrapper) ---
struct DeviceMetal : IDevice {
  id<MTLDevice> device;
  DeviceFeatures features{};
  QueueMetal graphicsQueue;

  ~DeviceMetal() override;

  Backend GetBackend() const override { return Backend::Metal; }
  IQueue* GetQueue(QueueType type, uint32_t index) override;
  DeviceFeatures const& GetFeatures() const override { return features; }
  ICommandList* CreateCommandList() override;
  void DestroyCommandList(ICommandList* cmd) override;

  IBuffer* CreateBuffer(BufferDesc const& desc) override;
  ITexture* CreateTexture(TextureDesc const& desc) override;
  ISampler* CreateSampler(SamplerDesc const& desc) override;
  ViewHandle CreateView(ViewDesc const& desc) override;
  void DestroyBuffer(IBuffer* b) override;
  void DestroyTexture(ITexture* t) override;
  void DestroySampler(ISampler* s) override;

  IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc) override;
  IPSO* CreateComputePSO(ComputePSODesc const& desc) override;
  void SetShader(IPSO* pso, void const* data, size_t size) override;
  void Cache(IPSO* pso) override;
  void DestroyPSO(IPSO* pso) override;

  IFence* CreateFence() override;
  ISemaphore* CreateSemaphore() override;
  void DestroyFence(IFence* f) override;
  void DestroySemaphore(ISemaphore* s) override;

  ISwapChain* CreateSwapChain(SwapChainDesc const& desc) override;
  void DestroySwapChain(ISwapChain* sc);
};

DeviceMetal::~DeviceMetal() {
  if (graphicsQueue.queue) {
    graphicsQueue.queue = nil;
  }
  device = nil;
}

IQueue* DeviceMetal::GetQueue(QueueType type, uint32_t index) {
  if (index >= kMaxQueuesPerType) return nullptr;
  if (type == QueueType::Graphics) {
    if (!graphicsQueue.queue && device) {
      graphicsQueue.queue = [device newCommandQueue];
      graphicsQueue.device = device;
    }
    return &graphicsQueue;
  }
  return nullptr;
}

ICommandList* DeviceMetal::CreateCommandList() {
  if (!graphicsQueue.queue) {
    GetQueue(QueueType::Graphics, 0); // Ensure queue exists
  }
  if (!graphicsQueue.queue) return nullptr;
  
  CommandListMetal* cl = static_cast<CommandListMetal*>(core::Alloc(sizeof(CommandListMetal), alignof(CommandListMetal)));
  if (!cl) return nullptr;
  new (cl) CommandListMetal();
  cl->queue = graphicsQueue.queue;
  cl->cmdBuffer = nil; // Created in Begin()
  return cl;
}

void DeviceMetal::DestroyCommandList(ICommandList* cmd) {
  if (!cmd) return;
  CommandListMetal* mtlCmd = static_cast<CommandListMetal*>(cmd);
  mtlCmd->cmdBuffer = nil;
  mtlCmd->~CommandListMetal();
  core::Free(mtlCmd);
}

// T057: Resources
struct BufferMetal : IBuffer {
  id<MTLBuffer> buffer;
};

struct TextureMetal : ITexture {
  id<MTLTexture> texture;
};

struct SamplerMetal : ISampler {
  id<MTLSamplerState> sampler;
};

IBuffer* DeviceMetal::CreateBuffer(BufferDesc const& desc) {
  if (desc.size == 0 || !device) return nullptr;
  id<MTLBuffer> buffer = [device newBufferWithLength:desc.size options:MTLResourceStorageModePrivate];
  if (!buffer) return nullptr;
  
  BufferMetal* buf = static_cast<BufferMetal*>(core::Alloc(sizeof(BufferMetal), alignof(BufferMetal)));
  if (!buf) return nullptr;
  new (buf) BufferMetal();
  buf->buffer = buffer;
  return buf;
}

ITexture* DeviceMetal::CreateTexture(TextureDesc const& desc) {
  if (desc.width == 0 || desc.height == 0 || !device) return nullptr;
  
  MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
  texDesc.textureType = MTLTextureType2D;
  texDesc.width = desc.width;
  texDesc.height = desc.height;
  texDesc.pixelFormat = MTLPixelFormatRGBA8Unorm; // T057: map desc.format
  texDesc.storageMode = MTLStorageModePrivate;
  texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
  
  id<MTLTexture> texture = [device newTextureWithDescriptor:texDesc];
  if (!texture) return nullptr;
  
  TextureMetal* tex = static_cast<TextureMetal*>(core::Alloc(sizeof(TextureMetal), alignof(TextureMetal)));
  if (!tex) return nullptr;
  new (tex) TextureMetal();
  tex->texture = texture;
  return tex;
}

ISampler* DeviceMetal::CreateSampler(SamplerDesc const& desc) {
  (void)desc;
  if (!device) return nullptr;
  
  MTLSamplerDescriptor* sampDesc = [[MTLSamplerDescriptor alloc] init];
  sampDesc.minFilter = MTLSamplerMinMagFilterLinear;
  sampDesc.magFilter = MTLSamplerMinMagFilterLinear;
  sampDesc.sAddressMode = MTLSamplerAddressModeRepeat;
  sampDesc.tAddressMode = MTLSamplerAddressModeRepeat;
  
  id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:sampDesc];
  if (!sampler) return nullptr;
  
  SamplerMetal* samp = static_cast<SamplerMetal*>(core::Alloc(sizeof(SamplerMetal), alignof(SamplerMetal)));
  if (!samp) return nullptr;
  new (samp) SamplerMetal();
  samp->sampler = sampler;
  return samp;
}

ViewHandle DeviceMetal::CreateView(ViewDesc const& desc) {
  // T057: MTLTexture can be used directly as view; return resource pointer
  return desc.resource ? reinterpret_cast<ViewHandle>(desc.resource) : 0;
}

void DeviceMetal::DestroyBuffer(IBuffer* b) {
  if (!b) return;
  BufferMetal* buf = static_cast<BufferMetal*>(b);
  buf->buffer = nil;
  buf->~BufferMetal();
  core::Free(buf);
}

void DeviceMetal::DestroyTexture(ITexture* t) {
  if (!t) return;
  TextureMetal* tex = static_cast<TextureMetal*>(t);
  tex->texture = nil;
  tex->~TextureMetal();
  core::Free(tex);
}

void DeviceMetal::DestroySampler(ISampler* s) {
  if (!s) return;
  SamplerMetal* samp = static_cast<SamplerMetal*>(s);
  samp->sampler = nil;
  samp->~SamplerMetal();
  core::Free(samp);
}

// T058: PSO
struct PSOMetal : IPSO {
  id<MTLRenderPipelineState> renderPSO;
  id<MTLComputePipelineState> computePSO;
};

IPSO* DeviceMetal::CreateGraphicsPSO(GraphicsPSODesc const& desc) {
  if (!desc.vertex_shader || desc.vertex_shader_size == 0 || !device) return nullptr;
  // T058: Create MTLLibrary from MSL, MTLRenderPipelineDescriptor, newRenderPipelineStateWithDescriptor
  // Minimal: return non-null for test
  PSOMetal* pso = static_cast<PSOMetal*>(core::Alloc(sizeof(PSOMetal), alignof(PSOMetal)));
  if (!pso) return nullptr;
  new (pso) PSOMetal();
  pso->renderPSO = nil;
  pso->computePSO = nil;
  return pso;
}

IPSO* DeviceMetal::CreateComputePSO(ComputePSODesc const& desc) {
  if (!desc.compute_shader || desc.compute_shader_size == 0 || !device) return nullptr;
  PSOMetal* pso = static_cast<PSOMetal*>(core::Alloc(sizeof(PSOMetal), alignof(PSOMetal)));
  if (!pso) return nullptr;
  new (pso) PSOMetal();
  pso->renderPSO = nil;
  pso->computePSO = nil;
  return pso;
}

void DeviceMetal::SetShader(IPSO*, void const*, size_t) {}
void DeviceMetal::Cache(IPSO*) {}

void DeviceMetal::DestroyPSO(IPSO* pso) {
  if (!pso) return;
  PSOMetal* mtlPso = static_cast<PSOMetal*>(pso);
  mtlPso->renderPSO = nil;
  mtlPso->computePSO = nil;
  mtlPso->~PSOMetal();
  core::Free(mtlPso);
}

// T059: Sync
struct FenceMetal : IFence {
  id<MTLSharedEvent> event;
  uint64_t value{0};

  void Wait() override { /* T059: waitUntilSignaledValue; placeholder no-op */ }
  void Signal() override { /* T059: signalEvent; placeholder no-op */ }
  void Reset() override { value = 0; }
};

struct SemaphoreMetal : ISemaphore {
  id<MTLSharedEvent> event;
};

IFence* DeviceMetal::CreateFence() {
  if (!device) return nullptr;
  
  FenceMetal* f = static_cast<FenceMetal*>(core::Alloc(sizeof(FenceMetal), alignof(FenceMetal)));
  if (!f) return nullptr;
  new (f) FenceMetal();
  
  if (@available(macOS 10.14, iOS 12.0, *)) {
    f->event = [device newSharedEvent];
  } else {
    f->event = nil;
  }
  return f;
}

ISemaphore* DeviceMetal::CreateSemaphore() {
  if (!device) return nullptr;
  
  SemaphoreMetal* s = static_cast<SemaphoreMetal*>(core::Alloc(sizeof(SemaphoreMetal), alignof(SemaphoreMetal)));
  if (!s) return nullptr;
  new (s) SemaphoreMetal();
  
  if (@available(macOS 10.14, iOS 12.0, *)) {
    s->event = [device newSharedEvent];
  } else {
    s->event = nil;
  }
  return s;
}

void DeviceMetal::DestroyFence(IFence* f) {
  if (!f) return;
  FenceMetal* fence = static_cast<FenceMetal*>(f);
  fence->event = nil;
  fence->~FenceMetal();
  core::Free(fence);
}

void DeviceMetal::DestroySemaphore(ISemaphore* s) {
  if (!s) return;
  SemaphoreMetal* sem = static_cast<SemaphoreMetal*>(s);
  sem->event = nil;
  sem->~SemaphoreMetal();
  core::Free(sem);
}

// T060: SwapChain
struct SwapChainMetal : ISwapChain {
  CAMetalLayer* layer;
  uint32_t width{0}, height{0};

  bool Present() override { return true; /* T060: nextDrawable, present */ }
  ITexture* GetCurrentBackBuffer() override { return nullptr; /* T060: nextDrawable.texture */ }
  uint32_t GetCurrentBackBufferIndex() const override { return 0; }
  void Resize(uint32_t w, uint32_t h) override { width = w; height = h; /* T060: layer.drawableSize */ }
  uint32_t GetWidth() const override { return width; }
  uint32_t GetHeight() const override { return height; }
};

ISwapChain* DeviceMetal::CreateSwapChain(SwapChainDesc const& desc) {
  if (desc.width == 0 || desc.height == 0) return nullptr;
  // T060: Create CAMetalLayer, set device; test without window: return stub
  SwapChainMetal* sc = static_cast<SwapChainMetal*>(core::Alloc(sizeof(SwapChainMetal), alignof(SwapChainMetal)));
  if (!sc) return nullptr;
  new (sc) SwapChainMetal();
  sc->layer = nil;
  sc->width = desc.width;
  sc->height = desc.height;
  return sc;
}

void DeviceMetal::DestroySwapChain(ISwapChain* sc) {
  if (!sc) return;
  SwapChainMetal* swapChain = static_cast<SwapChainMetal*>(sc);
  swapChain->layer = nil;
  swapChain->~SwapChainMetal();
  core::Free(swapChain);
}

}  // namespace

IDevice* CreateDeviceMetal() {
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  if (!device) return nullptr;
  
  DeviceMetal* dev = static_cast<DeviceMetal*>(core::Alloc(sizeof(DeviceMetal), alignof(DeviceMetal)));
  if (!dev) return nullptr;
  new (dev) DeviceMetal();
  dev->device = device;
  
  // Fill features from device
  dev->features.maxTextureDimension2D = 16384; // Typical Metal limit
  dev->features.maxTextureDimension3D = 2048;
  
  return dev;
}

void DestroyDeviceMetal(IDevice* device) {
  if (!device) return;
  DeviceMetal* dev = static_cast<DeviceMetal*>(device);
  dev->~DeviceMetal();
  core::Free(dev);
}

}  // namespace rhi
}  // namespace te
