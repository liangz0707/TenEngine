# Implementation Plan: 008-RHI 完整模块（含 DX11、DXR、ABI TODO）

**Branch**: `008-rhi-fullmodule-004` | **Date**: 2026-01-31 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/008-rhi-fullmodule-004/spec.md`；规约 `docs/module-specs/008-rhi.md`，契约 `specs/_contracts/008-rhi-public-api.md`。

## Summary

本 feature 在既有 Vulkan、D3D12、Metal 三后端与完整 RHI ABI 基础上，**补充实现**：(1) **D3D11 后端**，与 D3D12/Vulkan 接口一致（Device/Queue/CommandList/Resources/PSO/Sync/SwapChain 全量真实 API）；(2) **D3D12 光追（DXR）接口**，在 RHI 层暴露加速结构构建与 DispatchRays，D3D12 后端实现、其他后端返回不支持或空实现；(3) **ABI 中“TODO”项**的进一步实现：CreateFence(bool signaled)、GetLimits/DeviceLimits、Submit 全参暴露、DrawIndexed、Begin/EndRenderPass、Viewport/Scissor、CopyBuffer/CopyBufferToTexture/CopyTextureToBuffer、DescriptorSet 相关（IDescriptorSetLayout、IDescriptorSet 等）。技术栈 C++17、CMake；**禁止 stub**，所有后端均调用真实 API。

## 实现范围（TenEngine：实现全量 ABI 内容）

- **原始 ABI**：`specs/_contracts/008-rhi-ABI.md` 中已声明的全部符号（Backend、QueueType、DeviceFeatures、ResourceState、BufferBarrier、TextureBarrier、IDevice、IQueue、ICommandList、IBuffer、ITexture、ISampler、IPSO、IFence、ISemaphore、ISwapChain、CreateDevice、DestroyDevice、SelectBackend、GetSelectedBackend、Create*/Destroy*、Begin/End/Submit、Wait/Signal 等）。
- **本 feature 新增/修改**：
  - **Backend**：新增 `D3D11 = 3`；各后端（含 D3D11）均真实实现。
  - **DXR（仅 D3D12）**：新增类型与接口（见下方「契约更新」）；D3D12 后端实现，Vulkan/Metal/D3D11 可返回“不支持”或空实现。
  - **ABI TODO 实现**：CreateFence(bool signaled)；GetLimits()/DeviceLimits；Submit 全参暴露（自由函数重载）；DrawIndexed；BeginRenderPass/EndRenderPass、SetViewport、SetScissor；CopyBuffer、CopyBufferToTexture、CopyTextureToBuffer；IDescriptorSetLayout、IDescriptorSet、DescriptorSetLayoutDesc、DescriptorSetDesc、DescriptorWrite、CreateDescriptorSetLayout、AllocateDescriptorSet 等（见 research/data-model 与契约更新）。

实现时基于**全量 ABI 内容**（原始 + 新增 + 修改）进行实现。

## Technical Context

**Language/Version**: C++17  
**Primary Dependencies**: 001-Core（内存、平台、日志）；volk、Vulkan-Headers（Vulkan）；d3d11、d3d12、dxgi（Windows）；Metal 框架（macOS）  
**Storage**: N/A（运行时 GPU 资源与命令）  
**Testing**: CTest，现有 te_rhi_test、te_rhi_command_list_test、te_rhi_resources_test、te_rhi_pso_test、te_rhi_sync_test、te_rhi_swapchain_test；需扩展以覆盖 D3D11、DXR、新 ABI 接口  
**Target Platform**: Windows（D3D11/D3D12）、Linux/Android（Vulkan）、macOS/iOS（Metal）

**Performance Goals**: 与现有 RHI 一致；DXR 路径仅 D3D12 有实际开销  
**Constraints**: 仅使用 001-core-public-api 与 008-rhi-public-api/ABI 已声明类型；不暴露后端私有类型  
**Scale/Scope**: 四后端（Vulkan、D3D12、Metal、D3D11）+ DXR 扩展 + ABI TODO 补全

## 依赖引入方式（TenEngine 构建规约）

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-Core | **源码** | 通过 TenEngineHelpers / tenengine_resolve_my_dependencies 引入上游源码构建（同级 worktree 或 TENENGINE_CMAKE_DIR）。 |

**说明**：构建根目录为当前 worktree 根（如 `G:\AIHUMAN\WorkSpaceSDD\TenEngine-008-rhi`）；out-of-source 推荐 `build/`。依赖方式统一源码，无 DLL/预编译。

### 第三方依赖（本 feature 涉及模块所需）

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| volk | source (FetchContent) | 见 public-api | Vulkan 加载；TE_RHI_VULKAN 时启用 |
| Vulkan-Headers | source (FetchContent) | 见 public-api | Vulkan 头文件 |
| d3d11 | system (Windows SDK) | 见 public-api | D3D11 后端；TE_RHI_D3D11 + WIN32 |
| d3d12 | system (Windows SDK) | 见 public-api | D3D12 后端 + DXR；TE_RHI_D3D12 + WIN32 |
| dxgi | system (Windows SDK) | 见 public-api | SwapChain/Adapter；D3D11/D3D12 |
| Metal | system (Apple SDK) | 见 public-api | Metal 后端；TE_RHI_METAL + APPLE |

DXR 使用 D3D12 同 SDK（d3d12.lib、Windows 10 SDK 1809+），无额外第三方。

## Constitution Check

- **§I–II**：模块边界清晰；现代图形 API（Vulkan/D3D12/Metal）+ D3D11 兼容层；DXR 仅 D3D12 实现。
- **§VI**：实现仅使用契约/ABI 声明类型；全量 ABI 实现、无长期 stub；接口变更写回 ABI；下游所需接口在上游 ABI 以 TODO 登记。
- **构建**：依赖通过源码/系统库引入；禁止用 stub 代替真实后端。
- **Gate**：通过（无未澄清违规）。

## Project Structure

### Documentation (this feature)

```text
specs/008-rhi-fullmodule-004/
├── plan.md              # 本文件
├── research.md          # Phase 0 输出
├── data-model.md        # Phase 1 输出
├── quickstart.md        # Phase 1 输出
├── contracts/           # Phase 1 契约增量（仅新增/修改）
├── checklists/
│   └── requirements.md
└── tasks.md             # Phase 2 由 /speckit.tasks 生成
```

### Source Code (repository root)

```text
include/te/rhi/
├── types.hpp, resources.hpp, pso.hpp, device.hpp, queue.hpp,
├── command_list.hpp, sync.hpp, swapchain.hpp,
├── backend_vulkan.hpp, backend_d3d12.hpp, backend_d3d11.hpp, backend_metal.hpp
├── (可选) raytracing.hpp   # DXR 抽象类型与接口
src/
├── device/, command_list/, sync/, pso/, resources/, swapchain/
├── vulkan/, d3d12/, d3d11/, metal/
tests/
├── device_create.cpp, command_list_submit.cpp, resources_create.cpp,
├── pso_create.cpp, sync_fence_semaphore.cpp, swapchain_create.cpp
```

**Structure Decision**: 现有 RHI 单库结构保留；D3D11 新增 `src/d3d11/device_d3d11.cpp`；DXR 扩展在 D3D12 实现内或独立 raytracing 头/实现按 tasks 拆分。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

以下仅列出相对于现有 ABI 的**新增与修改**；实现时按**全量 ABI**（现有表 + 本表）实现。

| 操作 | 模块名 | 命名空间 | 符号 | 接口说明 | 头文件 | 说明 |
|------|--------|----------|------|----------|--------|------|
| 修改 | 008-RHI | te::rhi | Backend | 枚举增加 D3D11 = 3 | te/rhi/types.hpp | `enum class Backend : unsigned { Vulkan=0, D3D12=1, Metal=2, D3D11=3 };` |
| 新增 | 008-RHI | te::rhi | DeviceLimits | struct | te/rhi/types.hpp | 如 maxBufferSize, minUniformBufferOffsetAlignment 等（见 research） |
| 修改 | 008-RHI | te::rhi | IDevice::GetLimits | 成员函数 | te/rhi/device.hpp | `DeviceLimits const& GetLimits() const = 0;` |
| 修改 | 008-RHI | te::rhi | IDevice::CreateFence | 成员函数 | te/rhi/device.hpp | `IFence* CreateFence(bool initialSignaled = false) = 0;` |
| 新增 | 008-RHI | te::rhi | Submit (重载) | 自由函数 | te/rhi/command_list.hpp | `void Submit(ICommandList* cmd, IQueue* queue, IFence* signalFence, ISemaphore* waitSem, ISemaphore* signalSem);` |
| 新增 | 008-RHI | te::rhi | ICommandList::DrawIndexed | 成员函数 | te/rhi/command_list.hpp | `void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;` |
| 新增 | 008-RHI | te::rhi | RenderPassDesc / BeginRenderPass / EndRenderPass | 类型与成员 | te/rhi/command_list.hpp | 见 research/data-model；BeginRenderPass(desc)、EndRenderPass() |
| 新增 | 008-RHI | te::rhi | SetViewport / SetScissor | 成员函数 | te/rhi/command_list.hpp | Viewport/Scissor 结构体 + ICommandList 接口 |
| 新增 | 008-RHI | te::rhi | CopyBuffer / CopyBufferToTexture / CopyTextureToBuffer | 成员函数 | te/rhi/command_list.hpp | 资源间拷贝（替代或补充 Copy） |
| 新增 | 008-RHI | te::rhi | IDescriptorSetLayout, IDescriptorSet, CreateDescriptorSetLayout, AllocateDescriptorSet, DescriptorSetLayoutDesc, DescriptorWrite | 类型与接口 | te/rhi/descriptor_set.hpp 或 resources.hpp | 见 research；各后端映射到 Vulkan DescriptorSet、D3D12 DescriptorHeap、Metal 等 |
| 新增 | 008-RHI | te::rhi | DXR：RaytracingAccelerationStructureDesc, BuildAccelerationStructure, DispatchRays, IRaytracingPipeline / CreateRaytracingPSO（可选） | 类型与接口 | te/rhi/raytracing.hpp 或扩展 device/command_list | D3D12 实现 DXR；Vulkan/Metal/D3D11 返回 nullptr 或 no-op |

（上表为增量；全量 ABI 表见 `specs/_contracts/008-rhi-ABI.md` 与本次更新合并后的结果。）

## Complexity Tracking

（当前无需要豁免的违规；若 Phase 1 后出现再填。）
