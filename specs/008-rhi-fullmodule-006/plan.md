# Implementation Plan: 008-RHI 完整模块实现（含 ABI TODO）

**Branch**: `008-rhi-fullmodule-006` | **Date**: 2026-01-31 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/008-rhi-fullmodule-006/spec.md`。规约 `docs/module-specs/008-rhi.md`，契约 `specs/_contracts/008-rhi-public-api.md`；上游契约 `specs/_contracts/001-core-public-api.md`。**本 feature 实现完整模块内容，并额外实现 ABI 文件中的 TODO 部分。**

## Summary

在 008-RHI 已有全量 ABI 实现基础上，**补齐 ABI TODO**：提供 Buffer 的 CPU 写入（IDevice::UpdateBuffer）、Uniform 绑定到命令列表 slot（ICommandList::SetUniformBuffer）、BufferDesc.usage 的 Uniform 用途位（BufferUsage 枚举或等价常量），以及 009-RenderCore 与 008-RHI 描述符对接约定。tasks 与 implement 阶段**基于全量 ABI 内容**（见 `contracts/008-rhi-ABI-full.md`）进行实现；plan.md 的「契约更新」小节**仅保存相对于现有 `specs/_contracts/008-rhi-ABI.md` 的新增和修改部分**，用于写回契约。

## 实现范围（TenEngine：实现全量 ABI 内容）

> **ABI 生成与保存模式**（见 `.cursor/commands/speckit.plan.md`）：
> - **全量 ABI 内容**：plan 已生成**全量 ABI**（原始 ABI + 本次新增/修改），见 **`specs/008-rhi-fullmodule-006/contracts/008-rhi-ABI-full.md`**。tasks 与 implement **必须基于该全量内容**实现，不得仅实现变化部分。
> - **契约更新小节**：下方「契约更新」**只保存相对于现有 `specs/_contracts/008-rhi-ABI.md` 的新增和修改**；若无新增/修改则产出空清单。写回时仅将该部分增补或替换到 `specs/_contracts/008-rhi-ABI.md`，并移除或更新原 TODO 节。
>
> **本 feature 全量实现范围**：
> - **原始 ABI**：`specs/_contracts/008-rhi-ABI.md` 中已列全部符号与能力（类型、资源、PSO、设备、队列、命令列表、同步、交换链、描述符集、光追）。
> - **本次新增（原 TODO）**：BufferUsage 枚举（含 Uniform 位）、IDevice::UpdateBuffer、ICommandList::SetUniformBuffer；BufferDesc.usage 语义补充（含 Uniform 用途）。
> - **与 009 对接**：BufferDesc/TextureDesc 与 009 产出的描述可对接（类型转换或共用结构）；实现时保证 CreateBuffer(CreateUniformBuffer 等) 使用 BufferUsage::Uniform。

## Technical Context

**Language/Version**: C++17  
**Primary Dependencies**: 001-Core（源码）；volk、vulkan-headers（Vulkan，FetchContent）；d3d11、d3d12、metal（system，平台 SDK）  
**Storage**: N/A（RHI 无持久化）  
**Testing**: CMake + CTest；单元/集成测试验证 UpdateBuffer、SetUniformBuffer、BufferUsage::Uniform 及与 009 对接行为  
**Target Platform**: Windows（Vulkan/D3D11/D3D12）、macOS/iOS（Vulkan/Metal）、Linux（Vulkan）；宏 TE_RHI_* 选择后端

**Project Type**: 单模块静态库（te_rhi）；include 公开、src 按后端分子目录。  
**Performance Goals**: UpdateBuffer 与 SetUniformBuffer 满足每帧 Uniform 更新与绑定预算。  
**Constraints**: 仅使用 `001-core-public-api.md` 已声明类型与 API；不暴露后端类型。  
**Scale/Scope**: 四后端 × 全量 ABI（含 UpdateBuffer、SetUniformBuffer、BufferUsage）。

## 依赖引入方式（TenEngine 构建规约）

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-core | **源码** | 通过 TenEngineHelpers / `tenengine_resolve_my_dependencies("008-rhi" ...)` 引入上游源码构建。 |

### 第三方依赖（本 feature 涉及模块所需）

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| volk | source（FetchContent） | [docs/third_party/volk.md](../../docs/third_party/volk.md) | Vulkan 加载器；TE_RHI_VULKAN 时启用 |
| vulkan-headers | source（FetchContent） | [docs/third_party/vulkan-headers.md](../../docs/third_party/vulkan-headers.md) | Vulkan 头文件 |
| d3d11 | system | [docs/third_party/d3d11.md](../../docs/third_party/d3d11.md) | Windows D3D11；WIN32 + TE_RHI_D3D11 |
| d3d12 | system | [docs/third_party/d3d12.md](../../docs/third_party/d3d12.md) | Windows D3D12；WIN32 + TE_RHI_D3D12 |
| metal | system | [docs/third_party/metal.md](../../docs/third_party/metal.md) | Apple Metal；APPLE + TE_RHI_METAL |

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 原则 | 状态 | 说明 |
|------|------|------|
| I. Modular Renderer Architecture | 通过 | 008-RHI 边界以契约与 ABI 定义 |
| II. Modern Graphics API First | 通过 | Vulkan/D3D12/Metal/D3D11 真实后端 |
| III. Data-Driven Pipeline | 不适用 | RHI 层不负责资产/管线配置 |
| IV. Performance & Observability | 计划满足 | 实现可补充帧预算与日志 |
| V. Versioning & ABI Stability | 通过 | 接口以 ABI 为准；新增条目写入 ABI |
| VI. Module Boundaries & Contract-First | 通过 | 仅实现 ABI 与 public-api；依赖 001-core 契约；全量 ABI 实现（含 TODO 转正） |

## Project Structure

### Documentation (this feature)

```text
specs/008-rhi-fullmodule-006/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── 008-rhi-ABI-full.md   # 全量 ABI（实现参考）
└── tasks.md                  # /speckit.tasks 产出
```

### Source Code (repository root)

与 008-rhi-fullmodule-005 一致：`include/te/rhi/`、`src/`（device, command_list, resources, pso, sync, swapchain, vulkan, d3d12, d3d11, metal）、`tests/`。本次新增符号在现有头文件与后端实现中**增补**（resources.hpp 增 BufferUsage、device.hpp 增 UpdateBuffer、command_list.hpp 增 SetUniformBuffer）。

**Structure Decision**: 单库 te_rhi；全量 ABI 实现；本次在现有结构上增补 UpdateBuffer、SetUniformBuffer、BufferUsage 及 BufferDesc.usage 语义。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

> **注意**：本小节**只保存相对于现有 `specs/_contracts/008-rhi-ABI.md` 的新增和修改**条目，用于查阅和写回契约。写回时仅将下表增补到 ABI 对应节，并将原「TODO」节移除或改为“已实现”说明。
>
> **实现时使用全量内容**：tasks 和 implement 阶段应基于 **`contracts/008-rhi-ABI-full.md`** 全量实现。

| 操作 | 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|------|--------|----------|------|----------|----------|--------|------|
| 新增 | 008-RHI | te::rhi | BufferUsage | 枚举 | 缓冲用途位 | te/rhi/resources.hpp | `enum class BufferUsage : uint32_t { Vertex = 1u << 0, Index = 1u << 1, Uniform = 1u << 2, Storage = 1u << 3, CopySrc = 1u << 4, CopyDst = 1u << 5 };` BufferDesc.usage 使用此位掩码；CreateUniformBuffer 等须传 Uniform 位 |
| 修改 | 008-RHI | te::rhi | BufferDesc | struct | 缓冲创建描述 | te/rhi/resources.hpp | `size_t size; uint32_t usage;` **usage 语义补充**：为 BufferUsage 位掩码；含 Uniform 时表示可用于 Uniform 缓冲 |
| 新增 | 008-RHI | te::rhi | IDevice::UpdateBuffer | 成员函数 | CPU 写入 GPU 缓冲 | te/rhi/device.hpp | `void UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size) = 0;` 满足 009 UniformBuffer::Update |
| 新增 | 008-RHI | te::rhi | ICommandList::SetUniformBuffer | 成员函数 | 将 IBuffer 绑定到 slot | te/rhi/command_list.hpp | `void SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) = 0;` 满足 009 IUniformBuffer::Bind；slot 越界或 buffer==nullptr 行为由实现约定 |

## Complexity Tracking

无 Constitution 违规需豁免。
