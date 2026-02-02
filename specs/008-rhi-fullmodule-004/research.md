# Research: 008-RHI 补充实现（DX11、DXR、ABI TODO）

**Feature**: 008-rhi-fullmodule-004 | **Phase 0** | **Date**: 2026-01-31

## 1. D3D12 光追（DXR）接口

### Decision

- 在 RHI 层增加**可选**光追抽象：加速结构描述、构建命令、DispatchRays；**仅 D3D12 后端实现**，Vulkan/Metal/D3D11 返回“不支持”或空实现。
- 使用 D3D12 标准 DXR API：`ID3D12Device5`（QueryInterface 自 ID3D12Device）、`ID3D12GraphicsCommandList4`（BuildRaytracingAccelerationStructure、DispatchRays）、`D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS`、`D3D12_DISPATCH_RAYS_DESC` 等；依赖 Windows 10 SDK 1809+（d3d12.lib 已含 DXR）。

### Rationale

- 与 Unreal/Unity 的 RHI 光追抽象一致：上层不关心后端，D3D12 有则用 DXR，其他后端可后续接 Vulkan KHR ray tracing 或占位。
- 不引入额外第三方；DXR 与 D3D12 同 SDK。

### Alternatives considered

- **仅 D3D12 扩展头**：在 008-RHI 内只暴露 D3D12 专有接口（如 `IDeviceD3D12::GetRaytracingCommandList()`）。拒绝原因：下游需要统一抽象，便于多后端切换。
- **Vulkan KHR ray tracing 同步实现**：本 feature 仅做 DXR，Vulkan 光追留作后续；当前 ABI TODO 已较多，先完成 DXR 与 ABI 再扩展 Vulkan。

### 关键 DXR API（D3D12）

| RHI 抽象 | D3D12 实现 |
|----------|------------|
| 加速结构描述 | `D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS`；BLAS/TLAS 由上层组好传入 |
| 构建加速结构 | `ID3D12GraphicsCommandList4::BuildRaytracingAccelerationStructure` |
| 派发光线 | `ID3D12GraphicsCommandList4::DispatchRays(D3D12_DISPATCH_RAYS_DESC)` |
| 光追 PSO | `ID3D12StateObject`（Raytracing Pipeline）；可选在 RHI 层提供 `CreateRaytracingPSO` 或仅 D3D12 扩展 |

**结论**：RHI 新增类型（如 `RaytracingAccelerationStructureDesc`、`DispatchRaysDesc`）与 `ICommandList` 扩展（`BuildAccelerationStructure`、`DispatchRays`）或独立 `ICommandListRayTracing`；D3D12 实现内 QueryInterface 取 ID3D12GraphicsCommandList4 并调用上述 API。

---

## 2. D3D11 后端与 D3D12/Vulkan 一致性

### Decision

- D3D11 后端提供与 D3D12/Vulkan **同一套 RHI 接口**：IDevice、IQueue、ICommandList、IBuffer、ITexture、ISampler、IPSO、IFence、ISemaphore、ISwapChain；CreateDevice(Backend::D3D11)、GetQueue、CreateCommandList、CreateBuffer/CreateTexture/CreateSampler、CreateGraphicsPSO/CreateComputePSO、CreateFence/CreateSemaphore、CreateSwapChain 等全部用 **真实 D3D11 API** 实现。
- **队列**：D3D11 无多队列，仅 **Immediate Context** 作为“Graphics 队列”；GetQueue(Compute/Copy) 可返回 nullptr 或同一 Context 的包装（按契约“越界返回 nullptr”处理）。
- **命令列表**：使用 **Deferred Context**（CreateDeferredContext）录制；End() 时 FinishCommandList 得到 ID3D11CommandList；Submit 时 Immediate Context 执行 ExecuteCommandList。
- **Fence**：D3D11 无 ID3D12Fence；使用 **HANDLE + CreateEventEx**：CreateFence 创建事件，Signal 在 Queue::Submit 完成后由驱动线程或 Flush 后 SetEvent，Wait 为 WaitForSingleObject。与 D3D12/Vulkan 的“提交后 signal”语义一致。
- **ResourceBarrier**：D3D11 无显式屏障；实现为 **no-op**，在 ABI 中注明“D3D11 下为占位，状态由 API 隐式管理”。
- **SwapChain**：IDXGISwapChain + CreateSwapChainForHwnd（有 windowHandle 时）；无窗口时仅保存 width/height，与 D3D12 行为一致。

### Rationale

- 满足“与 dx12 和 vulkan 一致”的**接口一致**；行为上 D3D11 特性（无屏障、单队列）在文档与 ABI 中说明即可。
- 所有实现调用真实 d3d11/dxgi API，禁止 stub。

### Alternatives considered

- **D3D11 仅部分接口**：拒绝；规约要求“后端支持 dx11，要求与 dx12 和 vulkan 一致”，即全量同一接口。
- **Fence 用 D3D11_QUERY_EVENT**：可用但需每帧 CreateQuery/End/GetData，与“Fence 句柄 + Signal/Wait”模型略不同；事件句柄更贴近 D3D12/Vulkan 语义。

---

## 3. ABI TODO 实现范围与优先级

### Decision

| TODO 项 | 实现方式 | 优先级 |
|---------|----------|--------|
| CreateFence(bool signaled) | IDevice::CreateFence(bool initialSignaled = false)；各后端创建时若 initialSignaled 则创建后即 Signal（D3D12 用 Fence value=1 并 Signal，Vulkan 用 VkFence 创建时 signaled，D3D11 用 CreateEvent 的 bManualReset 与初始 SetEvent） | P1 |
| GetLimits() / DeviceLimits | 新增 struct DeviceLimits（如 maxBufferSize, minUniformBufferOffsetAlignment, maxTextureDimension2D/3D 等）；IDevice::GetLimits() const；各后端从底层 API 查询填入 | P1 |
| Submit 全参暴露 | 自由函数 Submit(ICommandList*, IQueue*, IFence*, ISemaphore*, ISemaphore*) 重载；内部调用 queue->Submit(cmd, signalFence, waitSem, signalSem) | P1 |
| DrawIndexed | ICommandList::DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance)；各后端映射到 DrawIndexedInstanced / vkCmdDrawIndexed / MTLDrawIndexedPrimitives | P1 |
| BeginRenderPass / EndRenderPass | 新增 RenderPassDesc（color/depth 附件、load/store）；ICommandList::BeginRenderPass(desc)、EndRenderPass()；Vulkan 对应 vkCmdBeginRenderPass/vkCmdEndRenderPass，D3D12 对应 OMSetRenderTargets + Clear，D3D11 对应 OMSetRenderTargets | P2 |
| SetViewport / SetScissor | Viewport 结构体（x,y,w,h,minDepth,maxDepth）；Scissor（x,y,w,h）；ICommandList::SetViewport/SetScissor；各后端 RSSetViewports/RSSetScissorRects / vkCmdSetViewport/vkCmdSetScissor | P1 |
| CopyBuffer / CopyBufferToTexture / CopyTextureToBuffer | ICommandList 新方法：CopyBuffer(srcBuf, srcOffset, dstBuf, dstOffset, size)；CopyBufferToTexture / CopyTextureToBuffer 传 IBuffer*、ITexture*、区域；各后端 CopyBufferRegion / vkCmdCopyBuffer / vkCmdCopyBufferToImage 等 | P2 |
| ClearRenderTarget / ClearDepthStencil | 可在 BeginRenderPass 的 loadOp 或单独接口；P2 与 RenderPass 一起考虑 | P2 |
| IDescriptorSetLayout / IDescriptorSet | 新增 IDescriptorSetLayout、IDescriptorSet、DescriptorSetLayoutDesc、DescriptorWrite；IDevice::CreateDescriptorSetLayout、AllocateDescriptorSet（或 UpdateDescriptorSet）；Vulkan 用 VkDescriptorSetLayout/VkDescriptorSet，D3D12 用 DescriptorHeap + 根签名绑定，D3D11 用 ID3D11ShaderResourceView 等绑定；Metal 用 MTLArgumentEncoder | P2 |

### Rationale

- P1 项（CreateFence(bool)、GetLimits、Submit 全参、DrawIndexed、SetViewport/SetScissor）与现有录制/提交/同步路径直接相关，先实现。
- P2 项（RenderPass、CopyBuffer*、DescriptorSet）依赖更多类型与后端映射，放在 Phase 1 设计完成后由 tasks 拆分为后续子任务。
- “PipelineState、VertexBuffers、IndexBuffer”等：部分已由 PSO 与 Draw/DrawIndexed 覆盖；显式 SetVertexBuffers/SetIndexBuffer 可作 P2 扩展。

### Alternatives considered

- **DescriptorSet 仅 Vulkan/D3D12**：D3D11 用 legacy 绑定（SetShaderResources）；为接口一致可提供 D3D11 的“DescriptorSet”包装（内部存 ID3D11ShaderResourceView* 数组），或 D3D11 上 CreateDescriptorSetLayout 返回空实现并文档注明。采纳“各后端尽量实现，D3D11 可简化”以保持 ABI 一致。

---

## 4. 技术栈与依赖确认

- **C++17**、**CMake**：已满足；无 NEEDS CLARIFICATION。
- **规约与契约**：docs/module-specs/008-rhi.md、specs/_contracts/008-rhi-public-api.md（及上游 001-core-public-api.md）；设计参考 Unity、Unreal 的 RHI/CommandList/Descriptor 抽象。
- **第三方**：volk、Vulkan-Headers（源码 FetchContent）；d3d11、d3d12、dxgi（系统 SDK）；Metal（系统）；无新增第三方。
- **构建根目录**：当前 worktree 根；依赖 001-Core 源码引入；不执行 cmake 生成工程前已在本 plan 中写明依赖引入方式，用户若需生成工程可据此执行。

---

## 5. 结论与 Phase 1 输入

- **DXR**：RHI 层抽象加速结构构建与 DispatchRays；D3D12 用 ID3D12GraphicsCommandList4 实现；其他后端暂不实现或返回不支持。
- **D3D11**：全量 RHI 接口、真实 D3D11 API；Fence 用事件句柄；ResourceBarrier 为 no-op；与 D3D12/Vulkan 接口一致。
- **ABI TODO**：P1（CreateFence(bool)、GetLimits、Submit 全参、DrawIndexed、Viewport/Scissor）本 feature 实现；P2（RenderPass、CopyBuffer*、DescriptorSet）在 data-model 与 contracts 中设计并拆入 tasks，可实现部分子集。

Phase 1 将产出 data-model.md（含 DeviceLimits、RenderPassDesc、Viewport、Scissor、DXR 描述、DescriptorSet 等）、contracts/（仅增量）、quickstart.md。
