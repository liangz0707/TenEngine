# Implementation Plan: 008-RHI 完整模块内容

**Branch**: `008-rhi-fullmodule-003` | **Date**: 2026-01-31 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `/specs/008-rhi-fullmodule-003/spec.md`

**模块标识**：**008-rhi**（NNN-modulename 从当前 worktree 推断）。

**技术栈**：C++17、CMake。

**规约与契约**：
- **规约**：`docs/module-specs/008-rhi.md`
- **契约**：`specs/_contracts/008-rhi-public-api.md`
- **上游契约**：`specs/_contracts/001-core-public-api.md`（001-Core；实现时只使用该契约已声明的类型与 API）
- **设计参考**：Unity、Unreal 的模块与 API 构造（见 `docs/research/engine-reference-unity-unreal-modules.md`）

## Summary

本 feature 实现 008-RHI 模块的**完整模块内容**：在 **C++17 + CMake** 技术栈下，提供多后端（**Vulkan、D3D12、Metal**）统一的图形 API 抽象，覆盖设备与队列、命令列表、资源与视图、PSO、同步、SwapChain/Present；ResourceBarrier 采用细粒度接口（按资源 + 状态过渡）；PSO 接受后端原生 Shader 字节码（SPIR-V/DXIL/MSL）；集成 Vulkan Validation Layer、D3D12 Debug Layer，通过构建/运行时选项启用。设计参考 **Unreal RHI/RHICore**、**Unity 底层图形封装** 的模块边界与 API 构造（见 `docs/research/engine-reference-unity-unreal-modules.md`）。

**实现约束（本 plan 强制）**：**必须实现真实 Vulkan/D3D12/Metal 后端**，并进行**合理的封装抽象、接口统一**；**禁止使用 stub**。每个启用的后端须调用对应原生 API（Vulkan vk*/volk、D3D12 ID3D12*、Metal MTL*），不得以空实现或仅返回默认值的占位实现作为交付。

## 实现范围（TenEngine：实现全量 ABI 内容）

- **规约**：本 feature 只实现 ABI 文件中列出的符号与能力；不得设计或实现 ABI 未声明的对外接口。设计时可参考 **Unity、Unreal** 的模块与 API 构造。对外接口以 ABI 文件为准。见 `specs/_contracts/README.md`、`.specify/memory/constitution.md` §VI。
- **全量 ABI 来源**：`specs/_contracts/008-rhi-ABI.md`。本 feature 需实现的**全量 ABI 内容**包括：
  - **类型与枚举**：BackendType、DeviceResourceType、BufferUsage、TextureUsage、PixelFormat、TextureType、ResourceState、PrimitiveTopology、PipelineBindPoint、FrameSlotId、DeviceDesc、BufferDesc、TextureDesc、SamplerDesc、PSODesc、DescriptorSetLayoutDesc、DescriptorSetDesc、Viewport、Rect2D、RenderPassDesc、BufferBarrier、TextureBarrier、DescriptorWrite、IndexType、BufferTextureCopy、SwapChainDesc 等。
  - **设备与队列**：CreateDevice、DestroyDevice、IDevice（GetGraphicsQueue/GetComputeQueue/GetCopyQueue、GetFeatures、GetLimits、GetCommandListForSlot、SubmitCommandList、WaitForSlot、ExecuteLogicalCommandBuffer、CreateDeviceResource、UpdateDeviceResource、DestroyResource、CreateSwapChain、CreateFence、CreateSemaphore、DestroyFence、DestroySemaphore）、IQueue（Submit、WaitIdle）。
  - **命令列表**：ICommandList（Begin、End、Submit、SetPipelineState、SetVertexBuffers、SetIndexBuffer、SetDescriptorSet、SetViewport、SetScissor、BeginRenderPass、EndRenderPass、Draw、DrawIndexed、DrawIndexedIndirect、Dispatch、CopyBuffer、CopyBufferToTexture、CopyTextureToBuffer、ResourceBarrier、ClearRenderTarget、ClearDepthStencil）。
  - **资源与 PSO**：IBuffer、ITexture、ISampler、IPSO、IDescriptorSetLayout、IDescriptorSet（含 Update）。
  - **同步**：IFence（Wait、Reset、GetStatus、GetCompletedValue）、ISemaphore。
  - **交换链**：ISwapChain（Present、GetCurrentBackBuffer、GetCurrentBackBufferIndex、Resize、GetWidth、GetHeight）。
- **本 spec 范围**（与 ABI 对齐）：Device、CommandList、Resources、PSO、Synchronization、SwapChain/Present；三后端 + Validation/Debug Layer；上游仅使用 001-Core 契约已声明类型与 API。

### 实现约束：真实后端，禁止 Stub

- **真实后端**：对每个选中的后端（TE_RHI_VULKAN / TE_RHI_D3D12 / TE_RHI_METAL），须实现**真实**的底层 API 调用（Vulkan：volk/vk*；D3D12：Windows SDK ID3D12*；Metal：Apple Metal MTL*）。CreateDevice(Backend) 须根据 Backend 创建真实设备实例（VkDevice / ID3D12Device / MTLDevice 等），不得返回仅内存占位、无底层句柄的实现。
- **封装抽象**：对外仅暴露契约中的抽象类型（IDevice、IQueue、ICommandList、IBuffer、ITexture、ISampler、IPSO、IFence、ISemaphore、ISwapChain）；后端句柄（VkDevice、ID3D12Device*、MTLDevice* 等）封装在实现内部，不泄漏到公共头文件。
- **接口统一**：同一套 RHI 接口（CreateDevice、GetQueue、CreateBuffer、CreateTexture、Draw、Dispatch、ResourceBarrier、Submit、Present 等）在三后端上语义一致；通过编译期宏或运行时多态选择后端实现，调用方无需区分后端。
- **禁止 stub**：禁止以“仅返回 nullptr/默认值、不调用底层 API”的 stub 作为正式实现。若某平台暂不支持某后端，CreateDevice(该 Backend) 返回 nullptr 并可选日志；已实现的 backend 路径必须为真实实现。

## Technical Context

**Language/Version**: C++17  
**Build**: CMake 3.16+，单构建系统，可复现构建  
**Primary Dependencies**: 001-Core（内存、平台、日志、数学、容器等）；见 `specs/_contracts/001-core-public-api.md`  
**Storage**: N/A（RHI 为运行时图形 API 抽象，无持久化存储）  
**Testing**: CTest；单元/集成测试覆盖契约保证（设备创建、命令提交、资源创建、PSO、同步、SwapChain）  
**Target Platform**: Windows（D3D12/Vulkan）、Linux（Vulkan）、macOS（Metal/Vulkan 经 MoltenVK）

**Design Reference**: Unreal RHI/RHICore（FRHICommandList、多后端抽象、真实 D3D12/Vulkan/Metal 实现）、Unity 底层图形封装与 SRP 边界；模块划分与依赖见 `docs/research/engine-reference-unity-unreal-modules.md`。实现须对标真实后端封装，不得以 stub 交付。

## 依赖引入方式（TenEngine 构建规约）

规约见 `docs/engine-build-module-convention.md`。当前所有子模块构建均使用**源码方式**。

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-Core | **源码** | 通过 TenEngineHelpers / `tenengine_resolve_my_dependencies("008-rhi" …)` 引入上游源码（同级 worktree 或 TENENGINE_xxx_DIR）。 |

### 第三方依赖（本 feature 涉及模块所需）

从 `specs/_contracts/008-rhi-public-api.md`「第三方依赖」读取并填入。

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| volk | FetchContent 或 add_subdirectory | docs/third_party/（若存在） | Vulkan 加载层；Vulkan 后端必需 |
| vulkan-headers | FetchContent 或系统/SDK | 同上 | Vulkan 头文件与定义 |
| D3D12 (Windows SDK) | 系统 SDK | — | Windows 下 D3D12 后端 |
| Metal (Apple) | 系统 SDK | docs/third_party/metal.md | macOS/iOS Metal 后端 |
| Vulkan Validation Layer | 可选，运行时/构建选项 | — | 调试用；发布构建可关闭 |
| D3D12 Debug Layer | 可选，运行时/构建选项 | — | 调试用；发布构建可关闭 |

**说明**：Task 阶段将生成「版本选择、自动下载、配置、安装、编译测试、部署进工程、配置实现」等任务；详见 `docs/third_party-integration-workflow.md`（若存在）。

## Constitution Check

- **§I 模块化**：RHI 为独立模块，边界清晰（Device/CommandList/Resources/PSO/Sync/SwapChain），可独立测试。通过。
- **§II 现代图形 API**：目标 Vulkan、D3D12、Metal；资源与管线显式管理；Shader 数据驱动（SPIR-V/DXIL/MSL）。通过。
- **§III 数据驱动**：PSO 与 Shader 字节码数据驱动；与 010-Shader 对接。通过。
- **§IV 性能与可观测**：开发构建可启用 Validation/Debug Layer；帧/GPU 时序由下游 Pipeline 与工具承担。通过。
- **§V 版本与 ABI**：对外 API 以契约与 ABI 为准；破坏性变更递增 MAJOR。通过。
- **§VI 契约优先**：实现仅使用 001-Core 与 008-RHI 契约已声明类型与 API；实现 ABI 全量符号；构建引入真实 001-Core 源码；**禁止 stub**：本 plan 要求真实 Vulkan/D3D12/Metal 后端实现，禁止以 stub 或占位实现作为交付。通过。

## Project Structure

### Documentation (this feature)

```text
specs/008-rhi-fullmodule-003/
├── plan.md              # 本文件 (/speckit.plan 输出)
├── spec.md              # Feature 规约与澄清
├── checklists/
│   └── requirements.md
├── research.md          # Phase 0 输出（可选，/speckit.plan 可生成）
├── data-model.md        # Phase 1 输出（可选）
├── quickstart.md        # Phase 1 输出（可选）
├── contracts/           # 本 feature 契约片段（可选）
└── tasks.md             # Phase 2 输出 (/speckit.tasks，非本命令创建)
```

### Source Code (repository root)

```text
include/
└── te/
    └── rhi/                    # 公开头文件（契约/ABI 约定命名空间 TenEngine::rhi；路径可为 TenEngine/rhi 或 te/rhi）
        ├── Backend.h          # BackendType、SelectBackend、GetSelectedBackend（若与 public-api 一致）
        ├── Device.h           # IDevice、CreateDevice、DestroyDevice、DeviceDesc、DeviceFeatures、DeviceLimits
        ├── Queue.h            # IQueue、QueueType
        ├── CommandList.hpp    # ICommandList、Begin/End、Draw、Dispatch、Copy、ResourceBarrier、Submit
        ├── Barrier.h          # BufferBarrier、TextureBarrier、ResourceState
        ├── DeviceResource.h   # BufferDesc、TextureDesc、SamplerDesc、PSODesc、DescriptorSetLayoutDesc、DescriptorSetDesc
        ├── Format.h           # PixelFormat
        ├── Buffer.h            # IBuffer
        ├── Texture.h           # ITexture
        ├── Sampler.h           # ISampler
        ├── PSO.h               # IPSO、PrimitiveTopology、PipelineBindPoint
        ├── DescriptorSet.h     # IDescriptorSetLayout、IDescriptorSet、DescriptorWrite
        ├── Fence.h             # IFence
        ├── Sync.h              # ISemaphore（若独立）
        ├── SwapChain.h         # ISwapChain、SwapChainDesc
        └── types.hpp           # Viewport、Rect2D、RenderPassDesc、IndexType、BufferTextureCopy 等

src/
├── device/              # 设备创建、队列、CreateDeviceResource、SwapChain、Fence/Semaphore 创建与销毁
├── command_list/        # 命令录制、Draw/Dispatch/Copy、ResourceBarrier、Submit
├── resources/           # Buffer、Texture、Sampler 实现（或合并在 device）
├── pso/                 # Graphics/Compute PSO、SetShader、Cache
├── sync/                # Fence/Semaphore Wait/Signal、ResourceBarrier 同步语义
└── swapchain/           # SwapChain、Present、GetCurrentBackBuffer、Resize

tests/
├── device_create.cpp    # CreateDevice、GetQueue、GetFeatures
├── command_list_submit.cpp
├── resources_create.cpp
├── pso_create.cpp
├── sync_fence_semaphore.cpp
└── swapchain_create.cpp # 可选，依赖窗口时可占位或条件编译
```

**Structure Decision**: 采用单库 `te_rhi`，按子域分目录（device、command_list、resources、pso、sync、swapchain）；头文件与 ABI 表对应，命名空间 `TenEngine::rhi`；实现仅依赖 001-Core 与 008-RHI 契约。**后端实现**：每后端可在 `src/` 下分子目录（如 `src/vulkan/`、`src/d3d12/`、`src/metal/`）或通过 `#if TE_RHI_VULKAN` 等条件编译同一文件内的真实实现，禁止 stub 路径作为交付。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

本 feature 实现**现有 ABI 全量**内容，无新增或修改 ABI 条目时，本小节为空。若实现过程中对 `specs/_contracts/008-rhi-ABI.md` 有增补或修改，在此记录以便写回。

| 操作 | 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|------|--------|----------|------|----------|--------|------|------|
| （无） | — | — | — | — | — | — | 当前为全量实现，无变更 |

## Complexity Tracking

无违反 Constitution 的例外；若后续有（如额外第三方、额外模块），在此表填写理由。

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| — | — | — |
