/** @file device_d3d11.cpp
 *  D3D11 backend: CreateDeviceD3D11, DestroyDeviceD3D11, CommandList, Fence (T029).
 */
#if defined(TE_RHI_D3D11) && defined(_WIN32)

#include <te/rhi/backend_d3d11.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/descriptor_set.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/queue.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/swapchain.hpp>
#include <te/rhi/sync.hpp>
#include <te/rhi/types.hpp>
#include <d3d11.h>
#include <cstddef>
#include <cstring>
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

struct BufferD3D11 final : IBuffer {
  ID3D11Buffer* buffer = nullptr;
  ~BufferD3D11() override { if (buffer) buffer->Release(); }
};

struct CommandListD3D11 final : ICommandList {
  ID3D11Device* device = nullptr;
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
  void SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) override {
    if (!deferredCtx || !recording) return;
    if (!buffer) {
      ID3D11Buffer* nullBuf = nullptr;
      deferredCtx->VSSetConstantBuffers(slot, 1, &nullBuf);
      deferredCtx->PSSetConstantBuffers(slot, 1, &nullBuf);
      deferredCtx->CSSetConstantBuffers(slot, 1, &nullBuf);
      return;
    }
    BufferD3D11* b = static_cast<BufferD3D11*>(buffer);
    if (!b->buffer) return;
    ID3D11Buffer* cb = b->buffer;
    (void)offset;  /* D3D11 VSSetConstantBuffers has no per-call offset; bind from buffer start */
    deferredCtx->VSSetConstantBuffers(slot, 1, &cb);
    deferredCtx->PSSetConstantBuffers(slot, 1, &cb);
    deferredCtx->CSSetConstantBuffers(slot, 1, &cb);
  }
  void SetVertexBuffer(uint32_t slot, IBuffer* buffer, size_t offset, uint32_t stride) override {
    if (!deferredCtx || !recording) return;
    if (!buffer) {
      ID3D11Buffer* nullBuf = nullptr;
      UINT zero = 0;
      deferredCtx->IASetVertexBuffers(slot, 1, &nullBuf, &zero, &zero);
      return;
    }
    BufferD3D11* b = static_cast<BufferD3D11*>(buffer);
    if (!b->buffer) return;
    UINT off = static_cast<UINT>(offset);
    deferredCtx->IASetVertexBuffers(slot, 1, &b->buffer, &stride, &off);
  }
  void SetIndexBuffer(IBuffer* buffer, size_t offset, uint32_t indexFormat) override {
    if (!deferredCtx || !recording) return;
    if (!buffer) {
      deferredCtx->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
      return;
    }
    BufferD3D11* b = static_cast<BufferD3D11*>(buffer);
    if (!b->buffer) return;
    DXGI_FORMAT fmt = (indexFormat == 1) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
    deferredCtx->IASetIndexBuffer(b->buffer, fmt, static_cast<UINT>(offset));
  }
  void SetGraphicsPSO(IPSO* pso) override {
    if (!deferredCtx || !recording) return;
    PSOD3D11* d = pso ? static_cast<PSOD3D11*>(pso) : nullptr;
    deferredCtx->VSSetShader(d ? d->vs : nullptr, nullptr, 0);
    deferredCtx->PSSetShader(d ? d->ps : nullptr, nullptr, 0);
    if (d) {
      const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
      deferredCtx->OMSetBlendState(d->blendState, blendFactor, 0xFFFFFFFFu);
      deferredCtx->OMSetDepthStencilState(d->depthStencilState, 0);
      deferredCtx->RSSetState(d->rasterizerState);
    }
  }
  void BindDescriptorSet(IDescriptorSet* set) override {
    BindDescriptorSet(0u, set);
  }
  void BindDescriptorSet(uint32_t setIndex, IDescriptorSet* set) override {
    if (setIndex != 0u) return;  /* D3D11 path only binds set 0 for now */
    if (!deferredCtx || !recording) return;
    if (!set) return;
    DescriptorSetD3D11* ds = static_cast<DescriptorSetD3D11*>(set);
    if (!ds->layout) return;
    for (uint32_t i = 0; i < ds->layout->desc.bindingCount && i < DescriptorSetD3D11::kMaxBindings; ++i) {
      uint32_t t = ds->layout->desc.bindings[i].descriptorType;
      uint32_t b = ds->layout->desc.bindings[i].binding;
      if (t == static_cast<uint32_t>(DescriptorType::UniformBuffer) && ds->bindingBuffer[b]) {
        deferredCtx->VSSetConstantBuffers(b, 1, &ds->bindingBuffer[b]);
        deferredCtx->PSSetConstantBuffers(b, 1, &ds->bindingBuffer[b]);
      } else if ((t == static_cast<uint32_t>(DescriptorType::CombinedImageSampler) || t == static_cast<uint32_t>(DescriptorType::Sampler)) && (ds->bindingSrv[b] || ds->bindingSampler[b])) {
        if (ds->bindingSrv[b])
          deferredCtx->PSSetShaderResources(b, 1, &ds->bindingSrv[b]);
        if (ds->bindingSampler[b])
          deferredCtx->PSSetSamplers(b, 1, &ds->bindingSampler[b]);
      }
    }
  }
  void BeginRenderPass(RenderPassDesc const& desc, IRenderPass* pass) override {
    (void)pass;
    if (!deferredCtx || !recording || desc.colorAttachmentCount == 0) return;
    ITexture* tex = desc.colorAttachments[0].texture;
    if (!tex) return;
    TextureD3D11* t = static_cast<TextureD3D11*>(tex);
    if (t->rtv)
      deferredCtx->OMSetRenderTargets(1, &t->rtv, nullptr);
  }
  void NextSubpass() override {}
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

struct TextureD3D11 final : ITexture {
  ID3D11Texture2D* texture = nullptr;
  ID3D11RenderTargetView* rtv = nullptr;  /* optional; used when bound as color target in BeginRenderPass */
  ~TextureD3D11() override {
    if (rtv) { rtv->Release(); rtv = nullptr; }
    if (texture) { texture->Release(); texture = nullptr; }
  }
};

struct SamplerD3D11 final : ISampler {
  ID3D11SamplerState* sampler = nullptr;
  ~SamplerD3D11() override { if (sampler) sampler->Release(); }
};

struct DescriptorSetLayoutD3D11 final : IDescriptorSetLayout {
  DescriptorSetLayoutDesc desc;
};

struct DescriptorSetD3D11 final : IDescriptorSet {
  static constexpr uint32_t kMaxBindings = 16u;
  ID3D11Device* device = nullptr;
  DescriptorSetLayoutD3D11* layout = nullptr;
  ID3D11Buffer* bindingBuffer[kMaxBindings] = {};
  ID3D11ShaderResourceView* bindingSrv[kMaxBindings] = {};
  ID3D11SamplerState* bindingSampler[kMaxBindings] = {};
  ~DescriptorSetD3D11() override {
    for (uint32_t i = 0; i < kMaxBindings; ++i) {
      if (bindingSrv[i]) { bindingSrv[i]->Release(); bindingSrv[i] = nullptr; }
      if (bindingBuffer[i]) { bindingBuffer[i]->Release(); bindingBuffer[i] = nullptr; }
      if (bindingSampler[i]) { bindingSampler[i]->Release(); bindingSampler[i] = nullptr; }
    }
  }
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
  ID3D11BlendState* blendState = nullptr;
  ID3D11DepthStencilState* depthStencilState = nullptr;
  ID3D11RasterizerState* rasterizerState = nullptr;
  ~PSOD3D11() override {
    if (vs) { vs->Release(); vs = nullptr; }
    if (ps) { ps->Release(); ps = nullptr; }
    if (cs) { cs->Release(); cs = nullptr; }
    if (blendState) { blendState->Release(); blendState = nullptr; }
    if (depthStencilState) { depthStencilState->Release(); depthStencilState = nullptr; }
    if (rasterizerState) { rasterizerState->Release(); rasterizerState = nullptr; }
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
    cl->device = device;
    cl->deferredCtx = deferredCtx;
    return cl;
  }
  void DestroyCommandList(ICommandList* cmd) override {
    delete static_cast<CommandListD3D11*>(cmd);
  }
  void UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size) override {
    if (!context || !buf || !data || size == 0) return;
    BufferD3D11* b = static_cast<BufferD3D11*>(buf);
    if (!b->buffer) return;
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (FAILED(context->Map(b->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
      return;
    std::memcpy(static_cast<char*>(mapped.pData) + offset, data, size);
    context->Unmap(b->buffer, 0);
  }
  IBuffer* CreateBuffer(BufferDesc const& desc) override {
    if (!device || desc.size == 0) return nullptr;
    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = (UINT)desc.size;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = 0;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    if (desc.usage != 0) {
      if (desc.usage & static_cast<uint32_t>(BufferUsage::Vertex)) bd.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
      if (desc.usage & static_cast<uint32_t>(BufferUsage::Index)) bd.BindFlags |= D3D11_BIND_INDEX_BUFFER;
      if (desc.usage & static_cast<uint32_t>(BufferUsage::Uniform)) {
        bd.BindFlags |= D3D11_BIND_CONSTANT_BUFFER;
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      }
      if (desc.usage & static_cast<uint32_t>(BufferUsage::Storage)) bd.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
      if (desc.usage & static_cast<uint32_t>(BufferUsage::CopySrc)) bd.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
      if (desc.usage & static_cast<uint32_t>(BufferUsage::CopyDst)) { /* D3D11 has no separate copy-dst bind */ }
    }
    if (bd.BindFlags == 0) bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
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
    return CreateGraphicsPSO(desc, nullptr);
  }
  IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc, IDescriptorSetLayout* layout) override {
    return CreateGraphicsPSO(desc, layout, nullptr, 0);
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
    auto* p = new PSOD3D11();
    if (desc.vertex_shader && desc.vertex_shader_size > 0)
      device->CreateVertexShader(desc.vertex_shader, desc.vertex_shader_size, nullptr, &p->vs);
    if (desc.fragment_shader && desc.fragment_shader_size > 0)
      device->CreatePixelShader(desc.fragment_shader, desc.fragment_shader_size, nullptr, &p->ps);
    if (desc.pipelineState) {
      auto toD3D11Blend = [](BlendFactor f) {
        switch (f) {
          case BlendFactor::Zero: return D3D11_BLEND_ZERO;
          case BlendFactor::One: return D3D11_BLEND_ONE;
          case BlendFactor::SrcColor: return D3D11_BLEND_SRC_COLOR;
          case BlendFactor::OneMinusSrcColor: return D3D11_BLEND_INV_SRC_COLOR;
          case BlendFactor::DstColor: return D3D11_BLEND_DEST_COLOR;
          case BlendFactor::OneMinusDstColor: return D3D11_BLEND_INV_DEST_COLOR;
          case BlendFactor::SrcAlpha: return D3D11_BLEND_SRC_ALPHA;
          case BlendFactor::OneMinusSrcAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
          case BlendFactor::DstAlpha: return D3D11_BLEND_DEST_ALPHA;
          case BlendFactor::OneMinusDstAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
          default: return D3D11_BLEND_ONE;
        }
      };
      auto toD3D11BlendOp = [](BlendOp o) {
        switch (o) {
          case BlendOp::Add: return D3D11_BLEND_OP_ADD;
          case BlendOp::Subtract: return D3D11_BLEND_OP_SUBTRACT;
          case BlendOp::ReverseSubtract: return D3D11_BLEND_OP_REVERSE_SUBTRACT;
          case BlendOp::Min: return D3D11_BLEND_OP_MIN;
          case BlendOp::Max: return D3D11_BLEND_OP_MAX;
          default: return D3D11_BLEND_OP_ADD;
        }
      };
      auto toD3D11Compare = [](CompareOp o) {
        switch (o) {
          case CompareOp::Never: return D3D11_COMPARISON_NEVER;
          case CompareOp::Less: return D3D11_COMPARISON_LESS;
          case CompareOp::Equal: return D3D11_COMPARISON_EQUAL;
          case CompareOp::LessOrEqual: return D3D11_COMPARISON_LESS_EQUAL;
          case CompareOp::Greater: return D3D11_COMPARISON_GREATER;
          case CompareOp::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
          case CompareOp::GreaterOrEqual: return D3D11_COMPARISON_GREATER_EQUAL;
          case CompareOp::Always: return D3D11_COMPARISON_ALWAYS;
          default: return D3D11_COMPARISON_LESS;
        }
      };
      if (desc.pipelineState->blendAttachmentCount > 0) {
        D3D11_BLEND_DESC bd = {};
        bd.AlphaToCoverageEnable = FALSE;
        bd.RenderTarget[0].BlendEnable = desc.pipelineState->blendAttachments[0].blendEnable ? TRUE : FALSE;
        bd.RenderTarget[0].SrcBlend = toD3D11Blend(desc.pipelineState->blendAttachments[0].srcColorBlend);
        bd.RenderTarget[0].DestBlend = toD3D11Blend(desc.pipelineState->blendAttachments[0].dstColorBlend);
        bd.RenderTarget[0].BlendOp = toD3D11BlendOp(desc.pipelineState->blendAttachments[0].colorBlendOp);
        bd.RenderTarget[0].SrcBlendAlpha = toD3D11Blend(desc.pipelineState->blendAttachments[0].srcAlphaBlend);
        bd.RenderTarget[0].DestBlendAlpha = toD3D11Blend(desc.pipelineState->blendAttachments[0].dstAlphaBlend);
        bd.RenderTarget[0].BlendOpAlpha = toD3D11BlendOp(desc.pipelineState->blendAttachments[0].alphaBlendOp);
        bd.RenderTarget[0].RenderTargetWriteMask = desc.pipelineState->blendAttachments[0].colorWriteMask & 0xFu;
        if (SUCCEEDED(device->CreateBlendState(&bd, &p->blendState))) { /* ok */ }
      }
      D3D11_DEPTH_STENCIL_DESC dsd = {};
      dsd.DepthEnable = desc.pipelineState->depthStencil.depthTestEnable ? TRUE : FALSE;
      dsd.DepthWriteMask = desc.pipelineState->depthStencil.depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
      dsd.DepthFunc = toD3D11Compare(desc.pipelineState->depthStencil.depthCompareOp);
      dsd.StencilEnable = FALSE;
      if (SUCCEEDED(device->CreateDepthStencilState(&dsd, &p->depthStencilState))) { /* ok */ }
      D3D11_RASTERIZER_DESC rd = {};
      rd.FillMode = D3D11_FILL_SOLID;
      rd.CullMode = D3D11_CULL_NONE;
      switch (desc.pipelineState->rasterization.cullMode) {
        case CullMode::None: rd.CullMode = D3D11_CULL_NONE; break;
        case CullMode::Front: rd.CullMode = D3D11_CULL_FRONT; break;
        case CullMode::Back: rd.CullMode = D3D11_CULL_BACK; break;
        case CullMode::FrontAndBack: rd.CullMode = D3D11_CULL_BACK; break;
        default: rd.CullMode = D3D11_CULL_BACK; break;
      }
      rd.FrontCounterClockwise = (desc.pipelineState->rasterization.frontFace == FrontFace::CounterClockwise) ? TRUE : FALSE;
      rd.DepthBias = 0;
      rd.SlopeScaledDepthBias = 0.f;
      rd.DepthBiasClamp = 0.f;
      rd.DepthClipEnable = TRUE;
      rd.ScissorEnable = FALSE;
      rd.MultisampleEnable = FALSE;
      rd.AntialiasedLineEnable = FALSE;
      if (SUCCEEDED(device->CreateRasterizerState(&rd, &p->rasterizerState))) { /* ok */ }
    }
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
    DXGI_FORMAT fmt = (desc.format == 0) ? DXGI_FORMAT_R8G8B8A8_UNORM : (DXGI_FORMAT)desc.format;
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = desc.width;
    td.Height = desc.height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = fmt;
    td.SampleDesc = { 1, 0 };
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    td.CPUAccessFlags = 0;
    td.MiscFlags = 0;
    ID3D11Texture2D* tex = nullptr;
    if (FAILED(device->CreateTexture2D(&td, nullptr, &tex)) || !tex) return nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    if (FAILED(device->CreateRenderTargetView(tex, nullptr, &rtv))) {
      tex->Release();
      return nullptr;
    }
    auto* t = new TextureD3D11();
    t->texture = tex;
    t->rtv = rtv;
    auto* sc = new SwapChainD3D11();
    sc->device = this;
    sc->backBuffer = t;
    sc->width = desc.width;
    sc->height = desc.height;
    return sc;
  }
  IDescriptorSetLayout* CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) override {
    if (desc.bindingCount == 0) return nullptr;
    auto* dsl = new DescriptorSetLayoutD3D11();
    dsl->desc = desc;
    return dsl;
  }
  IDescriptorSet* AllocateDescriptorSet(IDescriptorSetLayout* layout) override {
    if (!layout) return nullptr;
    auto* ds = new DescriptorSetD3D11();
    ds->device = device;
    ds->layout = static_cast<DescriptorSetLayoutD3D11*>(layout);
    return ds;
  }
  void UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes, uint32_t writeCount) override {
    if (!set || !writes || writeCount == 0) return;
    DescriptorSetD3D11* ds = static_cast<DescriptorSetD3D11*>(set);
    for (uint32_t i = 0; i < writeCount; ++i) {
      uint32_t b = writes[i].binding;
      if (b >= DescriptorSetD3D11::kMaxBindings) continue;
      if (writes[i].buffer) {
        ds->bindingBuffer[b] = static_cast<BufferD3D11*>(writes[i].buffer)->buffer;
        if (ds->bindingBuffer[b]) ds->bindingBuffer[b]->AddRef();
      }
      if (writes[i].texture) {
        if (ds->bindingSrv[b]) { ds->bindingSrv[b]->Release(); ds->bindingSrv[b] = nullptr; }
        TextureD3D11* tv = static_cast<TextureD3D11*>(writes[i].texture);
        if (tv->texture && device)
          device->CreateShaderResourceView(tv->texture, nullptr, &ds->bindingSrv[b]);
      }
      if (writes[i].sampler) {
        ds->bindingSampler[b] = static_cast<SamplerD3D11*>(writes[i].sampler)->sampler;
        if (ds->bindingSampler[b]) ds->bindingSampler[b]->AddRef();
      }
    }
  }
  IRenderPass* CreateRenderPass(RenderPassDesc const& desc) override { (void)desc; return nullptr; }
  void DestroyRenderPass(IRenderPass* pass) override { (void)pass; }
  void DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) override {
    delete static_cast<DescriptorSetLayoutD3D11*>(layout);
  }
  void DestroyDescriptorSet(IDescriptorSet* set) override {
    delete static_cast<DescriptorSetD3D11*>(set);
  }
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
