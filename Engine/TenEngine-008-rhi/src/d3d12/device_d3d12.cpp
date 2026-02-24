/** @file device_d3d12.cpp
 *  D3D12 backend: CreateDeviceD3D12, DestroyDeviceD3D12, CommandList, Fence (T028).
 */
#if defined(TE_RHI_D3D12) && defined(_WIN32)

#include <te/rhi/backend_d3d12.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/queue.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/swapchain.hpp>
#include <te/rhi/sync.hpp>
#include <te/rhi/types.hpp>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
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

using Microsoft::WRL::ComPtr;

struct FenceD3D12 final : IFence {
  ComPtr<ID3D12Fence> fence;
  UINT64 nextValue = 1;
  UINT64 lastSignaledValue = 0;
  HANDLE event = nullptr;
  void Wait() override {
    if (!fence || !event) return;
    UINT64 v = lastSignaledValue;
    if (fence->GetCompletedValue() < v)
      fence->SetEventOnCompletion(v, event);
    WaitForSingleObject(event, INFINITE);
  }
  void Signal() override { (void)0; }
  void Reset() override {
    if (event) ResetEvent(event);
  }
  ~FenceD3D12() override {
    if (event) CloseHandle(event);
  }
};

struct SemaphoreD3D12 final : ISemaphore {
  ComPtr<ID3D12Fence> fence;
  UINT64 value = 0;
  ~SemaphoreD3D12() override = default;
};

struct BufferD3D12 final : IBuffer {
  ComPtr<ID3D12Resource> resource;
  ~BufferD3D12() override = default;
};

struct TextureD3D12 final : ITexture {
  ComPtr<ID3D12Resource> resource;
  ~TextureD3D12() override = default;
};

struct PSOD3D12 final : IPSO {
    ComPtr<ID3D12PipelineState> pipeline;
    ComPtr<ID3D12RootSignature> rootSignature;
    ~PSOD3D12() override = default;
};


struct CommandListD3D12 final : ICommandList {
  ComPtr<ID3D12GraphicsCommandList> list;
  ComPtr<ID3D12CommandAllocator> allocator;
  ID3D12RootSignature* rootSignature = nullptr;
  bool recording = false;

  void Begin() override {
    if (!list || !allocator) return;
    if (recording) return;
    allocator->Reset();
    list->Reset(allocator.Get(), nullptr);
    if (rootSignature) {
      list->SetGraphicsRootSignature(rootSignature);
      list->SetComputeRootSignature(rootSignature);
    }
    recording = true;
  }
  void End() override {
    if (!list || !recording) return;
    list->Close();
    recording = false;
  }
  void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override {
    if (list && recording)
      list->DrawInstanced(vertex_count, instance_count, first_vertex, first_instance);
  }
  void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) override {
    if (list && recording)
      list->DrawIndexedInstanced(index_count, instance_count, first_index, vertex_offset, first_instance);
  }
  void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) override {
    if (!list || !viewports || count == 0) return;
    std::vector<D3D12_VIEWPORT> v(count);
    for (uint32_t i = 0; i < count; ++i) {
      v[i].TopLeftX = viewports[i].x;
      v[i].TopLeftY = viewports[i].y;
      v[i].Width = viewports[i].width;
      v[i].Height = viewports[i].height;
      v[i].MinDepth = viewports[i].minDepth;
      v[i].MaxDepth = viewports[i].maxDepth;
    }
    list->RSSetViewports(count, v.data());
  }
  void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) override {
    if (!list || !scissors || count == 0) return;
    std::vector<D3D12_RECT> r(count);
    for (uint32_t i = 0; i < count; ++i) {
      r[i].left = scissors[i].x;
      r[i].top = scissors[i].y;
      r[i].right = scissors[i].x + (LONG)scissors[i].width;
      r[i].bottom = scissors[i].y + (LONG)scissors[i].height;
    }
    list->RSSetScissorRects(count, r.data());
  }
  void SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) override {
    if (!list || !recording || !rootSignature) return;
    if (!buffer) return;
    BufferD3D12* b = static_cast<BufferD3D12*>(buffer);
    if (!b->resource) return;
    D3D12_GPU_VIRTUAL_ADDRESS gpuVa = b->resource->GetGPUVirtualAddress() + (UINT64)offset;
    list->SetGraphicsRootConstantBufferView(slot, gpuVa);
    list->SetComputeRootConstantBufferView(slot, gpuVa);
  }
  void SetVertexBuffer(uint32_t slot, IBuffer* buffer, size_t offset, uint32_t stride) override {
    if (!list || !recording) return;
    if (!buffer) {
      list->IASetVertexBuffers(slot, 1, nullptr);
      return;
    }
    BufferD3D12* b = static_cast<BufferD3D12*>(buffer);
    if (!b->resource) return;
    D3D12_RESOURCE_DESC rd = b->resource->GetDesc();
    UINT64 bufSize = rd.Width;
    D3D12_VERTEX_BUFFER_VIEW vbv = {};
    vbv.BufferLocation = b->resource->GetGPUVirtualAddress() + (UINT64)offset;
    vbv.SizeInBytes = (UINT)(offset < bufSize ? bufSize - (UINT64)offset : 0);
    vbv.StrideInBytes = stride;
    list->IASetVertexBuffers(slot, 1, &vbv);
  }
  void SetIndexBuffer(IBuffer* buffer, size_t offset, uint32_t indexFormat) override {
    if (!list || !recording) return;
    if (!buffer) {
      list->IASetIndexBuffer(nullptr);
      return;
    }
    BufferD3D12* b = static_cast<BufferD3D12*>(buffer);
    if (!b->resource) return;
    DXGI_FORMAT fmt = (indexFormat == 1) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
    D3D12_RESOURCE_DESC rd = b->resource->GetDesc();
    UINT64 bufSize = rd.Width;
    D3D12_INDEX_BUFFER_VIEW ibv = {};
    ibv.BufferLocation = b->resource->GetGPUVirtualAddress() + (UINT64)offset;
    ibv.SizeInBytes = (UINT)(offset < bufSize ? bufSize - (UINT64)offset : 0);
    ibv.Format = fmt;
    list->IASetIndexBuffer(&ibv);
  }
  void SetGraphicsPSO(IPSO* pso) override {
    if (!list || !recording) return;
    if (!pso) return;
    PSOD3D12* d = static_cast<PSOD3D12*>(pso);
    if (d->pipeline) list->SetPipelineState(d->pipeline.Get());
    if (d->rootSignature) list->SetGraphicsRootSignature(d->rootSignature.Get());
  }
  void BindDescriptorSet(IDescriptorSet* set) override { BindDescriptorSet(0u, set); }
  void BindDescriptorSet(uint32_t setIndex, IDescriptorSet* set) override { (void)setIndex; (void)set; /* TODO: D3D12 descriptor set binding */ }
  void BeginRenderPass(RenderPassDesc const& desc, IRenderPass* pass) override { (void)desc; (void)pass; }
  void NextSubpass() override {}
  void EndRenderPass() override {}
  void BeginOcclusionQuery(uint32_t queryIndex) override { (void)queryIndex; }
  void EndOcclusionQuery(uint32_t queryIndex) override { (void)queryIndex; }
  void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) override {
    if (!list || !recording || !src || !dst) return;
    BufferD3D12* srcD = static_cast<BufferD3D12*>(src);
    BufferD3D12* dstD = static_cast<BufferD3D12*>(dst);
    if (!srcD->resource || !dstD->resource) return;
    list->CopyBufferRegion(dstD->resource.Get(), dstOffset, srcD->resource.Get(), srcOffset, size);
  }
  void CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) override {
    if (!list || !recording || !src || !dst) return;
    BufferD3D12* srcD = static_cast<BufferD3D12*>(src);
    TextureD3D12* dstD = static_cast<TextureD3D12*>(dst);
    if (!srcD->resource || !dstD->resource) return;
    
    D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
    dstLoc.pResource = dstD->resource.Get();
    dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLoc.SubresourceIndex = dstRegion.mipLevel + dstRegion.arrayLayer;
    
    D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
    srcLoc.pResource = srcD->resource.Get();
    srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLoc.PlacedFootprint.Offset = srcOffset;
    srcLoc.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Default format
    srcLoc.PlacedFootprint.Footprint.Width = dstRegion.width;
    srcLoc.PlacedFootprint.Footprint.Height = dstRegion.height;
    srcLoc.PlacedFootprint.Footprint.Depth = dstRegion.depth;
    srcLoc.PlacedFootprint.Footprint.RowPitch = dstRegion.width * 4; // 4 bytes per pixel
    
    list->CopyTextureRegion(&dstLoc, dstRegion.x, dstRegion.y, dstRegion.z, &srcLoc, nullptr);
  }
  void CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) override {
    if (!list || !recording || !src || !dst) return;
    TextureD3D12* srcD = static_cast<TextureD3D12*>(src);
    BufferD3D12* dstD = static_cast<BufferD3D12*>(dst);
    if (!srcD->resource || !dstD->resource) return;
    
    D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
    srcLoc.pResource = srcD->resource.Get();
    srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLoc.SubresourceIndex = srcRegion.mipLevel + srcRegion.arrayLayer;
    
    D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
    dstLoc.pResource = dstD->resource.Get();
    dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dstLoc.PlacedFootprint.Offset = dstOffset;
    dstLoc.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    dstLoc.PlacedFootprint.Footprint.Width = srcRegion.width;
    dstLoc.PlacedFootprint.Footprint.Height = srcRegion.height;
    dstLoc.PlacedFootprint.Footprint.Depth = srcRegion.depth;
    dstLoc.PlacedFootprint.Footprint.RowPitch = srcRegion.width * 4;
    
    D3D12_BOX srcBox = {};
    srcBox.left = srcRegion.x;
    srcBox.top = srcRegion.y;
    srcBox.front = srcRegion.z;
    srcBox.right = srcRegion.x + srcRegion.width;
    srcBox.bottom = srcRegion.y + srcRegion.height;
    srcBox.back = srcRegion.z + srcRegion.depth;
    
    list->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, &srcBox);
  }
  /** Raytracing: explicit unsupported when DXR SDK not in build (documented no-op). */
  void BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result) override {
    (void)desc;(void)scratch;(void)result;
  }
  void DispatchRays(DispatchRaysDesc const& desc) override { (void)desc; }
  void Dispatch(uint32_t x, uint32_t y, uint32_t z) override {
    if (list && recording)
      list->Dispatch(x, y, z);
  }
  void Copy(void const* src, void* dst, size_t size) override { (void)src;(void)dst;(void)size; }
  void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers,
                       uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) override {
    if (!list || !recording) return;
    if (bufferBarrierCount == 0 && textureBarrierCount == 0) return;
    std::vector<D3D12_RESOURCE_BARRIER> barriers;
    for (uint32_t i = 0; i < bufferBarrierCount && bufferBarriers; ++i) {
      D3D12_RESOURCE_BARRIER b = {};
      b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      b.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      b.Transition.pResource = nullptr;
      b.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
      b.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
      b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      barriers.push_back(b);
    }
    for (uint32_t i = 0; i < textureBarrierCount && textureBarriers; ++i) {
      D3D12_RESOURCE_BARRIER b = {};
      b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      b.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      b.Transition.pResource = nullptr;
      b.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
      b.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
      b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      barriers.push_back(b);
    }
    if (!barriers.empty())
      list->ResourceBarrier((UINT)barriers.size(), barriers.data());
  }
  ~CommandListD3D12() override = default;
};

struct SamplerD3D12 final : ISampler {
  D3D12_SAMPLER_DESC desc{};
  ~SamplerD3D12() override = default;
};

struct SwapChainD3D12 final : ISwapChain {
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
  ~SwapChainD3D12() override {
    if (device && backBuffer) device->DestroyTexture(backBuffer);
  }
};

struct QueueD3D12 final : IQueue {
  ComPtr<ID3D12CommandQueue> queue;
  void Submit(ICommandList* cmdList, IFence* signalFence,
              ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) override {
    (void)waitSemaphore;
    (void)signalSemaphore;
    if (!queue || !cmdList) return;
    CommandListD3D12* d12 = static_cast<CommandListD3D12*>(cmdList);
    if (!d12->list) return;
    ID3D12CommandList* lists[] = { d12->list.Get() };
    queue->ExecuteCommandLists(1, lists);
    if (signalFence) {
      FenceD3D12* f = static_cast<FenceD3D12*>(signalFence);
      UINT64 v = f->nextValue++;
      f->lastSignaledValue = v;
      queue->Signal(f->fence.Get(), v);
    }
  }
  void WaitIdle() override {
    if (!queue) return;
    ComPtr<ID3D12Device> dev;
    if (FAILED(queue->GetDevice(IID_PPV_ARGS(&dev)))) return;
    ComPtr<ID3D12Fence> fence;
    if (FAILED(dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) return;
    HANDLE e = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    if (!e) return;
    queue->Signal(fence.Get(), 1);
    fence->SetEventOnCompletion(1, e);
    WaitForSingleObject(e, INFINITE);
    CloseHandle(e);
  }
  ~QueueD3D12() override = default;
};

struct DeviceD3D12 final : IDevice {
  ComPtr<ID3D12Device> device;
  ComPtr<ID3D12CommandQueue> queue;
  ComPtr<ID3D12RootSignature> rootSignature;
  QueueD3D12* queueWrapper = nullptr;
  DeviceFeatures features{};
  DeviceLimits limits{};

  Backend GetBackend() const override { return Backend::D3D12; }
  IQueue* GetQueue(QueueType type, uint32_t index) override {
    (void)type;
    (void)index;
    return queueWrapper;
  }
  DeviceFeatures const& GetFeatures() const override { return features; }
  DeviceLimits const& GetLimits() const override { return limits; }
  ICommandList* CreateCommandList() override {
    if (!device || !rootSignature) return nullptr;
    ComPtr<ID3D12CommandAllocator> allocator;
    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
      return nullptr;
    ComPtr<ID3D12GraphicsCommandList> list;
    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&list))))
      return nullptr;
    list->Close();
    auto* cl = new CommandListD3D12();
    cl->allocator = allocator;
    cl->list = list;
    cl->rootSignature = rootSignature.Get();
    return cl;
  }
  void DestroyCommandList(ICommandList* cmd) override {
    delete static_cast<CommandListD3D12*>(cmd);
  }
  void UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size) override {
    if (!device || !buf || !data || size == 0) return;
    BufferD3D12* b = static_cast<BufferD3D12*>(buf);
    if (!b->resource) return;
    void* ptr = nullptr;
    D3D12_RANGE readRange = { 0, 0 };
    if (FAILED(b->resource->Map(0, &readRange, &ptr))) return;
    std::memcpy(static_cast<char*>(ptr) + offset, data, size);
    D3D12_RANGE writeRange = { offset, offset + size };
    b->resource->Unmap(0, &writeRange);
  }
  IBuffer* CreateBuffer(BufferDesc const& desc) override {
    if (!device || desc.size == 0) return nullptr;
    bool isUniform = (desc.usage & static_cast<uint32_t>(BufferUsage::Uniform)) != 0;
    D3D12_HEAP_PROPERTIES hp = {};
    hp.Type = isUniform ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
    hp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    rd.Alignment = 0;
    rd.Width = desc.size;
    rd.Height = 1;
    rd.DepthOrArraySize = 1;
    rd.MipLevels = 1;
    rd.Format = DXGI_FORMAT_UNKNOWN;
    rd.SampleDesc = { 1, 0 };
    rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    rd.Flags = D3D12_RESOURCE_FLAG_NONE;
    D3D12_RESOURCE_STATES state = isUniform ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON;
    ComPtr<ID3D12Resource> res;
    if (FAILED(device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd,
                                                state, nullptr, IID_PPV_ARGS(&res))))
      return nullptr;
    auto* b = new BufferD3D12();
    b->resource = res;
    return b;
  }
  ITexture* CreateTexture(TextureDesc const& desc) override {
    if (!device || desc.width == 0 || desc.height == 0) return nullptr;
    DXGI_FORMAT fmt = (desc.format == 0) ? DXGI_FORMAT_R8G8B8A8_UNORM : static_cast<DXGI_FORMAT>(desc.format);
    D3D12_HEAP_PROPERTIES hp = {};
    hp.Type = D3D12_HEAP_TYPE_DEFAULT;
    hp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = (desc.depth > 1) ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rd.Alignment = 0;
    rd.Width = desc.width;
    rd.Height = desc.height;
    rd.DepthOrArraySize = desc.depth > 0 ? desc.depth : 1;
    rd.MipLevels = 1;
    rd.Format = fmt;
    rd.SampleDesc = { 1, 0 };
    rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    rd.Flags = D3D12_RESOURCE_FLAG_NONE;
    ComPtr<ID3D12Resource> res;
    if (FAILED(device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd,
                                                D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&res))))
      return nullptr;
    auto* t = new TextureD3D12();
    t->resource = res;
    return t;
  }
  ISampler* CreateSampler(SamplerDesc const& desc) override {
    if (!device) return nullptr;
    auto* s = new SamplerD3D12();
    s->desc.Filter = (desc.filter == 0) ? D3D12_FILTER_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    s->desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    s->desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    s->desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    s->desc.MipLODBias = 0.f;
    s->desc.MaxAnisotropy = 1;
    s->desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    s->desc.BorderColor[0] = s->desc.BorderColor[1] = s->desc.BorderColor[2] = s->desc.BorderColor[3] = 1.f;
    s->desc.MinLOD = 0.f;
    s->desc.MaxLOD = D3D12_FLOAT32_MAX;
    return s;
  }
  ViewHandle CreateView(ViewDesc const& desc) override { (void)desc; return 0; }
  void DestroyBuffer(IBuffer* b) override { delete static_cast<BufferD3D12*>(b); }
  void DestroyTexture(ITexture* t) override { delete static_cast<TextureD3D12*>(t); }
  void DestroySampler(ISampler* s) override { delete static_cast<SamplerD3D12*>(s); }
  IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc) override {
    return CreateGraphicsPSO(desc, nullptr);
  }
  IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc, IDescriptorSetLayout* layout) override {
    return CreateGraphicsPSO(desc, layout);
  }
  IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc, IDescriptorSetLayout* layout,
                          IRenderPass* pass, uint32_t subpassIndex,
                          IDescriptorSetLayout* layoutSet1) override {
    (void)pass;
    (void)subpassIndex;
    (void)layout;
    (void)layoutSet1;
    if (!device) return nullptr;
    if ((!desc.vertex_shader || desc.vertex_shader_size == 0) && (!desc.fragment_shader || desc.fragment_shader_size == 0))
      return nullptr;
    auto* p = new PSOD3D12();
    return p;
  }
  IPSO* CreateComputePSO(ComputePSODesc const& desc) override {
    if (!device) return nullptr;
    if (!desc.compute_shader || desc.compute_shader_size == 0) return nullptr;
    auto* p = new PSOD3D12();
    return p;
  }
  void SetShader(IPSO* pso, void const* data, size_t size) override { (void)pso; (void)data; (void)size; }
  void Cache(IPSO* pso) override { (void)pso; }
  void DestroyPSO(IPSO* pso) override { delete static_cast<PSOD3D12*>(pso); }
  IFence* CreateFence(bool initialSignaled) override {
    if (!device) return nullptr;
    ComPtr<ID3D12Fence> fence;
    if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
      return nullptr;
    HANDLE ev = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    if (!ev) return nullptr;
    auto* f = new FenceD3D12();
    f->fence = fence;
    f->nextValue = initialSignaled ? 1 : 0;
    f->event = ev;
    return f;
  }
  ISemaphore* CreateSemaphore() override {
    if (!device) return nullptr;
    ComPtr<ID3D12Fence> fence;
    if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&fence))))
      return nullptr;
    auto* sem = new SemaphoreD3D12();
    sem->fence = fence;
    sem->value = 0;
    return sem;
  }
  void DestroyFence(IFence* f) override { delete static_cast<FenceD3D12*>(f); }
  void DestroySemaphore(ISemaphore* s) override {
    if (s) delete static_cast<SemaphoreD3D12*>(s);
  }
  ISwapChain* CreateSwapChain(SwapChainDesc const& desc) override {
    if (!device || desc.width == 0 || desc.height == 0) return nullptr;
    TextureDesc td = {};
    td.width = desc.width;
    td.height = desc.height;
    td.depth = 1;
    td.format = desc.format;
    ITexture* tex = CreateTexture(td);
    if (!tex) return nullptr;
    auto* sc = new SwapChainD3D12();
    sc->device = this;
    sc->backBuffer = tex;
    sc->width = desc.width;
    sc->height = desc.height;
    return sc;
  }
  IDescriptorSetLayout* CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) override { (void)desc; return nullptr; }
  IDescriptorSet* AllocateDescriptorSet(IDescriptorSetLayout* layout) override { (void)layout; return nullptr; }
  void UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes, uint32_t writeCount) override { (void)set; (void)writes; (void)writeCount; }
  IRenderPass* CreateRenderPass(RenderPassDesc const& desc) override { (void)desc; return nullptr; }
  void DestroyRenderPass(IRenderPass* pass) override { (void)pass; }
  void DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) override { (void)layout; }
  void DestroyDescriptorSet(IDescriptorSet* set) override { (void)set; }
  ~DeviceD3D12() override { delete queueWrapper; }
};

}  // namespace

IDevice* CreateDeviceD3D12() {
  UINT dxgiFlags = 0;
  ComPtr<IDXGIFactory4> factory;
  if (FAILED(CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&factory))))
    return nullptr;
  ComPtr<IDXGIAdapter1> adapter;
  for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
    D3D12_FEATURE_DATA_FEATURE_LEVELS fl = {};
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0 };
    fl.NumFeatureLevels = 2;
    fl.pFeatureLevelsRequested = levels;
    ComPtr<ID3D12Device> device;
    if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
      D3D12_COMMAND_QUEUE_DESC qdesc = {};
      qdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
      qdesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
      ComPtr<ID3D12CommandQueue> queue;
      if (SUCCEEDED(device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&queue)))) {
        /* Minimal root signature: 16 CBV slots for SetUniformBuffer(slot 0..15). */
        enum { kMaxUniformSlots = 16 };
        D3D12_ROOT_PARAMETER params[kMaxUniformSlots] = {};
        for (UINT i = 0; i < kMaxUniformSlots; ++i) {
          params[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
          params[i].Descriptor.ShaderRegister = i;
          params[i].Descriptor.RegisterSpace = 0;
          params[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        }
        D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
        rsDesc.NumParameters = kMaxUniformSlots;
        rsDesc.pParameters = params;
        rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
        ComPtr<ID3DBlob> rsBlob;
        ComPtr<ID3DBlob> errBlob;
        if (FAILED(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &errBlob)))
          continue;
        ComPtr<ID3D12RootSignature> rootSig;
        if (FAILED(device->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(),
                                                IID_PPV_ARGS(&rootSig))))
          continue;
        auto* d = new DeviceD3D12();
        d->device = device;
        d->queue = queue;
        d->rootSignature = rootSig;
        d->queueWrapper = new QueueD3D12();
        d->queueWrapper->queue = queue;
        d->limits.maxBufferSize = 256 * 1024 * 1024ull;
        d->limits.maxTextureDimension2D = 16384u;
        d->limits.maxTextureDimension3D = 2048u;
        d->limits.minUniformBufferOffsetAlignment = 256;
        d->features.maxTextureDimension2D = 16384u;
        d->features.maxTextureDimension3D = 2048u;
        return d;
      }
    }
  }
  return nullptr;
}

void DestroyDeviceD3D12(IDevice* device) {
  delete static_cast<DeviceD3D12*>(device);
}

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_D3D12 && _WIN32
