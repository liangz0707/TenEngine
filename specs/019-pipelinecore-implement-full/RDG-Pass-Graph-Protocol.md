# TenEngine RDG 风格 Pass 图协议

**模块**：019-PipelineCore | **日期**：2026-02-03

本文档整理本引擎的 RDG（Render Dependency Graph）风格 Pass 图协议，供实现与下游消费参考。

---

## 1. 协议概览

| 维度 | 本引擎约定 |
|------|------------|
| **参考** | Unreal RDG、Unity SRP RenderGraph |
| **命名空间** | te::pipelinecore |
| **头文件** | te/pipelinecore/*.h |
| **依赖** | 008-RHI、009-RenderCore |

---

## 2. 阶段划分

```
┌─────────────────────────────────────────────────────────────────────────────┐
│  Setup（建图）                                                                │
│  IFrameGraph::AddPass → IPassBuilder 配置 → IFrameGraph::Compile             │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│  线程 B：构建逻辑管线                                                         │
│  BuildLogicalPipeline(graph, FrameContext) → ILogicalPipeline                │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│  线程 C：多线程收集                                                           │
│  CollectRenderItemsParallel → MergeRenderItems → IRenderItemList             │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│  线程 D（GPU/Device 线程）：准备资源与产出命令缓冲                             │
│  PrepareRenderResources → ConvertToLogicalCommandBuffer → ILogicalCommandBuffer│
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│  提交（020-Pipeline 驱动）                                                    │
│  ILogicalCommandBuffer → RHI Submit（见 pipeline-to-rci.md）                  │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 3. Pass 图构建 API

### 3.1 入口与编译

| 符号 | 签名 | 说明 |
|------|------|------|
| IFrameGraph::AddPass | `IPassBuilder* AddPass(char const* name);` | 添加 Pass 节点，返回 Builder |
| IFrameGraph::Compile | `bool Compile();` | 编译为可执行图；含环则返回 false |

### 3.2 Pass 配置（IPassBuilder）

| 方法 | 说明 |
|------|------|
| SetScene(ISceneWorld const*) | 绑定场景；020/004 实现 ISceneWorld |
| SetCullMode(CullMode) | None / FrustumCull / OcclusionCull / FrustumAndOcclusion |
| SetObjectTypeFilter(...) | 物体类型过滤 |
| SetRenderType(RenderType) | Opaque / Transparent / Overlay / Custom |
| SetOutput(PassOutputDesc const&) | 渲染目标、深度、多 RT、分辨率、格式 |
| SetExecuteCallback(PassExecuteCallback) | 执行回调 |

### 3.3 Pass 执行

| 概念 | 说明 |
|------|------|
| PassExecuteCallback | `void (*)(PassContext& ctx, ICommandList* cmd);` |
| PassContext::GetCollectedObjects() | 返回本 Pass 收集到的 IRenderObjectList const* |
| 执行顺序 | Compile 时拓扑排序；按 DeclareRead/DeclareWrite 推导依赖（与 009 PassProtocol 对接） |

---

## 4. 资源声明与生命周期（与 009-RenderCore 对接）

| 能力 | 来源 | 说明 |
|------|------|------|
| PassHandle | 009-RenderCore | 用于 DeclareRead/DeclareWrite |
| ResourceHandle | 009-RenderCore | 管线内资源句柄 |
| DeclareRead | 009-RenderCore | `void DeclareRead(PassHandle pass, ResourceHandle resource);` |
| DeclareWrite | 009-RenderCore | `void DeclareWrite(PassHandle pass, ResourceHandle resource);` |
| ResourceLifetime | 009-RenderCore | Transient / Persistent / External |
| AllocateTransient, Alias, RingBuffer, ReleaseAfterPass | 规约能力 | 由 Pass 图实现与 009 PassProtocol 协同；首版可简化 |

---

## 5. 命令缓冲与 RHI 映射

| 概念 | 说明 |
|------|------|
| ILogicalCommandBuffer | CPU 侧逻辑命令序列（Draw、Bind、Barrier 等） |
| ConvertToLogicalCommandBuffer | `ResultCode ConvertToLogicalCommandBuffer(..., ILogicalCommandBuffer** out);` |
| RecordPass / MapToRHI / InsertBarrier | 规约能力；在 Convert 与 Pass 执行中体现 |
| 提交 | 020-Pipeline 将 ILogicalCommandBuffer 交给 008-RHI；格式见 pipeline-to-rci.md |

---

## 6. 帧与 Slot

| 符号 | 说明 |
|------|------|
| FrameSlotId | uint32_t，[0, frameInFlightCount) |
| PipelineConfig::frameInFlightCount | 2～4 |
| FrameContext | 含 frameSlotId、scene、camera、viewport；020 构造 |

---

## 7. 线程约束

| 线程 | 职责 |
|------|------|
| 线程 B | BuildLogicalPipeline |
| 线程 C | CollectRenderItemsParallel、MergeRenderItems |
| **线程 D** | PrepareRenderResources、PrepareRenderMaterial、PrepareRenderMesh、ConvertToLogicalCommandBuffer；**所有 GPU 操作与资源创建须在线程 D** |

---

## 8. 错误与可观测性

| 项目 | 说明 |
|------|------|
| ResultCode | PrepareRenderResources、PrepareRenderMaterial、PrepareRenderMesh、ConvertToLogicalCommandBuffer 遇 RHI 失败返回 te::rendercore::ResultCode |
| TE_PIPELINECORE_PROFILING | 宏；启用时提供 Pass 耗时、Compile 耗时等 |

---

## 9. 首版范围与延后项

| 首版实现 | Phase 2 延后 |
|----------|--------------|
| 单队列（Graphics） | MultiQueue、async Compute |
| FrameGraph + Pass 配置 + Compile | 完整 RDG 瞬态分配、Split-Barrier 自动插入 |
| 与 009 PassProtocol 对接 | 未使用 Pass/资源剔除、RDG Insights 风格可视化 |
