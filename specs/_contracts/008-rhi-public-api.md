# 契约：008-RHI 渲染抽象层对外 API（RCI）

## 适用模块

- **实现方**：**008-RHI**（Render Hardware Interface，T0 图形 API 抽象层）
- **对应规格**：`docs/module-specs/008-rhi.md`
- **角色**：向上层提供统一渲染接口，向下对接 **Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等图形接口（及 **GLSL**、**HLSL/DXIL**、**MSL** 等 Shader 接口，与 010-Shader 对接）；RCI 与 RHI 同义，本契约即 RHI 对外 API。**可以通过宏来判断执行哪一段代码**（如 TE_RHI_VULKAN、TE_RHI_METAL、TE_RHI_D3D12），编译时选择后端相关实现路径。

## 消费者（T0 下游）

- 009-RenderCore（渲染类型与 Pass 协议、资源描述）
- 010-Shader（提交 Shader 字节码、PSO 绑定）
- 019-PipelineCore（命令列表、资源、屏障、提交约定）
- 020-Pipeline（命令缓冲生成与提交、SwapChain/XR）
- 024-Editor（视口渲染）
- 027-XR（提交到 XR 交换链）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| IDevice | 图形设备抽象；创建队列、资源、PSO | 由 RHI 管理，创建后直至销毁 |
| ICommandList | 命令缓冲录制；Draw/Dispatch/Copy、ResourceBarrier、Submit | 单次录制周期内有效 |
| IBuffer / ITexture | 缓冲、纹理；可带视图（SRV/UAV/RTV/DSV 等概念） | 创建后直至显式销毁 |
| ISampler | 采样器 | 创建后直至显式销毁 |
| IPSO | 管线状态对象（图形/计算）；与 Shader 字节码绑定 | 创建后直至显式销毁，可缓存 |
| Fence / Semaphore | 同步对象；多队列、跨帧同步 | 按实现约定 |
| 资源视图句柄 | 描述符或视图 ID，用于绑定到 PSO | 与资源生命周期一致 |

下游仅通过上述抽象类型与句柄访问，不暴露具体后端（Vulkan/Metal/D3D12）类型。平台与后端**可通过宏选择执行哪一段代码**（001-Core 平台宏、本模块 TE_RHI_* 后端宏）。

## 渲染资源显式控制位置

**准备/创建/更新 GPU 资源**（**CreateDeviceResource**、**UpdateDeviceResource**）由本模块提供；**提交到实际 GPU Command**（**SubmitCommandBuffer**）即 **executeLogicalCommandBuffer**、**submitCommandList**。创建逻辑渲染资源（CreateRenderItem）、收集逻辑 CommandBuffer（CollectCommandBuffer）、准备渲染资源（PrepareRenderMaterial、PrepareRenderMesh）见 019-PipelineCore/020-Pipeline。

## 能力列表（提供方保证）

1. **设备与队列**：创建设备、获取队列、特性检测、后端选择（**Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等）；多后端统一接口；**可通过宏判断执行哪一段代码**（编译时选择后端）。
2. **命令列表**：录制绘制/计算/拷贝命令、资源屏障、提交到队列；Begin/End、Draw、Dispatch、Copy、ResourceBarrier、Submit 语义明确。
3. **资源管理**：**CreateDeviceResource**（创建缓冲、纹理、PSO 等 GPU 资源）、**UpdateDeviceResource**（更新缓冲/纹理/描述符等）；创建 Buffer、Texture、Sampler、视图；内存管理与生命周期明确；失败时有明确报告。
4. **PSO**：创建图形/计算管线状态对象、与 Shader 字节码/模块绑定、可选缓存与编译；与 RenderCore/Shader 模块对接。
5. **同步**：Fence、Semaphore、资源屏障、多队列同步；提交顺序与等待语义明确。
6. **错误与恢复**：设备丢失或运行时错误可上报；支持回退或重建，不导致引擎崩溃。
7. **线程安全**：RHI 接口在多线程访问下的行为由实现定义并文档化。

## 调用顺序与约束

- 上层须先完成 RHI 初始化（及所需窗口/上下文，可能依赖 Core 平台抽象或 003-Application），再创建资源与提交命令。
- 资源销毁顺序不得违反底层 API 要求（如先释放依赖该资源的命令或 PSO）。
- Pipeline（020）产出的命令缓冲通过本契约约定的提交接口交给 RHI；具体数据结构与提交时机见契约 `pipeline-to-rci.md`。

## 与流水线的边界

- 渲染管线（020-Pipeline）按 PipelineCore（019）协议构建 Pass 图与命令缓冲，最终通过本契约的**提交接口**交给 RHI 执行；命令缓冲格式与资源状态约定见 `pipeline-to-rci.md`。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| （初始） | 从 002-rendering-rci-interface spec 提炼，供多 Agent 引用 |
| T0 更新 | 对齐 T0 架构 008-RHI：实现方改为 008-RHI，消费者改为 T0 模块列表；类型与能力与 docs/module-specs/008-rhi.md 一致 |
| 2026-01-29 | 契约更新由 plan 008-rhi-fullversion-001 同步 |
| 2026-01-28 | 008-rhi-ABI 补全：参考 Vulkan、Metal、D3D12、Unity/UE RHI，补全类型（BufferUsage、TextureUsage、PixelFormat、TextureType、ResourceState、PrimitiveTopology、PipelineBindPoint、Viewport、Rect2D、RenderPassDesc、BufferBarrier、TextureBarrier、DescriptorWrite、IndexType、BufferTextureCopy、SwapChainDesc）；设备与队列（GetGraphicsQueue/GetComputeQueue/GetCopyQueue、GetFeatures、GetLimits、CreateSwapChain、CreateFence、CreateSemaphore、DestroyResource）；命令列表（SetPipelineState、SetVertexBuffers、SetIndexBuffer、SetDescriptorSet、SetViewport、SetScissor、BeginRenderPass、EndRenderPass、Draw、DrawIndexed、DrawIndexedIndirect、Dispatch、CopyBuffer、CopyBufferToTexture、CopyTextureToBuffer、ResourceBarrier、ClearRenderTarget、ClearDepthStencil）；资源接口（IBuffer::GetSize/GetGPUAddress，ITexture::GetWidth/GetHeight/GetDepthOrArrayLayers/GetMipLevelCount/GetFormat，IPSO::GetPipelineLayout）；描述符集（IDescriptorSet::Update）；同步（IFence::Wait/Reset/GetStatus/GetCompletedValue，DestroyFence/DestroySemaphore）；交换链（GetCurrentBackBuffer、GetCurrentBackBufferIndex、Resize、GetWidth、GetHeight）；IQueue::Submit、WaitIdle |