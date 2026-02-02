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

// --- Queue (ID3D12CommandQueue wrapper) ---
struct QueueD3D12 : IQueue {
  ComPtr<ID3D12CommandQueue> queue;
  ComPtr<ID3D12Device> device;

  void Submit(ICommandList* cmdList, IFence* signalFence,
              ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) override;
  void WaitIdle() override;
};

// --- CommandList (ID3D12GraphicsCommandList wrapper) ---
struct CommandListD3D12 : ICommandList {
  ComPtr<ID3D12CommandAllocator> allocator;
  ComPtr<ID3D12GraphicsCommandList> cmdList;
  ComPtr<ID3D12Device> device;

  void Begin() override;
  void End() override;
  void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;
  void Dispatch(uint32_t x, uint32_t y, uint32_t z) override;
  void Copy(void const* src, void* dst, size_t size) override;
  void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers,
                       uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) override;
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
  (void)vc; (void)ic; (void)fv; (void)fi;
  // T047: cmdList->DrawInstanced(vc, ic, fv, fi); skip when no PSO bound
}

void CommandListD3D12::Dispatch(uint32_t x, uint32_t y, uint32_t z) {
  (void)x; (void)y; (void)z;
  // T047: cmdList->Dispatch(x, y, z); skip when no PSO bound
}

void CommandListD3D12::Copy(void const* src, void* dst, size_t size) {
  (void)src; (void)dst; (void)size;
  // T047: CopyBufferRegion; skip for minimal test
}

void CommandListD3D12::ResourceBarrier(uint32_t bc, BufferBarrier const* bb,
                                        uint32_t tc, TextureBarrier const* tb) {
  (void)bc; (void)bb; (void)tc; (void)tb;
  if (!cmdList) return;
  if (bc == 0 && tc == 0) return;
  // T047: convert BufferBarrier/TextureBarrier to D3D12_RESOURCE_BARRIER; placeholder: UAV barrier
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  barrier.UAV.pResource = nullptr;
  cmdList->ResourceBarrier(1, &barrier);
}

void QueueD3D12::Submit(ICommandList* cmdList, IFence* signalFence,
                        ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) {
  (void)signalFence; (void)waitSemaphore; (void)signalSemaphore;
  if (!cmdList || !queue) return;
  CommandListD3D12* d3dCmd = static_cast<CommandListD3D12*>(cmdList);
  if (!d3dCmd->cmdList) return;
  ID3D12CommandList* lists[] = {d3dCmd->cmdList.Get()};
  queue->ExecuteCommandLists(1, lists);
}

void QueueD3D12::WaitIdle() {
  // T046: Signal fence on queue, wait on CPU; placeholder: no-op
}

// --- Device (ID3D12Device wrapper) ---
struct DeviceD3D12 : IDevice {
  ComPtr<ID3D12Device> device;
  ComPtr<IDXGIFactory4> factory;
  DeviceFeatures features{};
  QueueD3D12 graphicsQueue;

  ~DeviceD3D12() override;

  Backend GetBackend() const override { return Backend::D3D12; }
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

DeviceD3D12::~DeviceD3D12() {
  if (device) {
    // Wait for GPU idle before destroying
    if (graphicsQueue.queue) {
      // T046: Signal/Wait fence; placeholder
    }
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
  uint64_t value{0};

  void Wait() override { /* T050: WaitForSingleObject(fence event); placeholder no-op */ }
  void Signal() override { /* T050: queue->Signal(fence, ++value); placeholder no-op */ }
  void Reset() override { value = 0; }
};

struct SemaphoreD3D12 : ISemaphore {};

IFence* DeviceD3D12::CreateFence() {
  if (!device) return nullptr;
  ComPtr<ID3D12Fence> fence;
  if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
    return nullptr;
  FenceD3D12* f = static_cast<FenceD3D12*>(core::Alloc(sizeof(FenceD3D12), alignof(FenceD3D12)));
  if (!f) return nullptr;
  new (f) FenceD3D12();
  f->fence = fence;
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
