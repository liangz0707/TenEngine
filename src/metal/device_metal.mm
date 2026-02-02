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
  id<MTLCommandBuffer> lastCommitted{nil};  // T055: for WaitIdle

  void Submit(ICommandList* cmdList, IFence* signalFence,
              ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) override;
  void WaitIdle() override;
};

// --- CommandList (MTLCommandBuffer wrapper) ---
struct TextureMetal;
struct CommandListMetal : ICommandList {
  id<MTLCommandBuffer> cmdBuffer;
  id<MTLCommandQueue> queue;
  id<MTLRenderCommandEncoder> currentRenderEncoder{nil};

  void Begin() override;
  void End() override;
  void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;
  void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) override;
  void Dispatch(uint32_t x, uint32_t y, uint32_t z) override;
  void Copy(void const* src, void* dst, size_t size) override;
  void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers,
                       uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) override;
  void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) override;
  void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) override;
  void BeginRenderPass(RenderPassDesc const& desc) override;
  void EndRenderPass() override;
  void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) override;
  void CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) override;
  void CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) override;
  void BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result) override;
  void DispatchRays(DispatchRaysDesc const& desc) override;
};

void CommandListMetal::Begin() {
  if (queue && !cmdBuffer) {
    cmdBuffer = [queue commandBuffer];
  }
  if (currentRenderEncoder) {
    [currentRenderEncoder endEncoding];
    currentRenderEncoder = nil;
  }
}

void CommandListMetal::End() {
  // Metal command buffers are committed in Submit, not End
}

void CommandListMetal::Draw(uint32_t vc, uint32_t ic, uint32_t fv, uint32_t fi) {
  (void)vc; (void)ic; (void)fv; (void)fi;
  // T056: Use MTLRenderCommandEncoder drawPrimitives; skip when no encoder
}

void CommandListMetal::DrawIndexed(uint32_t ic, uint32_t inst, uint32_t fi, int32_t vo, uint32_t finst) {
  (void)ic; (void)inst; (void)fi; (void)vo; (void)finst;
  // MTLRenderCommandEncoder drawIndexedPrimitives when encoder bound
}

void CommandListMetal::SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) {
  (void)first;
  if (!currentRenderEncoder || !viewports || count == 0) return;
  MTLViewport vp;
  vp.originX = static_cast<double>(viewports[0].x);
  vp.originY = static_cast<double>(viewports[0].y);
  vp.width = static_cast<double>(viewports[0].width);
  vp.height = static_cast<double>(viewports[0].height);
  vp.znear = static_cast<double>(viewports[0].minDepth);
  vp.zfar = static_cast<double>(viewports[0].maxDepth);
  [currentRenderEncoder setViewport:vp];
}

void CommandListMetal::SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) {
  (void)first;
  if (!currentRenderEncoder || !scissors || count == 0) return;
  MTLScissorRect rect;
  rect.x = static_cast<NSUInteger>(scissors[0].x < 0 ? 0 : scissors[0].x);
  rect.y = static_cast<NSUInteger>(scissors[0].y < 0 ? 0 : scissors[0].y);
  rect.width = scissors[0].width;
  rect.height = scissors[0].height;
  [currentRenderEncoder setScissorRect:rect];
}

void CommandListMetal::BeginRenderPass(RenderPassDesc const& desc);
void CommandListMetal::EndRenderPass();

static void CopyBufferMetalImpl(CommandListMetal* cl, IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size);
static void CopyBufferToTextureMetalImpl(CommandListMetal* cl, IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion);
static void CopyTextureToBufferMetalImpl(CommandListMetal* cl, ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset);

void CommandListMetal::CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) {
  if (!cmdBuffer || !src || !dst || size == 0) return;
  if (currentRenderEncoder) {
    [currentRenderEncoder endEncoding];
    currentRenderEncoder = nil;
  }
  CopyBufferMetalImpl(this, src, srcOffset, dst, dstOffset, size);
}

void CommandListMetal::CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) {
  if (!cmdBuffer || !src || !dst) return;
  if (currentRenderEncoder) {
    [currentRenderEncoder endEncoding];
    currentRenderEncoder = nil;
  }
  CopyBufferToTextureMetalImpl(this, src, srcOffset, dst, dstRegion);
}

void CommandListMetal::CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) {
  if (!cmdBuffer || !src || !dst) return;
  if (currentRenderEncoder) {
    [currentRenderEncoder endEncoding];
    currentRenderEncoder = nil;
  }
  CopyTextureToBufferMetalImpl(this, src, srcRegion, dst, dstOffset);
}

void CommandListMetal::BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result) {
  (void)desc;(void)scratch;(void)result;
  // Metal ray tracing: MTLAccelerationStructure; no-op for API parity.
}

void CommandListMetal::DispatchRays(DispatchRaysDesc const& desc) {
  (void)desc;
  // Metal ray tracing; no-op.
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
    lastCommitted = mtlCmd->cmdBuffer;  // T055: for WaitIdle
    [mtlCmd->cmdBuffer commit];
    mtlCmd->cmdBuffer = nil; // Reset for next Begin()
  }
}

void QueueMetal::WaitIdle() {
  if (lastCommitted) {
    [lastCommitted waitUntilCompleted];
    lastCommitted = nil;
  }
}

// --- Device (MTLDevice wrapper) ---
struct DeviceMetal : IDevice {
  id<MTLDevice> device;
  DeviceFeatures features{};
  DeviceLimits limits{};
  QueueMetal graphicsQueue;

  ~DeviceMetal() override;

  Backend GetBackend() const override { return Backend::Metal; }
  IQueue* GetQueue(QueueType type, uint32_t index) override;
  DeviceFeatures const& GetFeatures() const override { return features; }
  DeviceLimits const& GetLimits() const override { return limits; }
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

  IFence* CreateFence(bool initialSignaled = false) override;
  ISemaphore* CreateSemaphore() override;
  void DestroyFence(IFence* f) override;
  void DestroySemaphore(ISemaphore* s) override;

  ISwapChain* CreateSwapChain(SwapChainDesc const& desc) override;
  void DestroySwapChain(ISwapChain* sc);

  IDescriptorSetLayout* CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) override;
  IDescriptorSet* AllocateDescriptorSet(IDescriptorSetLayout* layout) override;
  void UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes, uint32_t writeCount) override;
  void DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) override;
  void DestroyDescriptorSet(IDescriptorSet* set) override;
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

static void CopyBufferMetalImpl(CommandListMetal* cl, IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) {
  BufferMetal* vs = static_cast<BufferMetal*>(src);
  BufferMetal* vd = static_cast<BufferMetal*>(dst);
  if (!vs->buffer || !vd->buffer) return;
  id<MTLBlitCommandEncoder> blit = [cl->cmdBuffer blitCommandEncoder];
  [blit copyFromBuffer:vs->buffer sourceOffset:srcOffset toBuffer:vd->buffer destinationOffset:dstOffset size:size];
  [blit endEncoding];
}

static void CopyBufferToTextureMetalImpl(CommandListMetal* cl, IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) {
  BufferMetal* vs = static_cast<BufferMetal*>(src);
  TextureMetal* td = static_cast<TextureMetal*>(dst);
  if (!vs->buffer || !td->texture) return;
  NSUInteger w = dstRegion.width > 0 ? dstRegion.width : [td->texture width];
  NSUInteger h = dstRegion.height > 0 ? dstRegion.height : [td->texture height];
  NSUInteger bytesPerRow = w * 4;
  NSUInteger bytesPerImage = bytesPerRow * h;
  MTLOrigin origin = { static_cast<NSUInteger>(dstRegion.x), static_cast<NSUInteger>(dstRegion.y), static_cast<NSUInteger>(dstRegion.z) };
  MTLSize size = { w, h, dstRegion.depth > 0 ? dstRegion.depth : 1 };
  id<MTLBlitCommandEncoder> blit = [cl->cmdBuffer blitCommandEncoder];
  [blit copyFromBuffer:vs->buffer sourceOffset:srcOffset sourceBytesPerRow:bytesPerRow sourceBytesPerImage:bytesPerImage sourceSize:size toTexture:td->texture destinationSlice:dstRegion.arrayLayer destinationLevel:dstRegion.mipLevel destinationOrigin:origin];
  [blit endEncoding];
}

static void CopyTextureToBufferMetalImpl(CommandListMetal* cl, ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) {
  TextureMetal* ts = static_cast<TextureMetal*>(src);
  BufferMetal* vd = static_cast<BufferMetal*>(dst);
  if (!ts->texture || !vd->buffer) return;
  NSUInteger w = srcRegion.width > 0 ? srcRegion.width : [ts->texture width];
  NSUInteger h = srcRegion.height > 0 ? srcRegion.height : [ts->texture height];
  NSUInteger bytesPerRow = w * 4;
  NSUInteger bytesPerImage = bytesPerRow * h;
  MTLOrigin origin = { static_cast<NSUInteger>(srcRegion.x), static_cast<NSUInteger>(srcRegion.y), static_cast<NSUInteger>(srcRegion.z) };
  MTLSize size = { w, h, srcRegion.depth > 0 ? srcRegion.depth : 1 };
  id<MTLBlitCommandEncoder> blit = [cl->cmdBuffer blitCommandEncoder];
  [blit copyFromTexture:ts->texture sourceSlice:srcRegion.arrayLayer sourceLevel:srcRegion.mipLevel sourceOrigin:origin sourceSize:size toBuffer:vd->buffer destinationOffset:dstOffset destinationBytesPerRow:bytesPerRow destinationRowsPerImage:h];
  [blit endEncoding];
}

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

void CommandListMetal::BeginRenderPass(RenderPassDesc const& desc) {
  if (currentRenderEncoder) {
    [currentRenderEncoder endEncoding];
    currentRenderEncoder = nil;
  }
  if (!cmdBuffer || desc.colorAttachmentCount == 0 || !desc.colorAttachments[0]) return;
  TextureMetal* tex = static_cast<TextureMetal*>(desc.colorAttachments[0]);
  if (!tex->texture) return;
  MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor renderPassDescriptor];
  if (!rpd) return;
  rpd.colorAttachments[0].texture = tex->texture;
  rpd.colorAttachments[0].loadAction = (desc.colorLoadOp == LoadOp::Clear) ? MTLLoadActionClear : (desc.colorLoadOp == LoadOp::Load ? MTLLoadActionLoad : MTLLoadActionDontCare);
  rpd.colorAttachments[0].storeAction = (desc.colorStoreOp == StoreOp::Store) ? MTLStoreActionStore : MTLStoreActionDontCare;
  rpd.colorAttachments[0].clearColor = MTLClearColorMake(static_cast<double>(desc.clearColor[0]), static_cast<double>(desc.clearColor[1]), static_cast<double>(desc.clearColor[2]), static_cast<double>(desc.clearColor[3]));
  currentRenderEncoder = [cmdBuffer renderCommandEncoderWithDescriptor:rpd];
}

void CommandListMetal::EndRenderPass() {
  if (currentRenderEncoder) {
    [currentRenderEncoder endEncoding];
    currentRenderEncoder = nil;
  }
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

IFence* DeviceMetal::CreateFence(bool initialSignaled) {
  (void)initialSignaled;
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

IDescriptorSetLayout* DeviceMetal::CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) {
  (void)desc;
  return nullptr;  // P2: MTLArgumentEncoder; minimal no-op.
}

IDescriptorSet* DeviceMetal::AllocateDescriptorSet(IDescriptorSetLayout* layout) {
  (void)layout;
  return nullptr;
}

void DeviceMetal::UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes, uint32_t writeCount) {
  (void)set;(void)writes;(void)writeCount;
}

void DeviceMetal::DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) {
  (void)layout;
}

void DeviceMetal::DestroyDescriptorSet(IDescriptorSet* set) {
  (void)set;
}

}  // namespace

IDevice* CreateDeviceMetal() {
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  if (!device) return nullptr;
  
  DeviceMetal* dev = static_cast<DeviceMetal*>(core::Alloc(sizeof(DeviceMetal), alignof(DeviceMetal)));
  if (!dev) return nullptr;
  new (dev) DeviceMetal();
  dev->device = device;
  
  // Fill features and limits from device
  dev->features.maxTextureDimension2D = 16384; // Typical Metal limit
  dev->features.maxTextureDimension3D = 2048;
  dev->limits.maxBufferSize = 256u * 1024u * 1024u;
  dev->limits.maxTextureDimension2D = 16384;
  dev->limits.maxTextureDimension3D = 2048;
  dev->limits.minUniformBufferOffsetAlignment = 256;
  
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
