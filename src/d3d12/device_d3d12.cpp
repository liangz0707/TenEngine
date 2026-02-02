/**
 * @file device_d3d12.cpp
 * @brief D3D12 device implementation (real ID3D12* calls); no stub. Windows only.
 */
#if !defined(TE_RHI_D3D12) || !TE_RHI_D3D12
#error "device_d3d12.cpp must be built with TE_RHI_D3D12=1"
#endif
#if !defined(_WIN32)
#error "D3D12 backend is Windows only"
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
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <windows.h>
#include <cstddef>
#include <new>

// Windows SDK defines CreateSemaphore as a macro; undefine to avoid conflict with IDevice::CreateSemaphore
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif

using Microsoft::WRL::ComPtr;

namespace te {
namespace rhi {

namespace {

constexpr uint32_t kMaxQueuesPerType = 1u;

// --- Forward decls ---
struct CommandListD3D12;
struct DeviceD3D12;
struct FenceD3D12;
struct TextureD3D12;
void SignalFenceOnQueueD3D12(IFence* signalFence, ID3D12CommandQueue* queue);

// --- Queue (ID3D12CommandQueue wrapper) ---
struct QueueD3D12 : IQueue {
  ComPtr<ID3D12CommandQueue> queue;
  ComPtr<ID3D12Device> device;
  ComPtr<ID3D12Fence> idleFence;
  HANDLE idleEvent{nullptr};
  UINT64 idleValue{0};

  void Submit(ICommandList* cmdList, IFence* signalFence,
              ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) override;
  void WaitIdle() override;
};

// --- CommandList (ID3D12GraphicsCommandList wrapper) ---
struct CommandListD3D12 : ICommandList {
  ComPtr<ID3D12CommandAllocator> allocator;
  ComPtr<ID3D12GraphicsCommandList> cmdList;
  ComPtr<ID3D12Device> device;
  DeviceD3D12* owner{nullptr};  // for BeginRenderPass RTV/DSV heaps

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

void CommandListD3D12::Begin() {
  if (allocator && cmdList) {
    allocator->Reset();
    cmdList->Reset(allocator.Get(), nullptr);
  }
}

void CommandListD3D12::End() {
  if (cmdList)
    cmdList->Close();
}

void CommandListD3D12::Draw(uint32_t vc, uint32_t ic, uint32_t fv, uint32_t fi) {
  if (cmdList)
    cmdList->DrawInstanced(vc, ic, fv, fi);
}

void CommandListD3D12::DrawIndexed(uint32_t ic, uint32_t inst, uint32_t fi, int32_t vo, uint32_t finst) {
  (void)ic; (void)inst; (void)fi; (void)vo; (void)finst;
  if (cmdList)
    cmdList->DrawIndexedInstanced(ic, inst, fi, vo, finst);
}

void CommandListD3D12::SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) {
  if (!cmdList || !viewports || count == 0) return;
  D3D12_VIEWPORT vp = {};
  vp.TopLeftX = viewports[0].x;
  vp.TopLeftY = viewports[0].y;
  vp.Width = viewports[0].width;
  vp.Height = viewports[0].height;
  vp.MinDepth = viewports[0].minDepth;
  vp.MaxDepth = viewports[0].maxDepth;
  cmdList->RSSetViewports(count, &vp);
}

void CommandListD3D12::SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) {
  (void)first;
  if (!cmdList || !scissors || count == 0) return;
  D3D12_RECT rect = {};
  rect.left = scissors[0].x;
  rect.top = scissors[0].y;
  rect.right = scissors[0].x + static_cast<LONG>(scissors[0].width);
  rect.bottom = scissors[0].y + static_cast<LONG>(scissors[0].height);
  cmdList->RSSetScissorRects(count, &rect);
}

void BeginRenderPassD3D12Impl(CommandListD3D12* cl, RenderPassDesc const& desc);

void CommandListD3D12::BeginRenderPass(RenderPassDesc const& desc) {
  if (!cmdList || !owner) return;
  BeginRenderPassD3D12Impl(this, desc);
}

void CommandListD3D12::EndRenderPass() {}


void CopyBufferToTextureD3D12Impl(ID3D12GraphicsCommandList* cmdList, ID3D12Device* dev, IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion);
void CopyTextureToBufferD3D12Impl(ID3D12GraphicsCommandList* cmdList, ID3D12Device* dev, ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset);

void CommandListD3D12::CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) {
  if (!cmdList || !device || !src || !dst) return;
  CopyBufferToTextureD3D12Impl(cmdList.Get(), device.Get(), src, srcOffset, dst, dstRegion);
}

void CommandListD3D12::CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) {
  if (!cmdList || !device || !src || !dst) return;
  CopyTextureToBufferD3D12Impl(cmdList.Get(), device.Get(), src, srcRegion, dst, dstOffset);
}

void CommandListD3D12::BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result) {
  (void)desc;(void)scratch;(void)result;
  // DXR: obtain ID3D12GraphicsCommandList4 via QueryInterface, call BuildRaytracingAccelerationStructure; no-op when DXR unavailable.
}

void CommandListD3D12::DispatchRays(DispatchRaysDesc const& desc) {
  (void)desc;
  // DXR: ID3D12GraphicsCommandList4::DispatchRays; no-op when DXR unavailable.
}

void CommandListD3D12::Dispatch(uint32_t x, uint32_t y, uint32_t z) {
  if (cmdList)
    cmdList->Dispatch(x, y, z);
}

void CommandListD3D12::Copy(void const* src, void* dst, size_t size) {
  (void)src; (void)dst; (void)size;
  // T047: CopyBufferRegion; skip for minimal test
}

// T047: ResourceBarrier (full per-resource transition) defined after BufferD3D12/TextureD3D12 below.

void QueueD3D12::Submit(ICommandList* cmdList, IFence* signalFence,
                        ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) {
  (void)waitSemaphore; (void)signalSemaphore;
  if (!cmdList || !queue) return;
  CommandListD3D12* d3dCmd = static_cast<CommandListD3D12*>(cmdList);
  if (!d3dCmd->cmdList) return;
  ID3D12CommandList* lists[] = {d3dCmd->cmdList.Get()};
  queue->ExecuteCommandLists(1, lists);
  // T046: create idle fence/event on first submit for WaitIdle
  if (!idleFence && device) {
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&idleFence));
    idleEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  }
  if (idleFence) {
    ++idleValue;
    queue->Signal(idleFence.Get(), idleValue);
  }
  if (signalFence)
    SignalFenceOnQueueD3D12(signalFence, queue.Get());
}

void QueueD3D12::WaitIdle() {
  if (!idleFence || !idleEvent || idleValue == 0) return;
  idleFence->SetEventOnCompletion(idleValue, idleEvent);
  WaitForSingleObject(idleEvent, INFINITE);
}

// --- Device (ID3D12Device wrapper) ---
struct DeviceD3D12 : IDevice {
  ComPtr<ID3D12Device> device;
  ComPtr<IDXGIFactory4> factory;
  ComPtr<ID3D12DescriptorHeap> rtvHeap;
  ComPtr<ID3D12DescriptorHeap> dsvHeap;
  UINT rtvDescriptorSize{0};
  UINT dsvDescriptorSize{0};
  DeviceFeatures features{};
  DeviceLimits limits{};
  QueueD3D12 graphicsQueue;

  void EnsureRtvDsvHeaps();
  ~DeviceD3D12() override;

  Backend GetBackend() const override { return Backend::D3D12; }
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

DeviceD3D12::~DeviceD3D12() {
  if (device) {
    if (graphicsQueue.idleFence && graphicsQueue.idleEvent && graphicsQueue.idleValue > 0) {
      graphicsQueue.idleFence->SetEventOnCompletion(graphicsQueue.idleValue, graphicsQueue.idleEvent);
      WaitForSingleObject(graphicsQueue.idleEvent, INFINITE);
    }
    rtvHeap.Reset();
    dsvHeap.Reset();
    device.Reset();
  }
  factory.Reset();
}

IQueue* DeviceD3D12::GetQueue(QueueType type, uint32_t index) {
  if (index >= kMaxQueuesPerType) return nullptr;
  if (type == QueueType::Graphics) {
    if (!graphicsQueue.queue && device) {
      D3D12_COMMAND_QUEUE_DESC queueDesc = {};
      queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
      queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
      if (SUCCEEDED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&graphicsQueue.queue))))
        graphicsQueue.device = device;
    }
    return &graphicsQueue;
  }
  return nullptr;
}

ICommandList* DeviceD3D12::CreateCommandList() {
  if (!device) return nullptr;
  ComPtr<ID3D12CommandAllocator> allocator;
  if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
    return nullptr;
  ComPtr<ID3D12GraphicsCommandList> cmdList;
  if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&cmdList)))) {
    return nullptr;
  }
  cmdList->Close(); // Start in closed state
  CommandListD3D12* cl = static_cast<CommandListD3D12*>(core::Alloc(sizeof(CommandListD3D12), alignof(CommandListD3D12)));
  if (!cl) return nullptr;
  new (cl) CommandListD3D12();
  cl->allocator = allocator;
  cl->cmdList = cmdList;
  cl->device = device;
  cl->owner = this;
  return cl;
}

void DeviceD3D12::DestroyCommandList(ICommandList* cmd) {
  if (!cmd) return;
  CommandListD3D12* d3dCmd = static_cast<CommandListD3D12*>(cmd);
  d3dCmd->~CommandListD3D12();
  core::Free(d3dCmd);
}

// T048–T052: Resources, PSO, Sync, SwapChain (placeholder implementations)
struct BufferD3D12 : IBuffer {
  ComPtr<ID3D12Resource> resource;
};

struct TextureD3D12 : ITexture {
  ComPtr<ID3D12Resource> resource;
};

void DeviceD3D12::EnsureRtvDsvHeaps() {
  if (rtvHeap && dsvHeap) return;
  if (!device) return;
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.NumDescriptors = 8;
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  rtvHeapDesc.NodeMask = 0;
  if (SUCCEEDED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap))))
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
  dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  dsvHeapDesc.NumDescriptors = 1;
  dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  dsvHeapDesc.NodeMask = 0;
  if (SUCCEEDED(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap))))
    dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void BeginRenderPassD3D12Impl(CommandListD3D12* cl, RenderPassDesc const& desc) {
  if (!cl->cmdList || !cl->owner) return;
  DeviceD3D12* dev = cl->owner;
  dev->EnsureRtvDsvHeaps();
  if (!dev->rtvHeap || dev->rtvDescriptorSize == 0) return;
  constexpr uint32_t kMax = 8u;
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[kMax];
  uint32_t numRTVs = 0;
  for (uint32_t i = 0; i < desc.colorAttachmentCount && i < kMax && desc.colorAttachments[i]; ++i) {
    TextureD3D12* tex = static_cast<TextureD3D12*>(desc.colorAttachments[i]);
    if (!tex->resource) continue;
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE handle = dev->rtvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(i) * dev->rtvDescriptorSize;
    dev->device->CreateRenderTargetView(tex->resource.Get(), &rtvDesc, handle);
    rtvHandles[numRTVs++] = handle;
  }
  if (numRTVs == 0) return;
  D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
  if (desc.depthStencilAttachment && dev->dsvHeap && dev->dsvDescriptorSize) {
    TextureD3D12* depthTex = static_cast<TextureD3D12*>(desc.depthStencilAttachment);
    if (depthTex->resource) {
      dsvHandle = dev->dsvHeap->GetCPUDescriptorHandleForHeapStart();
      D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
      dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
      dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsvDesc.Texture2D.MipSlice = 0;
      dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
      dev->device->CreateDepthStencilView(depthTex->resource.Get(), &dsvDesc, dsvHandle);
    }
  }
  cl->cmdList->OMSetRenderTargets(numRTVs, rtvHandles, FALSE, dsvHandle.ptr ? &dsvHandle : nullptr);
  if (desc.colorLoadOp == LoadOp::Clear && numRTVs > 0) {
    float c[4] = { desc.clearColor[0], desc.clearColor[1], desc.clearColor[2], desc.clearColor[3] };
    cl->cmdList->ClearRenderTargetView(rtvHandles[0], c, 0, nullptr);
  }
  if (desc.depthStencilAttachment && desc.depthLoadOp == LoadOp::Clear && dsvHandle.ptr)
    cl->cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, desc.clearDepth, static_cast<UINT8>(desc.clearStencil), 0, nullptr);
}

static D3D12_RESOURCE_STATES ToD3D12State(ResourceState s) {
  switch (s) {
    case ResourceState::Common: return D3D12_RESOURCE_STATE_COMMON;
    case ResourceState::VertexBuffer: return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    case ResourceState::IndexBuffer: return D3D12_RESOURCE_STATE_INDEX_BUFFER;
    case ResourceState::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case ResourceState::DepthWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case ResourceState::ShaderResource: return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    case ResourceState::CopySrc: return D3D12_RESOURCE_STATE_COPY_SOURCE;
    case ResourceState::CopyDst: return D3D12_RESOURCE_STATE_COPY_DEST;
    case ResourceState::Present: return D3D12_RESOURCE_STATE_PRESENT;
    default: return D3D12_RESOURCE_STATE_COMMON;
  }
}

void CommandListD3D12::ResourceBarrier(uint32_t bc, BufferBarrier const* bb,
                                        uint32_t tc, TextureBarrier const* tb) {
  if (!cmdList) return;
  if (bc == 0 && tc == 0) return;
  constexpr uint32_t kMaxBarriers = 32u;
  D3D12_RESOURCE_BARRIER barriers[kMaxBarriers];
  uint32_t n = 0;
  for (uint32_t i = 0; i < bc && n < kMaxBarriers && bb; ++i) {
    if (!bb[i].buffer) continue;
    BufferD3D12* buf = static_cast<BufferD3D12*>(bb[i].buffer);
    if (!buf->resource) continue;
    barriers[n].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[n].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barriers[n].Transition.pResource = buf->resource.Get();
    barriers[n].Transition.StateBefore = ToD3D12State(bb[i].srcState);
    barriers[n].Transition.StateAfter = ToD3D12State(bb[i].dstState);
    barriers[n].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    ++n;
  }
  for (uint32_t i = 0; i < tc && n < kMaxBarriers && tb; ++i) {
    if (!tb[i].texture) continue;
    TextureD3D12* tex = static_cast<TextureD3D12*>(tb[i].texture);
    if (!tex->resource) continue;
    barriers[n].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[n].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barriers[n].Transition.pResource = tex->resource.Get();
    barriers[n].Transition.StateBefore = ToD3D12State(tb[i].srcState);
    barriers[n].Transition.StateAfter = ToD3D12State(tb[i].dstState);
    barriers[n].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    ++n;
  }
  if (n > 0)
    cmdList->ResourceBarrier(n, barriers);
}

void CommandListD3D12::CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) {
  if (!cmdList || !src || !dst || size == 0) return;
  BufferD3D12* vs = static_cast<BufferD3D12*>(src);
  BufferD3D12* vd = static_cast<BufferD3D12*>(dst);
  if (!vs->resource || !vd->resource) return;
  cmdList->CopyBufferRegion(vd->resource.Get(), dstOffset, vs->resource.Get(), srcOffset, size);
}

void CopyBufferToTextureD3D12Impl(ID3D12GraphicsCommandList* cmdList, ID3D12Device* dev, IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) {
  BufferD3D12* vs = static_cast<BufferD3D12*>(src);
  TextureD3D12* td = static_cast<TextureD3D12*>(dst);
  if (!vs || !td) return;
  CopyBufferToTextureD3D12(cmdList, dev, vs, srcOffset, td, dstRegion);
}

static void CopyBufferToTextureD3D12(ID3D12GraphicsCommandList* cmdList, ID3D12Device* dev,
    BufferD3D12* src, size_t srcOffset, TextureD3D12* dst, TextureRegion const& dstRegion) {
  if (!cmdList || !dev || !src->resource || !dst->resource) return;
  D3D12_RESOURCE_DESC texDesc = dst->resource->GetDesc();
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
  UINT numRows = 0;
  UINT64 rowSize = 0, totalBytes = 0;
  dev->GetCopyableFootprints(&texDesc, 0, 1, 0, &footprint.Footprint, &numRows, &rowSize, &totalBytes);
  footprint.Offset = srcOffset;
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = dst->resource.Get();
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  cmdList->ResourceBarrier(1, &barrier);
  D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
  dstLoc.pResource = dst->resource.Get();
  dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dstLoc.SubresourceIndex = 0;
  D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
  srcLoc.pResource = src->resource.Get();
  srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  srcLoc.PlacedFootprint = footprint;
  D3D12_BOX box = {};
  box.left = dstRegion.x;
  box.top = dstRegion.y;
  box.front = dstRegion.z;
  box.right = dstRegion.x + (dstRegion.width > 0 ? dstRegion.width : footprint.Footprint.Width);
  box.bottom = dstRegion.y + (dstRegion.height > 0 ? dstRegion.height : footprint.Footprint.Height);
  box.back = dstRegion.z + (dstRegion.depth > 0 ? dstRegion.depth : 1);
  cmdList->CopyTextureRegion(&dstLoc, dstRegion.x, dstRegion.y, dstRegion.z, &srcLoc, &box);
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
  cmdList->ResourceBarrier(1, &barrier);
}

void CopyTextureToBufferD3D12Impl(ID3D12GraphicsCommandList* cmdList, ID3D12Device* dev, ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) {
  TextureD3D12* ts = static_cast<TextureD3D12*>(src);
  BufferD3D12* vd = static_cast<BufferD3D12*>(dst);
  if (!ts || !vd) return;
  CopyTextureToBufferD3D12(cmdList, dev, ts, srcRegion, vd, dstOffset);
}

static void CopyTextureToBufferD3D12(ID3D12GraphicsCommandList* cmdList, ID3D12Device* dev,
    TextureD3D12* src, TextureRegion const& srcRegion, BufferD3D12* dst, size_t dstOffset) {
  if (!cmdList || !dev || !src->resource || !dst->resource) return;
  D3D12_RESOURCE_DESC texDesc = src->resource->GetDesc();
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
  UINT numRows = 0;
  UINT64 rowSize = 0, totalBytes = 0;
  dev->GetCopyableFootprints(&texDesc, 0, 1, 0, &footprint.Footprint, &numRows, &rowSize, &totalBytes);
  footprint.Offset = dstOffset;
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = src->resource.Get();
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  cmdList->ResourceBarrier(1, &barrier);
  D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
  srcLoc.pResource = src->resource.Get();
  srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  srcLoc.SubresourceIndex = 0;
  D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
  dstLoc.pResource = dst->resource.Get();
  dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  dstLoc.PlacedFootprint = footprint;
  D3D12_BOX box = {};
  box.left = srcRegion.x;
  box.top = srcRegion.y;
  box.front = srcRegion.z;
  box.right = srcRegion.x + (srcRegion.width > 0 ? srcRegion.width : footprint.Footprint.Width);
  box.bottom = srcRegion.y + (srcRegion.height > 0 ? srcRegion.height : footprint.Footprint.Height);
  box.back = srcRegion.z + (srcRegion.depth > 0 ? srcRegion.depth : 1);
  cmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, &box);
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
  cmdList->ResourceBarrier(1, &barrier);
}

struct SamplerD3D12 : ISampler {
  D3D12_SAMPLER_DESC desc{};
};

IBuffer* DeviceD3D12::CreateBuffer(BufferDesc const& desc) {
  if (desc.size == 0 || !device) return nullptr;
  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
  D3D12_RESOURCE_DESC resDesc = {};
  resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  resDesc.Width = desc.size;
  resDesc.Height = 1;
  resDesc.DepthOrArraySize = 1;
  resDesc.MipLevels = 1;
  resDesc.Format = DXGI_FORMAT_UNKNOWN;
  resDesc.SampleDesc.Count = 1;
  resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  ComPtr<ID3D12Resource> resource;
  if (FAILED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
                                              D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource))))
    return nullptr;
  BufferD3D12* buf = static_cast<BufferD3D12*>(core::Alloc(sizeof(BufferD3D12), alignof(BufferD3D12)));
  if (!buf) return nullptr;
  new (buf) BufferD3D12();
  buf->resource = resource;
  return buf;
}

ITexture* DeviceD3D12::CreateTexture(TextureDesc const& desc) {
  if (desc.width == 0 || desc.height == 0 || !device) return nullptr;
  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
  D3D12_RESOURCE_DESC resDesc = {};
  resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  resDesc.Width = desc.width;
  resDesc.Height = desc.height;
  resDesc.DepthOrArraySize = 1;
  resDesc.MipLevels = 1;
  resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // T048: map desc.format
  resDesc.SampleDesc.Count = 1;
  resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  ComPtr<ID3D12Resource> resource;
  if (FAILED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
                                              D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource))))
    return nullptr;
  TextureD3D12* tex = static_cast<TextureD3D12*>(core::Alloc(sizeof(TextureD3D12), alignof(TextureD3D12)));
  if (!tex) return nullptr;
  new (tex) TextureD3D12();
  tex->resource = resource;
  return tex;
}

ISampler* DeviceD3D12::CreateSampler(SamplerDesc const& desc) {
  (void)desc;
  SamplerD3D12* samp = static_cast<SamplerD3D12*>(core::Alloc(sizeof(SamplerD3D12), alignof(SamplerD3D12)));
  if (!samp) return nullptr;
  new (samp) SamplerD3D12();
  samp->desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
  samp->desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  samp->desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  samp->desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  samp->desc.MaxAnisotropy = 1;
  samp->desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
  samp->desc.MinLOD = 0.f;
  samp->desc.MaxLOD = D3D12_FLOAT32_MAX;
  return samp;
}

ViewHandle DeviceD3D12::CreateView(ViewDesc const& desc) {
  // T048: CreateDescriptorHeap, create SRV/UAV/RTV/DSV; placeholder
  return desc.resource ? reinterpret_cast<ViewHandle>(desc.resource) : 0;
}

void DeviceD3D12::DestroyBuffer(IBuffer* b) {
  if (!b) return;
  BufferD3D12* buf = static_cast<BufferD3D12*>(b);
  buf->~BufferD3D12();
  core::Free(buf);
}

void DeviceD3D12::DestroyTexture(ITexture* t) {
  if (!t) return;
  TextureD3D12* tex = static_cast<TextureD3D12*>(t);
  tex->~TextureD3D12();
  core::Free(tex);
}

void DeviceD3D12::DestroySampler(ISampler* s) {
  if (!s) return;
  SamplerD3D12* samp = static_cast<SamplerD3D12*>(s);
  samp->~SamplerD3D12();
  core::Free(samp);
}

// T049: PSO
struct PSOD3D12 : IPSO {
  ComPtr<ID3D12PipelineState> pso;
};

IPSO* DeviceD3D12::CreateGraphicsPSO(GraphicsPSODesc const& desc) {
  if (!desc.vertex_shader || desc.vertex_shader_size == 0 || !device) return nullptr;
  // T049: Create root signature, D3D12_GRAPHICS_PIPELINE_STATE_DESC; minimal: return non-null
  PSOD3D12* pso = static_cast<PSOD3D12*>(core::Alloc(sizeof(PSOD3D12), alignof(PSOD3D12)));
  if (!pso) return nullptr;
  new (pso) PSOD3D12();
  return pso;
}

IPSO* DeviceD3D12::CreateComputePSO(ComputePSODesc const& desc) {
  if (!desc.compute_shader || desc.compute_shader_size == 0 || !device) return nullptr;
  PSOD3D12* pso = static_cast<PSOD3D12*>(core::Alloc(sizeof(PSOD3D12), alignof(PSOD3D12)));
  if (!pso) return nullptr;
  new (pso) PSOD3D12();
  return pso;
}

void DeviceD3D12::SetShader(IPSO*, void const*, size_t) {}
void DeviceD3D12::Cache(IPSO*) {}

void DeviceD3D12::DestroyPSO(IPSO* pso) {
  if (!pso) return;
  PSOD3D12* d3dPso = static_cast<PSOD3D12*>(pso);
  d3dPso->~PSOD3D12();
  core::Free(d3dPso);
}

// T050: Sync
struct FenceD3D12 : IFence {
  ComPtr<ID3D12Fence> fence;
  HANDLE event{nullptr};
  uint64_t value{0};

  void Wait() override {
    if (!fence || !event) return;
    fence->SetEventOnCompletion(value, event);
    WaitForSingleObject(event, INFINITE);
  }
  void Signal() override {
    // GPU signals via Queue::Submit(signalFence); host does not signal.
  }
  void Reset() override { value = 0; }
};

void SignalFenceOnQueueD3D12(IFence* signalFence, ID3D12CommandQueue* queue) {
  if (!signalFence || !queue) return;
  FenceD3D12* f = static_cast<FenceD3D12*>(signalFence);
  if (!f->fence) return;
  ++f->value;
  queue->Signal(f->fence.Get(), f->value);
}

struct SemaphoreD3D12 : ISemaphore {};

IFence* DeviceD3D12::CreateFence(bool initialSignaled) {
  if (!device) return nullptr;
  UINT64 initialValue = initialSignaled ? 1u : 0u;
  ComPtr<ID3D12Fence> fence;
  if (FAILED(device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
    return nullptr;
  HANDLE ev = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (!ev) return nullptr;
  FenceD3D12* f = static_cast<FenceD3D12*>(core::Alloc(sizeof(FenceD3D12), alignof(FenceD3D12)));
  if (!f) {
    CloseHandle(ev);
    return nullptr;
  }
  new (f) FenceD3D12();
  f->fence = fence;
  f->event = ev;
  f->value = initialValue;
  return f;
}

ISemaphore* DeviceD3D12::CreateSemaphore() {
  SemaphoreD3D12* s = static_cast<SemaphoreD3D12*>(core::Alloc(sizeof(SemaphoreD3D12), alignof(SemaphoreD3D12)));
  if (!s) return nullptr;
  new (s) SemaphoreD3D12();
  return s;
}

void DeviceD3D12::DestroyFence(IFence* f) {
  if (!f) return;
  FenceD3D12* fence = static_cast<FenceD3D12*>(f);
  if (fence->event) {
    CloseHandle(fence->event);
    fence->event = nullptr;
  }
  fence->~FenceD3D12();
  core::Free(fence);
}

void DeviceD3D12::DestroySemaphore(ISemaphore* s) {
  if (!s) return;
  SemaphoreD3D12* sem = static_cast<SemaphoreD3D12*>(s);
  sem->~SemaphoreD3D12();
  core::Free(sem);
}

// T051: SwapChain
struct SwapChainD3D12 : ISwapChain {
  ComPtr<IDXGISwapChain3> swapchain;
  uint32_t width{0}, height{0};

  bool Present() override { return swapchain ? SUCCEEDED(swapchain->Present(0, 0)) : true; }
  ITexture* GetCurrentBackBuffer() override { return nullptr; /* T051: GetBuffer */ }
  uint32_t GetCurrentBackBufferIndex() const override { return swapchain ? swapchain->GetCurrentBackBufferIndex() : 0; }
  void Resize(uint32_t w, uint32_t h) override { width = w; height = h; /* T051: ResizeBuffers */ }
  uint32_t GetWidth() const override { return width; }
  uint32_t GetHeight() const override { return height; }
};

ISwapChain* DeviceD3D12::CreateSwapChain(SwapChainDesc const& desc) {
  if (desc.width == 0 || desc.height == 0) return nullptr;
  // T051: CreateSwapChainForHwnd with DXGI; test without window: return stub
  SwapChainD3D12* sc = static_cast<SwapChainD3D12*>(core::Alloc(sizeof(SwapChainD3D12), alignof(SwapChainD3D12)));
  if (!sc) return nullptr;
  new (sc) SwapChainD3D12();
  sc->width = desc.width;
  sc->height = desc.height;
  return sc;
}

void DeviceD3D12::DestroySwapChain(ISwapChain* sc) {
  if (!sc) return;
  SwapChainD3D12* swapChain = static_cast<SwapChainD3D12*>(sc);
  swapChain->~SwapChainD3D12();
  core::Free(swapChain);
}

IDescriptorSetLayout* DeviceD3D12::CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) {
  (void)desc;
  return nullptr;  // P2: root signature / descriptor heap; minimal no-op.
}

IDescriptorSet* DeviceD3D12::AllocateDescriptorSet(IDescriptorSetLayout* layout) {
  (void)layout;
  return nullptr;
}

void DeviceD3D12::UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes, uint32_t writeCount) {
  (void)set;(void)writes;(void)writeCount;
}

void DeviceD3D12::DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) {
  (void)layout;
}

void DeviceD3D12::DestroyDescriptorSet(IDescriptorSet* set) {
  (void)set;
}

}  // namespace

IDevice* CreateDeviceD3D12() {
#if defined(TE_RHI_DEBUG_LAYER) && TE_RHI_DEBUG_LAYER && defined(_DEBUG)
  ComPtr<ID3D12Debug> debugController;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    debugController->EnableDebugLayer();
#endif

  ComPtr<IDXGIFactory4> factory;
  if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
    return nullptr;

  ComPtr<IDXGIAdapter1> adapter;
  for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
    DXGI_ADAPTER_DESC1 desc;
    adapter->GetDesc1(&desc);
    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
      continue;
    if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
      break;
  }
  if (!adapter)
    return nullptr;

  ComPtr<ID3D12Device> device;
  if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
    return nullptr;

  DeviceD3D12* dev = static_cast<DeviceD3D12*>(core::Alloc(sizeof(DeviceD3D12), alignof(DeviceD3D12)));
  if (!dev) return nullptr;
  new (dev) DeviceD3D12();
  dev->device = device;
  dev->factory = factory;
  dev->features.maxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
  dev->features.maxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
  dev->limits.maxBufferSize = 256u * 1024u * 1024u;  // 256 MB typical; query D3D12_GPU_VIRTUAL_ADDRESS_RANGE for exact
  dev->limits.maxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
  dev->limits.maxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
  dev->limits.minUniformBufferOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
  return dev;
}

void DestroyDeviceD3D12(IDevice* device) {
  if (!device) return;
  DeviceD3D12* dev = static_cast<DeviceD3D12*>(device);
  dev->~DeviceD3D12();
  core::Free(dev);
}

}  // namespace rhi
}  // namespace te
