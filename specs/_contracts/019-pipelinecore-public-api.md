# 契约：019-PipelineCore 模块对外 API

## 适用模块

- **实现方**：**019-PipelineCore**（管线协议与命令缓冲约定，RDG 风格）
- **对应规格**：`docs/module-specs/019-pipeline-core.md`
- **依赖**：008-RHI（008-rhi-public-api）、009-RenderCore（009-rendercore-public-api）

## 消费者（T0 下游）

- 020-Pipeline（Pass 图构建、命令缓冲生成、提交）
- 021-Effects（Pass 注册、资源声明、执行顺序）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| PassGraphBuilder | Pass 图构建器；AddPass、DeclareRead、DeclareWrite、TopologicalSort、ExecuteOrder | 单次图构建周期 |
| PassContext | Pass 执行上下文；RecordPass、资源访问、与 RHI 命令列表映射 | 单次 Pass 执行 |
| ResourceHandle | 管线内资源句柄；AllocateTransient、Alias、RingBuffer、ReleaseAfterPass | 与 Pass 图资源生命周期一致 |
| SubmitContext | 提交上下文；SubmitQueue、SyncPoint、与 RHI 队列对接、多队列（可选） | 单次提交周期 |

下游仅通过上述类型与句柄访问；命令缓冲最终通过 RHI 提交，具体格式与提交约定见契约 `pipeline-to-rci.md`。

## 能力列表（提供方保证）

1. **PassGraph**：AddPass、DeclareRead、DeclareWrite、TopologicalSort、ExecuteOrder；RDG 风格 Pass 图与 RenderCore Pass 协议一致。
2. **ResourceLifetime**：AllocateTransient、Alias、RingBuffer、ReleaseAfterPass；资源创建/使用/释放时机、与 RHI 屏障协同。
3. **CommandFormat**：RecordPass、MapToRHI、InsertBarrier；命令缓冲抽象与 RHI 命令列表映射。
4. **Submit**：SubmitQueue、SyncPoint、MultiQueue（可选）；提交时机与 RHI 队列、同步点约定。

## 调用顺序与约束

- 须在 RHI、RenderCore 初始化之后使用；Pass 资源声明须与 RenderCore PassProtocol 一致。
- 资源屏障与生命周期不得违反 RHI 要求；Pipeline（020）产出的命令缓冲通过 `pipeline-to-rci.md` 交给 RHI。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 019-PipelineCore 模块规格与依赖表新增契约；类型与能力与 docs/module-specs/019-pipeline-core.md 一致 |
