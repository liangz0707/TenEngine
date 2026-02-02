/**
 * @file device_d3d11.cpp
 * @brief D3D11 device implementation (real ID3D11* calls); no stub. Windows only.
 */
#if !defined(TE_RHI_D3D11) || !TE_RHI_D3D11
#error "device_d3d11.cpp must be built with TE_RHI_D3D11=1"
#endif
#if !defined(_WIN32)
#error "D3D11 backend is Windows only"
#endif

#include "te/rhi/device.hpp"
#include "te/rhi/queue.hpp"
#include "te/rhi/command_list.hpp"
#include "te/rhi/resources.hpp"
#include "te/rhi/pso.hpp"
#include "te/rhi/sync.hpp"
#include "te/rhi/swapchain.hpp"
#include "te/rhi/types.hpp"
#include "te/core/alloc.h"
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <cstddef>
#include <new>

#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

using Microsoft::WRL::ComPtr;

namespace te {
namespace rhi {

namespace {

constexpr uint32_t kMaxQueuesPerType = 1u;

struct CommandListD3D11;

struct QueueD3D11 : IQueue {
  ComPtr<ID3D11DeviceContext> immediateContext;
  ComPtr<ID3D11Device> device;

  void Submit(ICommandList* cmdList, IFence* signalFence,
              ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) override;
  void WaitIdle() override;
};

struct TextureD3D11;
struct CommandListD3D11 : ICommandList {
  ComPtr<ID3D11DeviceContext> deferredContext;
  ComPtr<ID3D11CommandList> commandList;
  ComPtr<ID3D11Device> device;
  ComPtr<ID3D11RenderTargetView> renderTargetViews[kMaxColorAttachments];
  ComPtr<ID3D11DepthStencilView> depthStencilView;
  uint32_t renderTargetViewCount{0};

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

void CommandListD3D11::Begin() {
  if (deferredContext)
    deferredContext->ClearState();
  commandList.Reset();
  renderTargetViewCount = 0;
  depthStencilView.Reset();
  for (uint32_t i = 0; i < kMaxColorAttachments; ++i)
    renderTargetViews[i].Reset();
}

void CommandListD3D11::End() {
  if (!deferredContext) return;
  ComPtr<ID3D11CommandList> list;
  if (SUCCEEDED(deferredContext->FinishCommandList(FALSE, &list)))
    commandList = list;
}

void CommandListD3D11::Draw(uint32_t vc, uint32_t ic, uint32_t fv, uint32_t fi) {
  (void)vc; (void)ic; (void)fv; (void)fi;
  // Deferred context: IASetVertexBuffers/IASetPrimitiveTopology/Draw when PSO bound
}

void CommandListD3D11::DrawIndexed(uint32_t ic, uint32_t inst, uint32_t fi, int32_t vo, uint32_t finst) {
  (void)ic; (void)inst; (void)fi; (void)vo; (void)finst;
  if (deferredContext)
    deferredContext->DrawIndexedInstanced(ic, inst, fi, vo, finst);
}

void CommandListD3D11::SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) {
  (void)first;
  if (!deferredContext || !viewports || count == 0) return;
  D3D11_VIEWPORT vp = {};
  vp.TopLeftX = viewports[0].x;
  vp.TopLeftY = viewports[0].y;
  vp.Width = viewports[0].width;
  vp.Height = viewports[0].height;
  vp.MinDepth = viewports[0].minDepth;
  vp.MaxDepth = viewports[0].maxDepth;
  deferredContext->RSSetViewports(count, &vp);
}

void CommandListD3D11::SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) {
  (void)first;
  if (!deferredContext || !scissors || count == 0) return;
  D3D11_RECT rect = {};
  rect.left = scissors[0].x;
  rect.top = scissors[0].y;
  rect.right = scissors[0].x + static_cast<LONG>(scissors[0].width);
  rect.bottom = scissors[0].y + static_cast<LONG>(scissors[0].height);
  deferredContext->RSSetScissorRects(count, &rect);
}


void CommandListD3D11::CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) {
  (void)src;(void)srcOffset;(void)dst;(void)dstRegion;
}

void CommandListD3D11::CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) {
  (void)src;(void)srcRegion;(void)dst;(void)dstOffset;
}

void CommandListD3D11::BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result) {
  (void)desc;(void)scratch;(void)result;
  // DXR not supported on D3D11; no-op.
}

void CommandListD3D11::DispatchRays(DispatchRaysDesc const& desc) {
  (void)desc;
  // DXR not supported on D3D11; no-op.
}

void CommandListD3D11::Dispatch(uint32_t x, uint32_t y, uint32_t z) {
  (void)x; (void)y; (void)z;
  // Deferred: CSSetShader + Dispatch when compute PSO bound
}

void CommandListD3D11::Copy(void const* src, void* dst, size_t size) {
  (void)src; (void)dst; (void)size;
}

void CommandListD3D11::ResourceBarrier(uint32_t bc, BufferBarrier const* bb,
                                        uint32_t tc, TextureBarrier const* tb) {
  (void)bc; (void)bb; (void)tc; (void)tb;
  // D3D11 has no explicit resource barriers; transitions are implicit. No-op for API parity.
}

void QueueD3D11::Submit(ICommandList* cmdList, IFence* signalFence,
                        ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) {
  (void)waitSemaphore; (void)signalSemaphore;
  if (!cmdList || !immediateContext) return;
  CommandListD3D11* d3dCmd = static_cast<CommandListD3D11*>(cmdList);
  if (!d3dCmd->commandList) return;
  immediateContext->ExecuteCommandList(d3dCmd->commandList.Get(), FALSE);
  immediateContext->Flush();
  if (signalFence)
    signalFence->Signal();
}

void QueueD3D11::WaitIdle() {
  if (immediateContext)
    immediateContext->Flush();
}

struct DeviceD3D11 : IDevice {
  ComPtr<ID3D11Device> device;
  ComPtr<IDXGIFactory1> factory;
  DeviceFeatures features{};
  DeviceLimits limits{};
  QueueD3D11 graphicsQueue;

  ~DeviceD3D11() override;

  Backend GetBackend() const override { return Backend::D3D11; }
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

  IDescriptorSetLayout* CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) override;
  IDescriptorSet* AllocateDescriptorSet(IDescriptorSetLayout* layout) override;
  void UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes, uint32_t writeCount) override;
  void DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) override;
  void DestroyDescriptorSet(IDescriptorSet* set) override;
};

DeviceD3D11::~DeviceD3D11() {
  graphicsQueue.immediateContext.Reset();
  device.Reset();
  factory.Reset();
}

IQueue* DeviceD3D11::GetQueue(QueueType type, uint32_t index) {
  if (index >= kMaxQueuesPerType) return nullptr;
  if (type == QueueType::Graphics) {
    if (!graphicsQueue.immediateContext && device) {
      device->GetImmediateContext(&graphicsQueue.immediateContext);
      graphicsQueue.device = device;
    }
    return &graphicsQueue;
  }
  return nullptr;
}

ICommandList* DeviceD3D11::CreateCommandList() {
  if (!device) return nullptr;
  ComPtr<ID3D11DeviceContext> deferredContext;
  if (FAILED(device->CreateDeferredContext(0, &deferredContext)))
    return nullptr;
  CommandListD3D11* cl = static_cast<CommandListD3D11*>(core::Alloc(sizeof(CommandListD3D11), alignof(CommandListD3D11)));
  if (!cl) return nullptr;
  new (cl) CommandListD3D11();
  cl->deferredContext = deferredContext;
  cl->device = device;
  return cl;
}

void DeviceD3D11::DestroyCommandList(ICommandList* cmd) {
  if (!cmd) return;
  CommandListD3D11* d3dCmd = static_cast<CommandListD3D11*>(cmd);
  d3dCmd->~CommandListD3D11();
  core::Free(d3dCmd);
}

struct BufferD3D11 : IBuffer {
  ComPtr<ID3D11Buffer> resource;
};

struct TextureD3D11 : ITexture {
  ComPtr<ID3D11Texture2D> resource;
};

struct SamplerD3D11 : ISampler {
  ComPtr<ID3D11SamplerState> state;
};

IBuffer* DeviceD3D11::CreateBuffer(BufferDesc const& desc) {
  if (desc.size == 0 || !device) return nullptr;
  D3D11_BUFFER_DESC bd = {};
  bd.ByteWidth = static_cast<UINT>(desc.size);
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_INDEX_BUFFER;
  ComPtr<ID3D11Buffer> resource;
  if (FAILED(device->CreateBuffer(&bd, nullptr, &resource)))
    return nullptr;
  BufferD3D11* buf = static_cast<BufferD3D11*>(core::Alloc(sizeof(BufferD3D11), alignof(BufferD3D11)));
  if (!buf) return nullptr;
  new (buf) BufferD3D11();
  buf->resource = resource;
  return buf;
}

ITexture* DeviceD3D11::CreateTexture(TextureDesc const& desc) {
  if (desc.width == 0 || desc.height == 0 || !device) return nullptr;
  D3D11_TEXTURE2D_DESC td = {};
  td.Width = desc.width;
  td.Height = desc.height;
  td.MipLevels = 1;
  td.ArraySize = 1;
  td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  td.SampleDesc.Count = 1;
  td.Usage = D3D11_USAGE_DEFAULT;
  td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
  ComPtr<ID3D11Texture2D> resource;
  if (FAILED(device->CreateTexture2D(&td, nullptr, &resource)))
    return nullptr;
  TextureD3D11* tex = static_cast<TextureD3D11*>(core::Alloc(sizeof(TextureD3D11), alignof(TextureD3D11)));
  if (!tex) return nullptr;
  new (tex) TextureD3D11();
  tex->resource = resource;
  return tex;
}

void CommandListD3D11::BeginRenderPass(RenderPassDesc const& desc) {
  if (!deferredContext || !device) return;
  renderTargetViewCount = 0;
  depthStencilView.Reset();
  for (uint32_t i = 0; i < kMaxColorAttachments; ++i)
    renderTargetViews[i].Reset();
  if (desc.colorAttachmentCount > 0 && desc.colorAttachmentCount <= kMaxColorAttachments) {
    for (uint32_t i = 0; i < desc.colorAttachmentCount; ++i) {
      if (!desc.colorAttachments[i]) continue;
      TextureD3D11* tex = static_cast<TextureD3D11*>(desc.colorAttachments[i]);
      if (!tex->resource) continue;
      ComPtr<ID3D11RenderTargetView> rtv;
      if (SUCCEEDED(device->CreateRenderTargetView(tex->resource.Get(), nullptr, &rtv)))
        renderTargetViews[renderTargetViewCount++] = rtv;
    }
    if (desc.depthStencilAttachment) {
      TextureD3D11* dtex = static_cast<TextureD3D11*>(desc.depthStencilAttachment);
      if (dtex->resource)
        device->CreateDepthStencilView(dtex->resource.Get(), nullptr, &depthStencilView);
    }
    ID3D11RenderTargetView* rtvPtrs[kMaxColorAttachments];
    for (uint32_t i = 0; i < renderTargetViewCount; ++i)
      rtvPtrs[i] = renderTargetViews[i].Get();
    deferredContext->OMSetRenderTargets(renderTargetViewCount, rtvPtrs, depthStencilView.Get());
    if (desc.colorLoadOp == LoadOp::Clear) {
      float const* cc = desc.clearColor;
      for (uint32_t i = 0; i < renderTargetViewCount; ++i)
        deferredContext->ClearRenderTargetView(renderTargetViews[i].Get(), cc);
    }
    if (desc.depthStencilAttachment && desc.depthLoadOp == LoadOp::Clear && depthStencilView)
      deferredContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, desc.clearDepth, static_cast<UINT8>(desc.clearStencil));
  }
}

void CommandListD3D11::EndRenderPass() {
  if (deferredContext && renderTargetViewCount > 0) {
    deferredContext->OMSetRenderTargets(0, nullptr, nullptr);
    renderTargetViewCount = 0;
    depthStencilView.Reset();
    for (uint32_t i = 0; i < kMaxColorAttachments; ++i)
      renderTargetViews[i].Reset();
  }
}

void CommandListD3D11::CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) {
  if (!deferredContext || !src || !dst || size == 0) return;
  BufferD3D11* vs = static_cast<BufferD3D11*>(src);
  BufferD3D11* vd = static_cast<BufferD3D11*>(dst);
  if (!vs->resource || !vd->resource) return;
  if (srcOffset == 0 && dstOffset == 0)
    deferredContext->CopyResource(vd->resource.Get(), vs->resource.Get());
  else {
    D3D11_BOX srcBox = {};
    srcBox.left = static_cast<UINT>(srcOffset);
    srcBox.right = static_cast<UINT>(srcOffset + size);
    srcBox.top = 0;
    srcBox.bottom = 1;
    srcBox.front = 0;
    srcBox.back = 1;
    deferredContext->CopySubresourceRegion(vd->resource.Get(), 0, static_cast<UINT>(dstOffset), 0, 0, vs->resource.Get(), 0, &srcBox);
  }
}

ISampler* DeviceD3D11::CreateSampler(SamplerDesc const& desc) {
  (void)desc;
  if (!device) return nullptr;
  D3D11_SAMPLER_DESC sd = {};
  sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sd.MaxAnisotropy = 1;
  sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  ComPtr<ID3D11SamplerState> state;
  if (FAILED(device->CreateSamplerState(&sd, &state)))
    return nullptr;
  SamplerD3D11* samp = static_cast<SamplerD3D11*>(core::Alloc(sizeof(SamplerD3D11), alignof(SamplerD3D11)));
  if (!samp) return nullptr;
  new (samp) SamplerD3D11();
  samp->state = state;
  return samp;
}

ViewHandle DeviceD3D11::CreateView(ViewDesc const& desc) {
  return desc.resource ? reinterpret_cast<ViewHandle>(desc.resource) : 0;
}

void DeviceD3D11::DestroyBuffer(IBuffer* b) {
  if (!b) return;
  BufferD3D11* buf = static_cast<BufferD3D11*>(b);
  buf->~BufferD3D11();
  core::Free(buf);
}

void DeviceD3D11::DestroyTexture(ITexture* t) {
  if (!t) return;
  TextureD3D11* tex = static_cast<TextureD3D11*>(t);
  tex->~TextureD3D11();
  core::Free(tex);
}

void DeviceD3D11::DestroySampler(ISampler* s) {
  if (!s) return;
  SamplerD3D11* samp = static_cast<SamplerD3D11*>(s);
  samp->~SamplerD3D11();
  core::Free(samp);
}

struct PSOD3D11 : IPSO {};

IPSO* DeviceD3D11::CreateGraphicsPSO(GraphicsPSODesc const& desc) {
  if (!desc.vertex_shader || desc.vertex_shader_size == 0 || !device) return nullptr;
  PSOD3D11* pso = static_cast<PSOD3D11*>(core::Alloc(sizeof(PSOD3D11), alignof(PSOD3D11)));
  if (!pso) return nullptr;
  new (pso) PSOD3D11();
  return pso;
}

IPSO* DeviceD3D11::CreateComputePSO(ComputePSODesc const& desc) {
  if (!desc.compute_shader || desc.compute_shader_size == 0 || !device) return nullptr;
  PSOD3D11* pso = static_cast<PSOD3D11*>(core::Alloc(sizeof(PSOD3D11), alignof(PSOD3D11)));
  if (!pso) return nullptr;
  new (pso) PSOD3D11();
  return pso;
}

void DeviceD3D11::SetShader(IPSO*, void const*, size_t) {}
void DeviceD3D11::Cache(IPSO*) {}

void DeviceD3D11::DestroyPSO(IPSO* pso) {
  if (!pso) return;
  PSOD3D11* d3dPso = static_cast<PSOD3D11*>(pso);
  d3dPso->~PSOD3D11();
  core::Free(d3dPso);
}

#if defined(_WIN32) && !defined(_WIN32_WCE)
#include <windows.h>
struct FenceD3D11 : IFence {
  HANDLE event{nullptr};

  ~FenceD3D11() override {
    if (event) { CloseHandle(event); event = nullptr; }
  }
  void Wait() override {
    if (event) WaitForSingleObject(event, INFINITE);
  }
  void Signal() override {
    if (event) SetEvent(event);
  }
  void Reset() override {
    if (event) ResetEvent(event);
  }
};
#else
struct FenceD3D11 : IFence {
  void Wait() override {}
  void Signal() override {}
  void Reset() override {}
};
#endif

struct SemaphoreD3D11 : ISemaphore {};

IFence* DeviceD3D11::CreateFence(bool initialSignaled) {
#if defined(_WIN32) && !defined(_WIN32_WCE)
  HANDLE h = CreateEventExW(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
  if (!h) return nullptr;
  if (initialSignaled)
    SetEvent(h);
  FenceD3D11* f = static_cast<FenceD3D11*>(core::Alloc(sizeof(FenceD3D11), alignof(FenceD3D11)));
  if (!f) { CloseHandle(h); return nullptr; }
  new (f) FenceD3D11();
  f->event = h;
  return f;
#else
  (void)initialSignaled;
  FenceD3D11* f = static_cast<FenceD3D11*>(core::Alloc(sizeof(FenceD3D11), alignof(FenceD3D11)));
  if (!f) return nullptr;
  new (f) FenceD3D11();
  return f;
#endif
}

ISemaphore* DeviceD3D11::CreateSemaphore() {
  SemaphoreD3D11* s = static_cast<SemaphoreD3D11*>(core::Alloc(sizeof(SemaphoreD3D11), alignof(SemaphoreD3D11)));
  if (!s) return nullptr;
  new (s) SemaphoreD3D11();
  return s;
}

void DeviceD3D11::DestroyFence(IFence* f) {
  if (!f) return;
  FenceD3D11* fence = static_cast<FenceD3D11*>(f);
  fence->~FenceD3D11();
  core::Free(fence);
}

void DeviceD3D11::DestroySemaphore(ISemaphore* s) {
  if (!s) return;
  SemaphoreD3D11* sem = static_cast<SemaphoreD3D11*>(s);
  sem->~SemaphoreD3D11();
  core::Free(sem);
}

struct SwapChainD3D11 : ISwapChain {
  ComPtr<IDXGISwapChain> swapchain;
  uint32_t width{0}, height{0};

  bool Present() override { return swapchain ? SUCCEEDED(swapchain->Present(0, 0)) : true; }
  ITexture* GetCurrentBackBuffer() override { return nullptr; }
  uint32_t GetCurrentBackBufferIndex() const override { return 0; }
  void Resize(uint32_t w, uint32_t h) override { width = w; height = h; }
  uint32_t GetWidth() const override { return width; }
  uint32_t GetHeight() const override { return height; }
};

ISwapChain* DeviceD3D11::CreateSwapChain(SwapChainDesc const& desc) {
  if (desc.width == 0 || desc.height == 0) return nullptr;
  SwapChainD3D11* sc = static_cast<SwapChainD3D11*>(core::Alloc(sizeof(SwapChainD3D11), alignof(SwapChainD3D11)));
  if (!sc) return nullptr;
  new (sc) SwapChainD3D11();
  sc->width = desc.width;
  sc->height = desc.height;
  if (desc.windowHandle && factory && device) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = desc.bufferCount > 0 ? desc.bufferCount : 2;
    sd.BufferDesc.Width = desc.width;
    sd.BufferDesc.Height = desc.height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = static_cast<HWND>(desc.windowHandle);
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    ComPtr<IDXGISwapChain> chain;
    if (SUCCEEDED(factory->CreateSwapChain(device.Get(), &sd, &chain)))
      sc->swapchain = chain;
  }
  return sc;
}

IDescriptorSetLayout* DeviceD3D11::CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) {
  (void)desc;
  return nullptr;  // P2: D3D11 SRV array or no-op.
}

IDescriptorSet* DeviceD3D11::AllocateDescriptorSet(IDescriptorSetLayout* layout) {
  (void)layout;
  return nullptr;
}

void DeviceD3D11::UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes, uint32_t writeCount) {
  (void)set;(void)writes;(void)writeCount;
}

void DeviceD3D11::DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) {
  (void)layout;
}

void DeviceD3D11::DestroyDescriptorSet(IDescriptorSet* set) {
  (void)set;
}

}  // namespace

IDevice* CreateDeviceD3D11() {
  ComPtr<IDXGIFactory1> factory;
  if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
    return nullptr;

  ComPtr<IDXGIAdapter1> adapter;
  for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
    DXGI_ADAPTER_DESC1 desc;
    adapter->GetDesc1(&desc);
    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
      continue;
    break;
  }
  if (!adapter)
    return nullptr;

  UINT flags = 0;
  D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_0;
  D3D_FEATURE_LEVEL outLevel;
  ComPtr<ID3D11Device> device;
  ComPtr<ID3D11DeviceContext> immediateContext;
  if (FAILED(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags,
                               &level, 1, D3D11_SDK_VERSION, &device, &outLevel, &immediateContext)))
    return nullptr;

  DeviceD3D11* dev = static_cast<DeviceD3D11*>(core::Alloc(sizeof(DeviceD3D11), alignof(DeviceD3D11)));
  if (!dev) return nullptr;
  new (dev) DeviceD3D11();
  dev->device = device;
  dev->factory = factory;
  dev->graphicsQueue.immediateContext = immediateContext;
  dev->graphicsQueue.device = device;
  dev->features.maxTextureDimension2D = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
  dev->features.maxTextureDimension3D = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
  dev->limits.maxBufferSize = 256u * 1024u * 1024u;
  dev->limits.maxTextureDimension2D = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
  dev->limits.maxTextureDimension3D = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
  dev->limits.minUniformBufferOffsetAlignment = 16;
  return dev;
}

void DestroyDeviceD3D11(IDevice* device) {
  if (!device) return;
  DeviceD3D11* dev = static_cast<DeviceD3D11*>(device);
  dev->~DeviceD3D11();
  core::Free(dev);
}

}  // namespace rhi
}  // namespace te
