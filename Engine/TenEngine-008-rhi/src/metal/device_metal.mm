/** @file device_metal.mm
 *  Metal backend: CreateDeviceMetal, DestroyDeviceMetal, CommandList, Fence (T030).
 */
#if defined(TE_RHI_METAL) && defined(__APPLE__)

#include <te/rhi/backend_metal.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/queue.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/swapchain.hpp>
#include <te/rhi/sync.hpp>
#include <te/rhi/types.hpp>
#include <Metal/Metal.h>
#include <Foundation/Foundation.h>
#include <dispatch/dispatch.h>
#include <cstddef>
#include <cstring>

namespace te {
namespace rhi {

namespace {

struct FenceMetal final : IFence {
  dispatch_semaphore_t semaphore = nullptr;
  void Wait() override {
    if (semaphore)
      dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
  }
  void Signal() override {
    if (semaphore)
      dispatch_semaphore_signal(semaphore);
  }
  void Reset() override { /* dispatch_semaphore has no reset; no-op */ }
  ~FenceMetal() override {
    if (semaphore) {
      dispatch_release(semaphore);
      semaphore = nullptr;
    }
  }
};

struct BufferMetal final : IBuffer {
  id<MTLBuffer> buffer = nil;
  ~BufferMetal() override { if (buffer) [buffer release]; }
};

struct CommandListMetal final : ICommandList {
  id<MTLDevice> device = nil;
  id<MTLCommandQueue> queue = nil;
  id<MTLCommandBuffer> buffer = nil;
  id<MTLRenderCommandEncoder> renderEncoder = nil;
  id<MTLComputeCommandEncoder> computeEncoder = nil;
  id<MTLBlitCommandEncoder> blitEncoder = nil;
  id<MTLTexture> dummyTexture = nil;
  id<MTLBuffer> dummyIndexBuffer = nil;
  bool recording = false;
  MTLViewport lastViewport = {0, 0, 0, 0, 0, 1};
  MTLScissorRect lastScissor = {0, 0, 0, 0};
  bool viewportSet = false;
  bool scissorSet = false;
  static constexpr uint32_t kMaxVertexBufferSlots = 8u;
  id<MTLBuffer> boundVertexBuffers[kMaxVertexBufferSlots] = {nil};
  NSUInteger boundVertexOffsets[kMaxVertexBufferSlots] = {0};
  uint32_t boundVertexStrides[kMaxVertexBufferSlots] = {0};
  id<MTLBuffer> boundIndexBuffer = nil;
  NSUInteger boundIndexOffset = 0;
  MTLIndexType boundIndexType = MTLIndexTypeUInt16;
  id<MTLRenderPipelineState> boundGraphicsPSO = nil;

  void ensureRenderEncoder() {
    if (renderEncoder || !buffer || !device || !queue) return;
    if (!dummyTexture) {
      MTLTextureDescriptor* td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR8Unorm width:1 height:1 mipmapped:NO];
      td.usage = MTLTextureUsageRenderTarget;
      dummyTexture = [device newTextureWithDescriptor:td];
      if (!dummyTexture) return;
    }
    MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor renderPassDescriptor];
    rpd.colorAttachments[0].texture = dummyTexture;
    rpd.colorAttachments[0].loadAction = MTLLoadActionDontCare;
    rpd.colorAttachments[0].storeAction = MTLStoreActionDontCare;
    renderEncoder = [buffer renderCommandEncoderWithDescriptor:rpd];
    if (renderEncoder) {
      if (boundGraphicsPSO) [renderEncoder setRenderPipelineState:boundGraphicsPSO];
      if (viewportSet) [renderEncoder setViewport:lastViewport];
      if (scissorSet) [renderEncoder setScissorRect:lastScissor];
      for (uint32_t i = 0; i < kMaxVertexBufferSlots; ++i) {
        if (boundVertexBuffers[i])
          [renderEncoder setVertexBuffer:boundVertexBuffers[i] offset:boundVertexOffsets[i] atIndex:i];
      }
    }
  }

  void endRenderEncoder() {
    if (renderEncoder) {
      [renderEncoder endEncoding];
      renderEncoder = nil;
    }
  }

  void endComputeEncoder() {
    if (computeEncoder) {
      [computeEncoder endEncoding];
      computeEncoder = nil;
    }
  }

  void endBlitEncoder() {
    if (blitEncoder) {
      [blitEncoder endEncoding];
      blitEncoder = nil;
    }
  }

  void Begin() override {
    if (!queue) return;
    endRenderEncoder();
    endComputeEncoder();
    endBlitEncoder();
    buffer = [queue commandBuffer];
    recording = true;
    viewportSet = false;
    scissorSet = false;
    for (uint32_t i = 0; i < kMaxVertexBufferSlots; ++i) {
      boundVertexBuffers[i] = nil;
      boundVertexOffsets[i] = 0;
      boundVertexStrides[i] = 0;
    }
    boundIndexBuffer = nil;
    boundIndexOffset = 0;
    boundIndexType = MTLIndexTypeUInt16;
    boundGraphicsPSO = nil;
  }

  void End() override {
    endRenderEncoder();
    endComputeEncoder();
    endBlitEncoder();
    recording = false;
  }

  void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override {
    if (!recording || !buffer) return;
    ensureRenderEncoder();
    if (!renderEncoder) return;
    [renderEncoder setVertexBuffer:nil offset:0 atIndex:0];
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:first_vertex vertexCount:vertex_count instanceCount:instance_count baseInstance:first_instance];
  }

  void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) override {
    if (!recording || !buffer || !device) return;
    ensureRenderEncoder();
    if (!renderEncoder) return;
    id<MTLBuffer> idxBuf = boundIndexBuffer;
    NSUInteger idxOff = boundIndexOffset + (first_index * (boundIndexType == MTLIndexTypeUInt32 ? 4 : 2));
    if (!idxBuf) {
      if (!dummyIndexBuffer) {
        NSUInteger len = 6;
        dummyIndexBuffer = [device newBufferWithLength:len options:MTLResourceStorageModeShared];
        if (!dummyIndexBuffer) return;
      }
      idxBuf = dummyIndexBuffer;
      idxOff = 0;
    }
    for (uint32_t i = 0; i < kMaxVertexBufferSlots; ++i) {
      if (boundVertexBuffers[i])
        [renderEncoder setVertexBuffer:boundVertexBuffers[i] offset:boundVertexOffsets[i] atIndex:i];
    }
    [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:index_count indexType:(idxBuf == dummyIndexBuffer ? MTLIndexTypeUInt16 : boundIndexType) indexBuffer:idxBuf indexBufferOffset:idxOff instanceCount:instance_count baseVertex:vertex_offset baseInstance:first_instance];
  }

  void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) override {
    if (!viewports || count == 0) return;
    lastViewport.originX = viewports[0].x;
    lastViewport.originY = viewports[0].y;
    lastViewport.width = viewports[0].width;
    lastViewport.height = viewports[0].height;
    lastViewport.znear = viewports[0].minDepth;
    lastViewport.zfar = viewports[0].maxDepth;
    viewportSet = true;
    if (renderEncoder)
      [renderEncoder setViewport:lastViewport];
  }

  void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) override {
    if (!scissors || count == 0) return;
    lastScissor.x = scissors[0].x;
    lastScissor.y = scissors[0].y;
    lastScissor.width = scissors[0].width;
    lastScissor.height = scissors[0].height;
    scissorSet = true;
    if (renderEncoder)
      [renderEncoder setScissorRect:lastScissor];
  }

  void SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) override {
    if (!recording || !buffer) return;
    BufferMetal* b = static_cast<BufferMetal*>(buffer);
    id<MTLBuffer> mb = b ? b->buffer : nil;
    NSUInteger off = (NSUInteger)offset;
    if (renderEncoder)
      [renderEncoder setVertexBuffer:mb offset:off atIndex:slot];
    if (renderEncoder)
      [renderEncoder setFragmentBuffer:mb offset:off atIndex:slot];
    if (computeEncoder)
      [computeEncoder setBuffer:mb offset:off atIndex:slot];
  }

  void SetVertexBuffer(uint32_t slot, IBuffer* buffer, size_t offset, uint32_t stride) override {
    if (slot >= kMaxVertexBufferSlots) return;
    if (!buffer) {
      boundVertexBuffers[slot] = nil;
      boundVertexOffsets[slot] = 0;
      boundVertexStrides[slot] = 0;
      if (renderEncoder)
        [renderEncoder setVertexBuffer:nil offset:0 atIndex:slot];
      return;
    }
    BufferMetal* b = static_cast<BufferMetal*>(buffer);
    boundVertexBuffers[slot] = b ? b->buffer : nil;
    boundVertexOffsets[slot] = (NSUInteger)offset;
    boundVertexStrides[slot] = stride;
    if (renderEncoder && boundVertexBuffers[slot])
      [renderEncoder setVertexBuffer:boundVertexBuffers[slot] offset:boundVertexOffsets[slot] atIndex:slot];
  }
  void SetIndexBuffer(IBuffer* buffer, size_t offset, uint32_t indexFormat) override {
    if (!buffer) {
      boundIndexBuffer = nil;
      boundIndexOffset = 0;
      boundIndexType = MTLIndexTypeUInt16;
      return;
    }
    BufferMetal* b = static_cast<BufferMetal*>(buffer);
    boundIndexBuffer = b ? b->buffer : nil;
    boundIndexOffset = (NSUInteger)offset;
    boundIndexType = (indexFormat == 1) ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
  }

  void SetGraphicsPSO(IPSO* pso) override {
    boundGraphicsPSO = (pso && static_cast<PSOMetal*>(pso)->renderPipeline) ? static_cast<PSOMetal*>(pso)->renderPipeline : nil;
    if (renderEncoder && boundGraphicsPSO)
      [renderEncoder setRenderPipelineState:boundGraphicsPSO];
  }

  void BeginRenderPass(RenderPassDesc const& desc) override { (void)desc; }
  void EndRenderPass() override {}
  void BeginOcclusionQuery(uint32_t queryIndex) override { (void)queryIndex; }
  void EndOcclusionQuery(uint32_t queryIndex) override { (void)queryIndex; }
  void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) override {
    (void)src;(void)srcOffset;(void)dst;(void)dstOffset;(void)size;
  }
  void CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) override {
    (void)src;(void)srcOffset;(void)dst;(void)dstRegion;
  }
  void CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) override {
    (void)src;(void)srcRegion;(void)dst;(void)dstOffset;
  }
  /** Raytracing unsupported on Metal backend (explicit no-op). */
  void BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result) override {
    (void)desc;(void)scratch;(void)result;
  }
  void DispatchRays(DispatchRaysDesc const& desc) override { (void)desc; }

  void Dispatch(uint32_t x, uint32_t y, uint32_t z) override {
    if (!recording || !buffer || !queue) return;
    endRenderEncoder();
    if (!computeEncoder)
      computeEncoder = [buffer computeCommandEncoder];
    if (!computeEncoder) return;
    [computeEncoder dispatchThreadgroups:MTLSizeMake(x, y, z) threadsPerThreadgroup:MTLSizeMake(1, 1, 1)];
  }

  void Copy(void const* src, void* dst, size_t size) override { (void)src;(void)dst;(void)size; }

  void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers,
                       uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) override {
    (void)bufferBarrierCount;(void)bufferBarriers;(void)textureBarrierCount;(void)textureBarriers;
    if (!recording || !buffer) return;
    endRenderEncoder();
    endComputeEncoder();
    if (!blitEncoder)
      blitEncoder = [buffer blitCommandEncoder];
    if (blitEncoder)
      [blitEncoder memoryBarrierWithScope:MTLBarrierScopeBuffers afterStages:MTLRenderStageFragment beforeStages:MTLRenderStageFragment];
  }

  ~CommandListMetal() override {
    endRenderEncoder();
    endComputeEncoder();
    endBlitEncoder();
    if (dummyTexture) { [dummyTexture release]; dummyTexture = nil; }
    if (dummyIndexBuffer) { [dummyIndexBuffer release]; dummyIndexBuffer = nil; }
    if (buffer) { buffer = nil; }
  }
};

struct TextureMetal final : ITexture {
  id<MTLTexture> texture = nil;
  ~TextureMetal() override { if (texture) [texture release]; }
};

struct SamplerMetal final : ISampler {
  id<MTLSamplerState> sampler = nil;
  ~SamplerMetal() override { if (sampler) [sampler release]; }
};

struct SwapChainMetal final : ISwapChain {
  IDevice* device = nullptr;
  ITexture* backBuffer = nullptr;
  uint32_t width = 0;
  uint32_t height = 0;
  bool Present() override { return true; }
  ITexture* GetCurrentBackBuffer() override { return backBuffer; }
  uint32_t GetCurrentBackBufferIndex() const override { return 0; }
  void Resize(uint32_t w, uint32_t h) override { width = w; height = h; }
  uint32_t GetWidth() const override { return width; }
  uint32_t GetHeight() const override { return height; }
  ~SwapChainMetal() override {
    if (device && backBuffer) device->DestroyTexture(backBuffer);
  }
};

struct PSOMetal final : IPSO {
  id<MTLLibrary> library = nil;
  id<MTLRenderPipelineState> renderPipeline = nil;
  id<MTLComputePipelineState> computePipeline = nil;
  ~PSOMetal() override {
    if (library) [library release];
    if (renderPipeline) [renderPipeline release];
    if (computePipeline) [computePipeline release];
  }
};

struct QueueMetal final : IQueue {
  id<MTLCommandQueue> queue = nil;

  void Submit(ICommandList* cmdList, IFence* signalFence,
              ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) override {
    (void)waitSemaphore;
    (void)signalSemaphore;
    if (!queue || !cmdList) return;
    CommandListMetal* metal = static_cast<CommandListMetal*>(cmdList);
    if (!metal->buffer) return;
    __block IFence* fence = signalFence;
    if (fence) {
      [metal->buffer addCompletedHandler:^(id<MTLCommandBuffer>) {
        if (fence) fence->Signal();
      }];
    }
    [metal->buffer commit];
  }

  void WaitIdle() override {
    if (queue) {
      id<MTLCommandBuffer> buf = [queue commandBuffer];
      [buf commit];
      [buf waitUntilCompleted];
    }
  }

  ~QueueMetal() override = default;
};

struct DeviceMetal final : IDevice {
  id<MTLDevice> device = nil;
  id<MTLCommandQueue> queue = nil;
  QueueMetal* queueWrapper = nullptr;
  DeviceFeatures features{};
  DeviceLimits limits{};

  Backend GetBackend() const override { return Backend::Metal; }
  IQueue* GetQueue(QueueType type, uint32_t index) override {
    (void)type;
    (void)index;
    return queueWrapper;
  }
  DeviceFeatures const& GetFeatures() const override { return features; }
  DeviceLimits const& GetLimits() const override { return limits; }

  ICommandList* CreateCommandList() override {
    if (!device || !queue) return nullptr;
    auto* cl = new CommandListMetal();
    cl->device = device;
    cl->queue = queue;
    return cl;
  }

  void DestroyCommandList(ICommandList* cmd) override {
    delete static_cast<CommandListMetal*>(cmd);
  }

  void UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size) override {
    if (!device || !buf || !data || size == 0) return;
    BufferMetal* b = static_cast<BufferMetal*>(buf);
    if (!b->buffer) return;
    void* ptr = [b->buffer contents];
    if (!ptr) return;
    std::memcpy(static_cast<char*>(ptr) + offset, data, size);
  }

  IBuffer* CreateBuffer(BufferDesc const& desc) override {
    if (!device || desc.size == 0) return nullptr;
    /* Uniform / CPU-writable: use Shared so UpdateBuffer can write via contents. */
    bool wantHostVisible = (desc.usage & static_cast<uint32_t>(BufferUsage::Uniform)) != 0;
    MTLResourceOptions opts = wantHostVisible ? MTLResourceStorageModeShared : MTLResourceStorageModeShared;
    id<MTLBuffer> buf = [device newBufferWithLength:desc.size options:opts];
    if (!buf) return nullptr;
    auto* b = new BufferMetal();
    b->buffer = buf;
    return b;
  }
  ITexture* CreateTexture(TextureDesc const& desc) override {
    if (!device || desc.width == 0 || desc.height == 0) return nullptr;
    MTLPixelFormat fmt = (desc.format == 0) ? MTLPixelFormatRGBA8Unorm : (MTLPixelFormat)desc.format;
    MTLTextureDescriptor* td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:fmt width:desc.width height:desc.height mipmapped:NO];
    td.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
    id<MTLTexture> tex = [device newTextureWithDescriptor:td];
    if (!tex) return nullptr;
    auto* t = new TextureMetal();
    t->texture = tex;
    return t;
  }
  ISampler* CreateSampler(SamplerDesc const& desc) override {
    if (!device) return nullptr;
    MTLSamplerDescriptor* sd = [MTLSamplerDescriptor new];
    sd.minFilter = (desc.filter == 0) ? MTLSamplerMinMagFilterNearest : MTLSamplerMinMagFilterLinear;
    sd.magFilter = sd.minFilter;
    sd.mipFilter = MTLSamplerMipFilterNearest;
    sd.sAddressMode = MTLSamplerAddressModeRepeat;
    sd.tAddressMode = MTLSamplerAddressModeRepeat;
    sd.rAddressMode = MTLSamplerAddressModeRepeat;
    id<MTLSamplerState> samp = [device newSamplerStateWithDescriptor:sd];
    [sd release];
    if (!samp) return nullptr;
    auto* s = new SamplerMetal();
    s->sampler = samp;
    return s;
  }
  ViewHandle CreateView(ViewDesc const& desc) override { (void)desc; return 0; }
  void DestroyBuffer(IBuffer* b) override { delete static_cast<BufferMetal*>(b); }
  void DestroyTexture(ITexture* t) override { delete static_cast<TextureMetal*>(t); }
  void DestroySampler(ISampler* s) override { delete static_cast<SamplerMetal*>(s); }
  IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc) override {
    if (!device) return nullptr;
    if ((!desc.vertex_shader || desc.vertex_shader_size == 0) && (!desc.fragment_shader || desc.fragment_shader_size == 0))
      return nullptr;
    auto* p = new PSOMetal();
    NSData* data = nil;
    if (desc.vertex_shader && desc.vertex_shader_size > 0)
      data = [NSData dataWithBytes:desc.vertex_shader length:desc.vertex_shader_size];
    else if (desc.fragment_shader && desc.fragment_shader_size > 0)
      data = [NSData dataWithBytes:desc.fragment_shader length:desc.fragment_shader_size];
    if (data)
      p->library = [device newLibraryWithData:data error:nil];
    return p;
  }
  IPSO* CreateComputePSO(ComputePSODesc const& desc) override {
    if (!device) return nullptr;
    if (!desc.compute_shader || desc.compute_shader_size == 0) return nullptr;
    NSData* data = [NSData dataWithBytes:desc.compute_shader length:desc.compute_shader_size];
    if (!data) return nullptr;
    id<MTLLibrary> lib = [device newLibraryWithData:data error:nil];
    if (!lib) return nullptr;
    auto* p = new PSOMetal();
    p->library = lib;
    return p;
  }
  void SetShader(IPSO* pso, void const* data, size_t size) override { (void)pso; (void)data; (void)size; }
  void Cache(IPSO* pso) override { (void)pso; }
  void DestroyPSO(IPSO* pso) override { delete static_cast<PSOMetal*>(pso); }

  IFence* CreateFence(bool initialSignaled) override {
    long value = initialSignaled ? 1 : 0;
    dispatch_semaphore_t s = dispatch_semaphore_create(value);
    if (!s) return nullptr;
    auto* f = new FenceMetal();
    f->semaphore = s;
    return f;
  }

  ISemaphore* CreateSemaphore() override { return nullptr; }
  void DestroyFence(IFence* f) override { delete static_cast<FenceMetal*>(f); }
  void DestroySemaphore(ISemaphore* s) override { (void)s; }
  ISwapChain* CreateSwapChain(SwapChainDesc const& desc) override {
    if (!device || desc.width == 0 || desc.height == 0) return nullptr;
    TextureDesc td = {};
    td.width = desc.width;
    td.height = desc.height;
    td.depth = 1;
    td.format = desc.format;
    ITexture* tex = CreateTexture(td);
    if (!tex) return nullptr;
    auto* sc = new SwapChainMetal();
    sc->device = this;
    sc->backBuffer = tex;
    sc->width = desc.width;
    sc->height = desc.height;
    return sc;
  }
  IDescriptorSetLayout* CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) override { (void)desc; return nullptr; }
  IDescriptorSet* AllocateDescriptorSet(IDescriptorSetLayout* layout) override { (void)layout; return nullptr; }
  void UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes, uint32_t writeCount) override { (void)set; (void)writes; (void)writeCount; }
  void DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) override { (void)layout; }
  void DestroyDescriptorSet(IDescriptorSet* set) override { (void)set; }

  ~DeviceMetal() override {
    delete queueWrapper;
    queueWrapper = nullptr;
    if (queue) { [queue release]; queue = nil; }
    if (device) { [device release]; device = nil; }
  }
};

}  // namespace

IDevice* CreateDeviceMetal() {
  id<MTLDevice> dev = MTLCreateSystemDefaultDevice();
  if (!dev) return nullptr;
  id<MTLCommandQueue> q = [dev newCommandQueue];
  if (!q) {
    [dev release];
    return nullptr;
  }
  auto* d = new DeviceMetal();
  d->device = dev;
  d->queue = q;
  d->queueWrapper = new QueueMetal();
  d->queueWrapper->queue = q;
  d->limits.maxBufferSize = 256 * 1024 * 1024ull;
  d->limits.maxTextureDimension2D = 16384u;
  d->limits.maxTextureDimension3D = 2048u;
  d->limits.minUniformBufferOffsetAlignment = 256;
  d->features.maxTextureDimension2D = 16384u;
  d->features.maxTextureDimension3D = 2048u;
  return d;
}

void DestroyDeviceMetal(IDevice* device) {
  delete static_cast<DeviceMetal*>(device);
}

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_METAL && __APPLE__
