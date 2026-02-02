# Data Model: 008-RHI 补充实现（DX11、DXR、ABI TODO）

**Feature**: 008-rhi-fullmodule-004 | **Phase 1** | **Date**: 2026-01-31

## 1. 既有实体（规约与 ABI 已声明）

| 实体 | 字段/关系 | 校验与说明 |
|------|------------|------------|
| IDevice | GetBackend, GetQueue, GetFeatures, CreateCommandList, CreateBuffer/CreateTexture/CreateSampler, CreateView, CreateGraphicsPSO/CreateComputePSO, SetShader, Cache, CreateFence, CreateSemaphore, CreateSwapChain, Destroy* | 创建后直至 DestroyDevice；失败返回 nullptr |
| IQueue | Submit(cmdList, signalFence, waitSemaphore, signalSemaphore), WaitIdle | 非拥有；生命周期与 IDevice 一致 |
| ICommandList | Begin, End, Draw, Dispatch, Copy, ResourceBarrier | 单次录制周期有效；由 IDevice 分配 |
| IBuffer, ITexture, ISampler | — | 创建后直至显式 Destroy |
| IPSO | — | 与 Shader 字节码绑定；可缓存 |
| IFence | Wait, Signal, Reset | 多队列/跨帧同步 |
| ISemaphore | — | 多队列同步 |
| ISwapChain | Present, GetCurrentBackBuffer, GetCurrentBackBufferIndex, Resize, GetWidth, GetHeight | 与窗口可选绑定 |
| Backend | Vulkan=0, D3D12=1, Metal=2 | 本 feature 新增 D3D11=3 |
| DeviceFeatures | maxTextureDimension2D, maxTextureDimension3D | 最小集 |
| BufferBarrier, TextureBarrier | buffer/texture, offset/size, srcState, dstState | 细粒度屏障 |

## 2. 本 feature 新增/修改实体

### 2.1 Backend

- **修改**：枚举增加 `D3D11 = 3`。
- **校验**：CreateDevice(Backend::D3D11) 仅在 TE_RHI_D3D11 且 WIN32 时有效。

### 2.2 DeviceLimits（新增）

| 字段 | 类型 | 说明 |
|------|------|------|
| maxBufferSize | size_t | 单 Buffer 最大字节数 |
| maxTextureDimension2D | uint32_t | 与 DeviceFeatures 对齐 |
| maxTextureDimension3D | uint32_t | 与 DeviceFeatures 对齐 |
| minUniformBufferOffsetAlignment | uint32_t | 可选；Vulkan/D3D12 有 |
| 其他 | 按需扩展 | 见各后端查询能力 |

- **关系**：IDevice::GetLimits() const 返回 DeviceLimits const&。
- **校验**：各后端从底层 API 查询填入；未定义项可为 0。

### 2.3 CreateFence(bool)（修改）

- **修改**：IDevice::CreateFence(bool initialSignaled = false)。
- **语义**：initialSignaled 为 true 时，创建后 Fence 处于已信号状态（D3D12 用 Fence value 1 并 Signal；Vulkan 用 VK_FENCE_CREATE_SIGNALED_BIT；D3D11 用 CreateEvent + SetEvent）。

### 2.4 Submit 全参（新增重载）

- **自由函数**：`void Submit(ICommandList* cmd, IQueue* queue, IFence* signalFence, ISemaphore* waitSem, ISemaphore* signalSem);`
- **关系**：内部调用 `queue->Submit(cmd, signalFence, waitSem, signalSem)`；与既有 `Submit(cmd, queue)` 并存。

### 2.5 DrawIndexed（新增）

- **ICommandList**：`void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;`
- **映射**：D3D12 DrawIndexedInstanced；Vulkan vkCmdDrawIndexed；Metal MTLDrawIndexedPrimitives；D3D11 IASetIndexBuffer + DrawIndexedInstanced。

### 2.6 Viewport / Scissor（新增）

| 实体 | 字段 | 说明 |
|------|------|------|
| Viewport | x, y, width, height (float); minDepth, maxDepth (float) | 视口范围与深度 |
| ScissorRect | x, y, width, height (int32_t 或 uint32_t) | 裁剪矩形 |

- **ICommandList**：`void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports);`、`void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors);`
- **校验**：count 与底层 API 限制一致（通常至少 1）。

### 2.7 RenderPassDesc / BeginRenderPass / EndRenderPass（新增，P2）

| 实体 | 字段 | 说明 |
|------|------|------|
| RenderPassDesc | colorAttachmentCount, colorAttachments[]; depthStencilAttachment; loadOp, storeOp 等 | 与 Vulkan VkRenderPassBeginInfo 对齐；D3D12/D3D11 映射到 RTV/DSV 与 Clear |

- **ICommandList**：`void BeginRenderPass(RenderPassDesc const& desc);`、`void EndRenderPass();`
- **状态**：BeginRenderPass 与 EndRenderPass 成对；中间可录制 Draw/DrawIndexed 等。

### 2.8 CopyBuffer / CopyBufferToTexture / CopyTextureToBuffer（新增，P2）

- **ICommandList**：
  - `void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size);`
  - `void CopyBufferToTexture(IBuffer* src, ITexture* dst, TextureRegion const& dstRegion, ...);`
  - `void CopyTextureToBuffer(ITexture* src, BufferRegion const& srcRegion, IBuffer* dst, size_t dstOffset);`
- **TextureRegion / BufferRegion**：偏移、宽高、mip、arrayLayer 等；与 Vulkan/D3D12 拷贝区域一致。

### 2.9 DescriptorSet（新增，P2）

| 实体 | 字段/关系 | 说明 |
|------|------------|------|
| DescriptorSetLayoutDesc | bindings[] (type, count, stageFlags) | 与 VkDescriptorSetLayoutBinding 对齐 |
| DescriptorWrite | dstSet, binding, type, buffer/texture/sampler 等 | 单次写入描述 |
| IDescriptorSetLayout | — | 由 CreateDescriptorSetLayout 创建；不可变 |
| IDescriptorSet | — | 由 AllocateDescriptorSet 或 UpdateDescriptorSet 得到；可写 |

- **IDevice**：CreateDescriptorSetLayout(DescriptorSetLayoutDesc const&)、AllocateDescriptorSet(IDescriptorSetLayout*)、UpdateDescriptorSet(IDescriptorSet*, DescriptorWrite const*, uint32_t count)。
- **关系**：PSO 绑定描述符集时使用 IDescriptorSet* 或 ViewHandle；各后端映射到 VkDescriptorSet、D3D12 DescriptorHeap 槽、D3D11 SRV 数组、Metal ArgumentEncoder。

### 2.10 DXR（新增，仅 D3D12）

| 实体 | 字段/关系 | 说明 |
|------|------------|------|
| RaytracingAccelerationStructureDesc | type (BLAS/TLAS), geometryCount, geometries[] | 加速结构输入；映射 D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS |
| DispatchRaysDesc | rayGenerationShaderRecord, missShaderTable, hitGroupTable, callableShaderTable; width, height, depth | 映射 D3D12_DISPATCH_RAYS_DESC |
| IRaytracingPipeline（可选） | — | 光追 PSO；IDevice::CreateRaytracingPSO 或 D3D12 扩展 |

- **ICommandList 扩展或独立接口**：
  - `void BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result);`
  - `void DispatchRays(DispatchRaysDesc const& desc);`
- **校验**：仅 D3D12 后端实现；Vulkan/Metal/D3D11 调用时返回或 no-op，或接口返回“不支持”错误码。

## 3. 状态与生命周期

- **IDevice**：CreateDevice 成功 → 可用；DestroyDevice 后不可再使用。
- **ICommandList**：Begin → 录制中；End → 可 Submit；Submit 后可再次 Begin（复用）或 Destroy。
- **IFence**：CreateFence(false) → 未信号；Signal 后 → 已信号；Wait 阻塞直到已信号；Reset 后 → 未信号。
- **RenderPass**：BeginRenderPass → 在 RenderPass 内；EndRenderPass → 离开 RenderPass。

## 4. 与规约/契约对齐

- 所有类型与接口以 `specs/_contracts/008-rhi-public-api.md` 与 `008-rhi-ABI.md`（含本 feature 增量）为准。
- 下游仅通过上述抽象访问；不暴露 Vulkan/D3D12/Metal/D3D11 具体类型。
- 实现仅使用 `001-core-public-api.md` 已声明类型与 API。
