# 契约：019-PipelineCore 模块对外 API

## 适用模块

- **实现方**：019-PipelineCore（L3；命令缓冲格式、Pass 图协议（RDG 风格）、与 RHI 提交约定）
- **对应规格**：`docs/module-specs/019-pipeline-core.md`
- **依赖**：008-RHI、009-RenderCore

## 消费者

- 020-Pipeline、021-Effects

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| PassGraphBuilder | Pass 图构建器；AddPass、DeclareRead、DeclareWrite、TopologicalSort、ExecuteOrder | 单次图构建周期 |
| PassContext | Pass 执行上下文；RecordPass、资源访问、与 RHI 命令列表映射 | 单次 Pass 执行 |
| ResourceHandle | 管线内资源句柄；AllocateTransient、Alias、RingBuffer、ReleaseAfterPass | 与 Pass 图资源生命周期一致 |
| SubmitContext | 提交上下文；SubmitQueue、SyncPoint、与 RHI 队列对接 | 单次提交周期 |
| CreateRenderItem / PrepareRenderMaterial / PrepareRenderMesh | 创建逻辑渲染资源、准备材质/网格；CollectCommandBuffer 产出逻辑命令缓冲 | 单帧或单次收集 |

命令缓冲最终通过 RHI 提交；格式与提交约定见 `pipeline-to-rci.md`。

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | PassGraph | AddPass、DeclareRead、DeclareWrite、TopologicalSort、ExecuteOrder；RDG 风格与 RenderCore Pass 协议一致 |
| 2 | ResourceLifetime | AllocateTransient、Alias、RingBuffer、ReleaseAfterPass；与 RHI 屏障协同 |
| 3 | CommandFormat | RecordPass、MapToRHI、InsertBarrier；命令缓冲抽象与 RHI 命令列表映射 |
| 4 | Submit | SubmitQueue、SyncPoint、MultiQueue（可选）；与 RHI 队列、同步点约定 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 RHI、RenderCore 初始化之后使用。Pass 资源声明须与 RenderCore PassProtocol 一致。资源屏障与生命周期不得违反 RHI 要求。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 019-PipelineCore 契约 |
| 2026-02-05 | 统一目录；能力列表用表格；去除 ABI 引用 |
