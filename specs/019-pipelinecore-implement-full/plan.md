# Implementation Plan: 019-PipelineCore 完整模块实现

**Branch**: `019-pipelinecore-implement-full` | **Date**: 2026-02-03 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/019-pipelinecore-implement-full/spec.md`

**Note**: 执行流程见 `.cursor/commands/speckit.plan.md`。本 feature 可充分参考 Unity、Unreal 及上游模块进行设计与实现。

## Summary

实现 019-PipelineCore 完整模块：RDG 风格 Pass 图协议、命令缓冲格式、资源生命周期、与 RHI 提交约定。依赖 008-RHI、009-RenderCore；首版仅支持单队列（Graphics）；错误返回 ResultCode；提供可选 profiling 钩子；FrameContext、ISceneWorld 由本模块定义，020 构造/实现。

## 实现范围（TenEngine：实现全量 ABI 内容）

> **ABI 生成与保存模式**（2.0.1 全量生成、增量保存）：
> - **生成全量 ABI 内容**：参考 spec、Unity/Unreal、上游契约，生成本 feature 需实现的全部 ABI（见 `contracts/019-pipelinecore-ABI-full.md`）。
> - **文档只保存变化部分**：下方「契约更新」仅列新增/修改条目，用于写回。
> - **实现基于全量内容**：tasks 与 implement 必须基于 `contracts/019-pipelinecore-ABI-full.md` 全量实现。

**全量 ABI 符号**（引用 `contracts/019-pipelinecore-ABI-full.md`）：

- Config：kMaxFramesInFlight、PipelineConfig、FrameSlotId
- FrameContext、ViewportDesc（新增）
- ISceneWorld（新增）
- IFrameGraph、IPassBuilder、CullMode、RenderType、PassOutputDesc、PassContext、PassExecuteCallback、IRenderObjectList
- ILogicalPipeline、BuildLogicalPipeline
- RenderItem、IRenderItemList、CreateRenderItem、PrepareRenderResources、PrepareRenderMaterial、PrepareRenderMesh（后三者返回 ResultCode）
- CollectRenderItemsParallel、MergeRenderItems
- ILogicalCommandBuffer、ConvertToLogicalCommandBuffer（返回 ResultCode + out）
- Profiling 钩子（新增）

## Technical Context

**Language/Version**: C++17  
**Primary Dependencies**: 008-RHI (te_rhi)、009-RenderCore (te_rendercore)  
**Storage**: N/A（运行时管线数据结构）  
**Testing**: 单元测试（Pass 图、拓扑排序、资源生命周期）；集成测试（与 008/009 契约行为）  
**Target Platform**: Windows / Linux / macOS（与 008-RHI 后端一致）

**Project Type**: 引擎模块（单库）  
**Performance Goals**: Pass 图 Compile 与执行开销可接受；profiling 可控  
**Constraints**: 线程 D 约束；仅使用 008/009 已声明 API  
**Scale/Scope**: 典型场景 <100 Pass、<1000 RenderItem/帧

## 依赖引入方式（TenEngine 构建规约）

> 规约见 `docs/engine-build-module-convention.md` §3。**当前所有子模块构建均使用源码方式**。

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 008-RHI | **源码** | TenEngineHelpers / tenengine_resolve_my_dependencies 解析；TENENGINE_008_RHI_DIR 或同级 worktree |
| 009-RenderCore | **源码** | 同上；TENENGINE_009_RENDERCORE_DIR |

**说明**：构建根目录为 worktree 根（如 `TenEngine-019-pipeline-core`）；out-of-source 推荐 `build/`。

### 第三方依赖（本 feature 涉及模块所需）

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| 本 feature 无第三方依赖 | — | — | 019-public-api 未声明第三方库 |

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 原则 | 状态 | 说明 |
|------|------|------|
| I. Modular Renderer Architecture | PASS | 019 为独立管线协议模块，边界清晰 |
| II. Modern Graphics API First | PASS | 通过 RHI 抽象，不泄露后端 |
| III. Data-Driven Pipeline | PASS | Pass 图、配置可数据驱动 |
| IV. Performance & Observability | PASS | 提供 profiling 钩子 |
| V. Versioning & ABI Stability | PASS | 遵循契约版本化 |
| VI. Module Boundaries & Contract-First | PASS | 仅用 008/009 已声明 API；实现全量 ABI；构建引入真实子模块源码 |

## Project Structure

### Documentation (this feature)

```text
specs/019-pipelinecore-implement-full/
├── plan.md                    # 本文件
├── research.md                # Phase 0 产出
├── data-model.md              # Phase 1 产出
├── quickstart.md              # Phase 1 产出
├── contracts/                 # Phase 1 产出
│   └── 019-pipelinecore-ABI-full.md   # 全量 ABI（实现用）
├── RDG-Pass-Graph-Protocol.md # RDG 风格 Pass 图协议整理
├── consistency-verification.md# ABI-full / 契约 ABI / plan 一致性验证
├── checklists/
│   └── requirements.md
└── tasks.md                   # Phase 2 产出（/speckit.tasks）
```

### Source Code (repository root)

```text
TenEngine-019-pipeline-core/
├── CMakeLists.txt
├── include/
│   └── te/
│       └── pipelinecore/
│           ├── Config.h
│           ├── FrameContext.h
│           ├── FrameGraph.h
│           ├── LogicalPipeline.h
│           ├── RenderItem.h
│           ├── CollectPass.h
│           ├── LogicalCommandBuffer.h
│           └── Profiling.h
├── src/
│   ├── FrameGraph.cpp
│   ├── LogicalPipeline.cpp
│   ├── RenderItem.cpp
│   ├── CollectPass.cpp
│   └── LogicalCommandBuffer.cpp
├── tests/
│   ├── unit/
│   └── integration/
└── deps/                # 可选，上游源码路径
```

**Structure Decision**: 单库结构；公开头在 `include/te/pipelinecore/`（与 008 `te/rhi/`、009 `te/rendercore/` 风格一致）；命名空间 `te::pipelinecore`。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

> **注意**：本小节只保存相对于现有 ABI 的新增和修改条目。写回时仅将下表增补或替换到 `specs/_contracts/019-pipelinecore-ABI.md`。
>
> **实现时使用全量内容**：见 `contracts/019-pipelinecore-ABI-full.md`。
>
> **命名空间对齐**：参考上游 008-RHI（te::rhi）、009-RenderCore（te::rendercore），019 命名空间改为 **te::pipelinecore**，头文件路径 **te/pipelinecore/**；全表命名空间与头文件须同步更新。

| 操作 | 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|------|--------|----------|------|----------|--------|------|------|
| 修改 | 019-PipelineCore | te::pipelinecore | — | 全局 | — | 命名空间/头文件 | 命名空间 TenEngine::pipelinecore → te::pipelinecore；头文件 TenEngine/pipelinecore/ → te/pipelinecore/；全表同步更新 |
| 新增 | 019-PipelineCore | te::pipelinecore | — | struct | te/pipelinecore/FrameContext.h | FrameContext | scene (ISceneWorld const*), camera, viewport, frameSlotId 等；020 构造 |
| 新增 | 019-PipelineCore | te::pipelinecore | — | struct | te/pipelinecore/FrameContext.h | ViewportDesc | width, height 等视口参数 |
| 新增 | 019-PipelineCore | te::pipelinecore | ISceneWorld | 抽象接口 | te/pipelinecore/FrameGraph.h | ISceneWorld | 最小接口；020/004 实现 |
| 修改 | 019-PipelineCore | te::pipelinecore | — | 自由函数 | te/pipelinecore/RenderItem.h | PrepareRenderResources | `ResultCode PrepareRenderResources(IRenderItemList const* items, IDevice* device);` |
| 修改 | 019-PipelineCore | te::pipelinecore | — | 自由函数 | te/pipelinecore/RenderItem.h | PrepareRenderMaterial | `ResultCode PrepareRenderMaterial(IMaterialHandle const* material, IDevice* device);` |
| 修改 | 019-PipelineCore | te::pipelinecore | — | 自由函数 | te/pipelinecore/RenderItem.h | PrepareRenderMesh | `ResultCode PrepareRenderMesh(IMeshHandle const* mesh, IDevice* device);` |
| 修改 | 019-PipelineCore | te::pipelinecore | — | 自由函数 | te/pipelinecore/LogicalCommandBuffer.h | ConvertToLogicalCommandBuffer | `ResultCode ConvertToLogicalCommandBuffer(IRenderItemList const* items, ILogicalPipeline const* pipeline, ILogicalCommandBuffer** out);` |
| 新增 | 019-PipelineCore | te::pipelinecore | — | 宏/回调 | te/pipelinecore/Profiling.h | TE_PIPELINECORE_PROFILING | 启用 profiling 时定义 |
| 新增 | 019-PipelineCore | te::pipelinecore | PassProfilingScope | struct | te/pipelinecore/Profiling.h | PassProfilingScope | RAII Pass 计时 |
| 新增 | 019-PipelineCore | te::pipelinecore | — | 回调 | te/pipelinecore/Profiling.h | OnCompileProfiling | Compile 耗时回调 |

**ResultCode**：使用 009-RenderCore 的 `te::rendercore::ResultCode`。

## Complexity Tracking

（无违规需豁免）
