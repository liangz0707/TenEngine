/** @file device_d3d11.cpp
 *  D3D11 backend: CreateDeviceD3D11, DestroyDeviceD3D11, CommandList, Fence (T029).
 */
#if defined(TE_RHI_D3D11) && defined(_WIN32)

#include <te/rhi/backend_d3d11.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/queue.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/swapchain.hpp>
#include <te/rhi/sync.hpp>
#include <te/rhi/types.hpp>
#include <d3d11.h>
#include <cstddef>
#include <vector>
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>

namespace te {
namespace rhi {

namespace {

struct FenceD3D11 final : IFence {
  HANDLE event = nullptr;
  void Wait() override {
    if (event)
      WaitForSingleObject(event, INFINITE);
  }
  void Signal() override {
    if (event)
      SetEvent(event);
  }
  void Reset() override {
    if (event)
      ResetEvent(event);
  }
  ~FenceD3D11() override {
    if (event) {
      CloseHandle(event);
      event = nullptr;
    }
  }
};

struct CommandListD3D11 final : ICommandList {
  ID3D11DeviceContext* deferredCtx = nullptr;
  ID3D11CommandList* recordedList = nullptr;
  bool recording = false;

  void Begin() override {
    if (recordedList) {
      recordedList->Release();
      recordedList = nullptr;
    }
    recording = true;
  }
  void End() override {
    if (!deferredCtx || !recording) return;
    ID3D11CommandList* list = nullptr;
    if (SUCCEEDED(deferredCtx->FinishCommandList(FALSE, &list)))
      recordedList = list;
    recording = false;
  }
  void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override {
    if (deferredCtx && recording)
      deferredCtx->DrawInstanced(vertex_count, instance_count, first_vertex, first_instance);
  }
  void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) override {
    if (deferredCtx && recording)
      deferredCtx->DrawIndexedInstanced(index_count, instance_count, first_index, vertex_offset, first_instance);
  }
  void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) override {
    if (!deferredCtx || !viewports || count == 0) return;
    std::vector<D3D11_VIEWPORT> v(count);
    for (uint32_t i = 0; i < count; ++i) {
      v[i].TopLeftX = viewports[i].x;
      v[i].TopLeftY = viewports[i].y;
      v[i].Width = viewports[i].width;
      v[i].Height = viewports[i].height;
      v[i].MinDepth = viewports[i].minDepth;
      v[i].MaxDepth = viewports[i].maxDepth;
    }
    deferredCtx->RSSetViewports(count, v.data());
  }
  void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) override {
    if (!deferredCtx || !scissors || count == 0) return;
    std::vector<D3D11_RECT> r(count);
    for (uint32_t i = 0; i < count; ++i) {
      r[i].left = scissors[i].x;
      r[i].top = scissors[i].y;
      r[i].right = scissors[i].x + (LONG)scissors[i].width;
      r[i].bottom = scissors[i].y + (LONG)scissors[i].height;
    }
    deferredCtx->RSSetScissorRects(count, r.data());
  }
  void BeginRenderPass(RenderPassDesc const& desc) override { (void)desc; }
  void EndRenderPass() override {}
  void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) override {
    (void)src;(void)srcOffset;(void)dst;(void)dstOffset;(void)size;
  }
  void CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) override {
    (void)src;(void)srcOffset;(void)dst;(void)dstRegion;
  }
  void CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) override {
    (void)src;(void)srcRegion;(void)dst;(void)dstOffset;
  }
  /** Raytracing unsupported on D3D11 backend (no DXR). */
  void BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result) override {
    (void)desc;(void)scratch;(void)result;
  }
  void DispatchRays(DispatchRaysDesc const& desc) override { (void)desc; }
  void Dispatch(uint32_t x, uint32_t y, uint32_t z) override {
    if (deferredCtx && recording)
      deferredCtx->Dispatch(x, y, z);
  }
  void Copy(void const* src, void* dst, size_t size) override { (void)src;(void)dst;(void)size; }
  /** D3D11 has no explicit resource barrier API; documented no-op per spec. */
  void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers,
                       uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) override {
    (void)bufferBarrierCount;(void)bufferBarriers;(void)textureBarrierCount;(void)textureBarriers;
  }
  ~CommandListD3D11() override {
    if (recordedList) {
      recordedList->Release();
      recordedList = nullptr;
    }
    if (deferredCtx) {
      deferredCtx->Release();
      deferredCtx = nullptr;
    }
  }
};

struct BufferD3D11 final : IBuffer {
  ID3D11Buffer* buffer = nullptr;
  ~BufferD3D11() override { if (buffer) buffer->Release(); }
};

struct TextureD3D11 final : ITexture {
  ID3D11Texture2D* texture = nullptr;
  ~TextureD3D11() override { if (texture) texture->Release(); }
};

struct SamplerD3D11 final : ISampler {
  ID3D11SamplerState* sampler = nullptr;
  ~SamplerD3D11() override { if (sampler) sampler->Release(); }
};

struct SwapChainD3D11 final : ISwapChain {
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
  ~SwapChainD3D11() override {
    if (device && backBuffer) device->DestroyTexture(backBuffer);
  }
};

struct PSOD3D11 final : IPSO {
  ID3D11VertexShader* vs = nullptr;
  ID3D11PixelShader* ps = nullptr;
  ID3D11ComputeShader* cs = nullptr;
  ~PSOD3D11() override {
    if (vs) { vs->Release(); vs = nullptr; }
    if (ps) { ps->Release(); ps = nullptr; }
    if (cs) { cs->Release(); cs = nullptr; }
  }
};

struct QueueD3D11 final : IQueue {
  ID3D11DeviceContext* immediateContext = nullptr;

  void Submit(ICommandList* cmdList, IFence* signalFence,
              ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) override {
    (void)waitSemaphore;
    (void)signalSemaphore;
    if (!immediateContext || !cmdList) return;
    CommandListD3D11* d11 = static_cast<CommandListD3D11*>(cmdList);
    if (!d11->recordedList) return;
    immediateContext->ExecuteCommandList(d11->recordedList, FALSE);
    if (signalFence) {
      immediateContext->Flush();
      signalFence->Signal();
    }
  }
  void WaitIdle() override {
    if (immediateContext)
      immediateContext->Flush();
  }
  ~QueueD3D11() override = default;
};

struct DeviceD3D11 final : IDevice {
  ID3D11Device* device = nullptr;
  ID3D11DeviceContext* context = nullptr;
  QueueD3D11* queueWrapper = nullptr;
  DeviceFeatures features{};
  DeviceLimits limits{};

  Backend GetBackend() const override { return Backend::D3D11; }
  IQueue* GetQueue(QueueType type, uint32_t index) override {
    (void)type;
    (void)index;
    return queueWrapper;
  }
  DeviceFeatures const& GetFeatures() const override { return features; }
  DeviceLimits const& GetLimits() const override { return limits; }
  ICommandList* CreateCommandList() override {
    if (!device) return nullptr;
    ID3D11DeviceContext* deferredCtx = nullptr;
    if (FAILED(device->CreateDeferredContext(0, &deferredCtx)) || !deferredCtx)
      return nullptr;
    auto* cl = new CommandListD3D11();
    cl->deferredCtx = deferredCtx;
    return cl;
  }
  void DestroyCommandList(ICommandList* cmd) override {
    delete static_cast<CommandListD3D11*>(cmd);
  }
  IBuffer* CreateBuffer(BufferDesc const& desc) override {
    if (!device || desc.size == 0) return nullptr;
    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = (UINT)desc.size;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    if (desc.usage != 0)
      bd.BindFlags = (UINT)desc.usage;
    ID3D11Buffer* buf = nullptr;
    if (FAILED(device->CreateBuffer(&bd, nullptr, &buf)) || !buf)
      return nullptr;
    auto* b = new BufferD3D11();
    b->buffer = buf;
    return b;
  }
  ITexture* CreateTexture(TextureDesc const& desc) override {
    if (!device || desc.width == 0 || desc.height == 0) return nullptr;
    DXGI_FORMAT fmt = (desc.format == 0) ? DXGI_FORMAT_R8G8B8A8_UNORM : (DXGI_FORMAT)desc.format;
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = desc.width;
    td.Height = desc.height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = fmt;
    td.SampleDesc = { 1, 0 };
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    td.CPUAccessFlags = 0;
    td.MiscFlags = 0;
    ID3D11Texture2D* tex = nullptr;
    if (FAILED(device->CreateTexture2D(&td, nullptr, &tex)) || !tex)
      return nullptr;
    auto* t = new TextureD3D11();
    t->texture = tex;
    return t;
  }
  ISampler* CreateSampler(SamplerDesc const& desc) override {
    if (!device) return nullptr;
    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = (desc.filter == 0) ? D3D11_FILTER_MIN_MAG_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.MipLODBias = 0.f;
    sd.MaxAnisotropy = 1;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.BorderColor[0] = sd.BorderColor[1] = sd.BorderColor[2] = sd.BorderColor[3] = 1.f;
    sd.MinLOD = 0.f;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    ID3D11SamplerState* samp = nullptr;
    if (FAILED(device->CreateSamplerState(&sd, &samp)) || !samp)
      return nullptr;
    auto* s = new SamplerD3D11();
    s->sampler = samp;
    return s;
  }
  ViewHandle CreateView(ViewDesc const& desc) override { (void)desc; return 0; }
  void DestroyBuffer(IBuffer* b) override { delete static_cast<BufferD3D11*>(b); }
  void DestroyTexture(ITexture* t) override { delete static_cast<TextureD3D11*>(t); }
  void DestroySampler(ISampler* s) override { delete static_cast<SamplerD3D11*>(s); }
  IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc) override {
    if (!device) return nullptr;
    if ((!desc.vertex_shader || desc.vertex_shader_size == 0) && (!desc.fragment_shader || desc.fragment_shader_size == 0))
      return nullptr;
    auto* p = new PSOD3D11();
    if (desc.vertex_shader && desc.vertex_shader_size > 0)
      device->CreateVertexShader(desc.vertex_shader, desc.vertex_shader_size, nullptr, &p->vs);
    if (desc.fragment_shader && desc.fragment_shader_size > 0)
      device->CreatePixelShader(desc.fragment_shader, desc.fragment_shader_size, nullptr, &p->ps);
    return p;
  }
  IPSO* CreateComputePSO(ComputePSODesc const& desc) override {
    if (!device) return nullptr;
    if (!desc.compute_shader || desc.compute_shader_size == 0) return nullptr;
    auto* p = new PSOD3D11();
    if (FAILED(device->CreateComputeShader(desc.compute_shader, desc.compute_shader_size, nullptr, &p->cs))) {
      delete p;
      return nullptr;
    }
    return p;
  }
  void SetShader(IPSO* pso, void const* data, size_t size) override { (void)pso; (void)data; (void)size; }
  void Cache(IPSO* pso) override { (void)pso; }
  void DestroyPSO(IPSO* pso) override { delete static_cast<PSOD3D11*>(pso); }
  IFence* CreateFence(bool initialSignaled) override {
    HANDLE ev = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    if (!ev) return nullptr;
    if (initialSignaled)
      SetEvent(ev);
    auto* f = new FenceD3D11();
    f->event = ev;
    return f;
  }
  ISemaphore* CreateSemaphore() override { return nullptr; }
  void DestroyFence(IFence* f) override { delete static_cast<FenceD3D11*>(f); }
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
    auto* sc = new SwapChainD3D11();
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
  ~DeviceD3D11() override {
    delete queueWrapper;
    queueWrapper = nullptr;
    if (context) { context->Release(); context = nullptr; }
    if (device) { device->Release(); device = nullptr; }
  }
};

}  // namespace

IDevice* CreateDeviceD3D11() {
  D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
  ID3D11Device* dev = nullptr;
  ID3D11DeviceContext* ctx = nullptr;
  if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                levels, 2, D3D11_SDK_VERSION, &dev, nullptr, &ctx)))
    return nullptr;
  auto* d = new DeviceD3D11();
  d->device = dev;
  d->context = ctx;
  d->queueWrapper = new QueueD3D11();
  d->queueWrapper->immediateContext = ctx;
  d->limits.maxBufferSize = 256 * 1024 * 1024ull;
  d->limits.maxTextureDimension2D = 16384u;
  d->limits.maxTextureDimension3D = 2048u;
  d->limits.minUniformBufferOffsetAlignment = 256;
  d->features.maxTextureDimension2D = 16384u;
  d->features.maxTextureDimension3D = 2048u;
  return d;
}

void DestroyDeviceD3D11(IDevice* device) {
  delete static_cast<DeviceD3D11*>(device);
}

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_D3D11 && _WIN32
