# Implementation Plan: 009-RenderCore 完整模块与 Shader Reflection 对接

**Branch**: `009-render-core-shader-reflection-abi` | **Date**: 2026-02-03 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/009-render-core-shader-reflection-abi/spec.md`

**Note**: 执行流程见 `.cursor/commands/speckit.plan.md`。

## Summary

实现 009-RenderCore 完整模块（ResourceDesc、UniformLayout、PassProtocol、UniformBuffer），并完成 ABI 中「TODO（010-Shader 反射对接）」所列项：确保 CreateUniformLayout 接受 010-Shader GetReflection 产出的 UniformLayoutDesc 格式，成员类型映射、偏移、命名与 010-Shader 约定一致。实现仅使用 001-Core、008-RHI 契约已声明的类型与 API；构建通过 add_subdirectory 引入 008-RHI 源码。

## 实现范围（TenEngine：实现全量 ABI 内容）

> **ABI 生成与保存模式**：见 `.cursor/commands/speckit.plan.md`。全量 ABI 内容见 `contracts/009-rendercore-ABI-full.md`，供 tasks 与 implement 参考。
>
> **规约**：本 feature 只实现 ABI 文件中列出的符号与能力；不得设计或实现 ABI 未声明的对外接口。

**全量 ABI 符号**（原始 ABI，本 feature 均需实现）：

- **types.hpp**：ResultCode, PassHandle, ResourceHandle, FrameSlotId, ResourceLifetime, BindSlot
- **resource_desc.hpp**：VertexAttributeFormat, VertexAttribute, VertexFormat, VertexFormatDesc, IndexType, IndexFormat, IndexFormatDesc, TextureFormat, TextureUsage, TextureDesc, TextureDescParams, BufferUsage, BufferDesc, BufferDescParams, CreateVertexFormat, CreateIndexFormat, CreateTextureDesc, CreateBufferDesc
- **uniform_layout.hpp**：UniformMemberType, UniformMember, UniformLayoutDesc, IUniformLayout, CreateUniformLayout, ReleaseUniformLayout
- **pass_protocol.hpp**：PassResourceDecl, DeclareRead, DeclareWrite, SetResourceLifetime
- **uniform_buffer.hpp**：IUniformBuffer, CreateUniformBuffer, ReleaseUniformBuffer
- **api.hpp**：聚合头

**ABI TODO 实现**（额外范围）：

- [ ] 实现 te/rendercore/uniform_layout.hpp 及 CreateUniformLayout/ReleaseUniformLayout
- [ ] 与 010-Shader GetReflection 产出的 UniformLayoutDesc 格式对齐（成员类型映射、偏移、命名）

## Technical Context

**Language/Version**: C++17  
**Primary Dependencies**: 001-Core（内存、数学、日志）、008-RHI（IDevice、IBuffer、ICommandList、CreateBuffer、UpdateBuffer、SetUniformBuffer、BufferUsage::Uniform）  
**Storage**: N/A（本模块不持久化；描述符与句柄由调用方或 RHI 管理）  
**Testing**: 单元测试（ResourceDesc、UniformLayout、PassProtocol、UniformBuffer）；契约测试（与 RHI 对接）  
**Target Platform**: Windows、Linux、macOS（与 008-RHI 一致）

**Project Type**: 渲染引擎中间层模块  
**Performance Goals**: Uniform 布局与资源描述为轻量级；Update/Bind 为热路径，无额外分配  
**Constraints**: 单线程使用；不直接持有 GPU 资源；仅使用上游契约声明的 API

## 依赖引入方式（TenEngine 构建规约）

> **规约**：见 `docs/engine-build-module-convention.md`。当前所有子模块构建均使用源码方式。

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 008-RHI | **源码** | add_subdirectory 引入 008-RHI 源码；008-RHI 通过 TenEngineHelpers 引入 001-Core |
| 001-Core | 源码（经 008-RHI） | 008-RHI 的 tenengine_resolve_my_dependencies 解析；本模块不直接 add_subdirectory |

**说明**：本模块仅 add_subdirectory(008-RHI)；008-RHI 同级需有 001-Core（如 TenEngine-001-core）。构建根目录：worktree 根（如 `TenEngine-009-render-core`）；out-of-source build 至 `build/`。

### 第三方依赖（本 feature 涉及模块所需）

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| 本 feature 无第三方依赖 | — | — | 009-RenderCore 契约未声明第三方库 |

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 检查项 | 状态 |
|--------|------|
| §VI 模块边界：仅实现契约/ABI 声明的接口 | 通过 |
| §VI 完整 ABI 实现：实现 ABI 中全部符号 | 通过 |
| §VI 构建引入真实子模块：add_subdirectory(008-RHI) | 通过 |
| §VI 禁止 stub/占位：无 no-op、TODO 占位 | 通过 |
| §VI 契约更新：接口变更在 ABI 中增补；下游所需在上游 ABI 以 TODO 登记 | 通过（本次实现 ABI 已有 TODO，完成后更新 TODO 小节） |

## Project Structure

### Documentation (this feature)

```text
specs/009-render-core-shader-reflection-abi/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── README.md
│   └── 009-rendercore-ABI-full.md   # 全量 ABI，实现参考
└── tasks.md             # /speckit.tasks 产出
```

### Source Code (repository root)

```text
include/te/rendercore/
├── api.hpp
├── types.hpp
├── resource_desc.hpp
├── uniform_layout.hpp
├── pass_protocol.hpp
└── uniform_buffer.hpp

src/render_core/
├── resource_desc.cpp
├── uniform_layout.cpp
├── pass_protocol.cpp
└── uniform_buffer.cpp

tests/
├── unit/
│   └── test_render_core.cpp
└── contract/
    └── test_rhi_integration.cpp
```

**Structure Decision**: 单模块 C++ 库；公开头在 include/te/rendercore/，实现因此前 refactor 已就绪；本 feature 重点为 Shader Reflection 对接验证与 ABI TODO 完成。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

> **注意**：本小节只保存相对于现有 ABI 的新增和修改条目。本次**无新增 ABI 符号**；**修改**为：实现完成后需更新 ABI 文件中「TODO（010-Shader 反射对接）」小节。
>
> **实现时使用全量内容**：tasks 和 implement 阶段基于 `contracts/009-rendercore-ABI-full.md` 全量 ABI 实现。

| 操作 | 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|------|--------|----------|------|----------|--------|------|------|
| （无新增符号） | — | — | — | — | — | — | — |
| 修改 | 009-RenderCore | — | — | TODO 小节 | specs/_contracts/009-rendercore-ABI.md | — | 实现完成后：将「TODO（010-Shader 反射对接）」中 - [ ] 改为 - [x]，并将该小节改为「已实现」或移除 |

## Complexity Tracking

（无 Constitution 违规需豁免）
