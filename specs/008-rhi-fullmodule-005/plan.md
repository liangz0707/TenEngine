# Implementation Plan: 008-RHI 完整模块实现

**Branch**: `008-rhi-fullmodule-005` | **Date**: 2026-01-31 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/008-rhi-fullmodule-005/spec.md`。规约 `docs/module-specs/008-rhi.md`，契约 `specs/_contracts/008-rhi-public-api.md`；上游契约 `specs/_contracts/001-core-public-api.md`。**实现真实 Vulkan/D3D12/Metal/D3D11 后端，统一抽象，禁止 stub 与 no-op。**

## Summary

实现 008-RHI 模块全量 ABI：在 C++17 + CMake 下提供统一 RHI 接口，**四个真实后端**（Vulkan、D3D12、Metal、D3D11）均通过各自原生 API 实现设备、队列、命令列表、资源、PSO、同步、SwapChain、描述符集与拷贝/渲染通道；设计参考 Unity/Unreal 的 RHI 分层与 API 构造；不提供 stub 或长期 no-op 实现。

## 实现范围（TenEngine：实现全量 ABI 内容）

> **ABI 生成与保存模式**：见 `.cursor/commands/speckit.plan.md`。本 feature **仅实现现有 ABI**，无新增/修改条目；「契约更新」小节为空。
>
> **全量 ABI 内容**：实现 `specs/_contracts/008-rhi-ABI.md` 中**全部**符号与能力，包括：
> - 类型与枚举（te/rhi/types.hpp）：Backend、DeviceLimits、QueueType、DeviceFeatures、ResourceState、BufferBarrier、TextureBarrier、前向声明
> - 资源描述与接口（te/rhi/resources.hpp）：BufferDesc、TextureDesc、SamplerDesc、ViewDesc、ViewHandle、IBuffer、ITexture、ISampler
> - PSO（te/rhi/pso.hpp）：GraphicsPSODesc、ComputePSODesc、IPSO
> - 设备与队列（te/rhi/device.hpp, te/rhi/queue.hpp）：SelectBackend、GetSelectedBackend、CreateDevice、DestroyDevice、IDevice 全部成员、IQueue::Submit/WaitIdle
> - 命令列表（te/rhi/command_list.hpp）：ICommandList 全部成员、LoadOp/StoreOp、Viewport/ScissorRect、RenderPassDesc、BufferRegion/TextureRegion、Begin/End/Submit
> - 同步（te/rhi/sync.hpp）：IFence、ISemaphore、Wait、Signal
> - 交换链（te/rhi/swapchain.hpp）：SwapChainDesc、ISwapChain 全部成员
> - 描述符集（te/rhi/descriptor_set.hpp）：DescriptorSetLayoutDesc、DescriptorWrite、IDescriptorSetLayout、IDescriptorSet
> - 光追（te/rhi/raytracing.hpp）：RaytracingAccelerationStructureDesc、DispatchRaysDesc；D3D12 后端在 SDK 可用时真实实现，其余后端显式“不支持”路径（非静默 no-op）
>
> **实现约束**：每个后端（Vulkan/D3D12/Metal/D3D11）必须调用对应原生 API 完成上述能力；禁止长期 stub（仅返回 nullptr/空实现）或未文档化的 no-op。D3D11 无显式资源屏障时，ResourceBarrier 实现为文档化“D3D11 无屏障语义”的显式空实现，不属于禁止的 stub。

## Technical Context

**Language/Version**: C++17  
**Primary Dependencies**: 001-Core（源码）；volk、vulkan-headers（Vulkan 后端，FetchContent）；d3d11、d3d12、metal（system，平台 SDK）  
**Storage**: N/A（RHI 无持久化存储）  
**Testing**: CMake + CTest；单元/集成测试验证各后端 CreateDevice、资源创建、命令录制与提交、Fence 同步等契约行为  
**Target Platform**: Windows（Vulkan/D3D11/D3D12）、macOS/iOS（Vulkan/Metal）、Linux（Vulkan）；编译期宏 TE_RHI_VULKAN、TE_RHI_D3D12、TE_RHI_D3D11、TE_RHI_METAL 选择后端

**Project Type**: 单模块静态库（te_rhi）；include 公开、src 按后端分子目录。  
**Performance Goals**: 设备创建与命令提交满足实时渲染帧预算；资源屏障与拷贝为真实 GPU 操作。  
**Constraints**: 仅使用 `001-core-public-api.md` 已声明类型与 API；不暴露 Vulkan/D3D12/Metal/D3D11 具体类型给下游。  
**Scale/Scope**: 四后端 × 全量 ABI 符号；设计参考 Unreal FRHICommandList、Unity 底层图形封装。

## 依赖引入方式（TenEngine 构建规约）

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-core | **源码** | 通过 TenEngineHelpers / `tenengine_resolve_my_dependencies("008-rhi" ...)` 引入上游源码构建（同级 worktree 或 TENENGINE_xxx_DIR）。 |

### 第三方依赖（本 feature 涉及模块所需）

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| volk | source（FetchContent） | [docs/third_party/volk.md](../../docs/third_party/volk.md) | Vulkan 加载器；TE_RHI_VULKAN 时启用 |
| vulkan-headers | source（FetchContent） | [docs/third_party/vulkan-headers.md](../../docs/third_party/vulkan-headers.md) | Vulkan 头文件；与 volk 同用 |
| d3d11 | system | [docs/third_party/d3d11.md](../../docs/third_party/d3d11.md) | Windows D3D11；WIN32 + TE_RHI_D3D11 |
| d3d12 | system | [docs/third_party/d3d12.md](../../docs/third_party/d3d12.md) | Windows D3D12；WIN32 + TE_RHI_D3D12 |
| metal | system | [docs/third_party/metal.md](../../docs/third_party/metal.md) | Apple Metal；APPLE + TE_RHI_METAL |

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 原则 | 状态 | 说明 |
|------|------|------|
| I. Modular Renderer Architecture | 通过 | 008-RHI 为独立模块，边界以契约与 ABI 定义 |
| II. Modern Graphics API First | 通过 | Vulkan/D3D12/Metal 为真实后端；D3D11 为显式支持的即时模式后端 |
| III. Data-Driven Pipeline | 不适用 | RHI 层不负责资产/管线配置 |
| IV. Performance & Observability | 计划满足 | 实现阶段可补充帧预算与日志约定 |
| V. Versioning & ABI Stability | 通过 | 接口以 008-rhi-ABI.md 为准，无未声明符号 |
| VI. Module Boundaries & Contract-First | 通过 | 仅实现 ABI 与 public-api 声明；依赖 001-core 契约；构建引入真实 001-core 源码；**禁止 stub/长期占位**；全量 ABI 实现 |

## Project Structure

### Documentation (this feature)

```text
specs/008-rhi-fullmodule-005/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
└── tasks.md          # /speckit.tasks 产出
```

### Source Code (repository root)

```text
include/te/rhi/
├── types.hpp
├── resources.hpp
├── pso.hpp
├── queue.hpp
├── sync.hpp
├── swapchain.hpp
├── command_list.hpp
├── device.hpp
├── descriptor_set.hpp
├── raytracing.hpp
├── backend_vulkan.hpp    # Vulkan 后端工厂声明
├── backend_d3d12.hpp
├── backend_d3d11.hpp
└── backend_metal.hpp

src/
├── device/           # 设备工厂、Stub 仅用于未启用任何后端时的明确回退
│   └── device.cpp
├── command_list/
│   └── command_list.cpp
├── resources/
│   ├── buffer.cpp
│   ├── texture.cpp
│   └── sampler.cpp
├── pso/
│   └── pso.cpp
├── sync/
│   ├── sync.cpp
│   └── fence.cpp
├── swapchain/
│   └── swapchain.cpp
├── vulkan/           # Vulkan 真实实现
│   └── device_vulkan.cpp
├── d3d12/
│   └── device_d3d12.cpp
├── d3d11/
│   └── device_d3d11.cpp
└── metal/
    └── device_metal.mm

tests/
├── device_create.cpp
├── command_list_submit.cpp
├── resources_create.cpp
├── pso_create.cpp
├── sync_fence_semaphore.cpp
└── swapchain_create.cpp
```

**Structure Decision**: 单库 te_rhi；公开 API 在 `include/te/rhi/`，按 ABI 头文件分文件；各后端在 `src/<backend>/` 内实现真实设备/队列/命令列表/资源/PSO/同步/SwapChain/描述符集，由 `device.cpp` 按 Backend 与宏分发到对应工厂。测试按契约场景覆盖多后端。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

本 feature **无**相对于现有 `specs/_contracts/008-rhi-ABI.md` 的新增或修改条目；实现时严格按现有 ABI 全量实现。

| 操作 | 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|------|--------|----------|------|----------|--------|------|------|
| — | — | — | — | 无变更 | — | — | 实现全量现有 ABI |

## Complexity Tracking

无 Constitution 违规需豁免。
