# 一致性验证：019-pipelinecore-ABI-full、契约 ABI、plan

**日期**：2026-02-03

## 1. 三文件对照

| 维度 | contracts/019-pipelinecore-ABI-full.md | specs/_contracts/019-pipelinecore-ABI.md | plan.md |
|------|----------------------------------------|------------------------------------------|---------|
| 命名空间 | te::pipelinecore ✓ | te::pipelinecore ✓ | te::pipelinecore ✓ |
| 头文件路径 | te/pipelinecore/ ✓ | te/pipelinecore/ ✓ | te/pipelinecore/ ✓ |
| CMake Target | te_pipelinecore ✓ | te_pipelinecore ✓ | — |
| Config | kMaxFramesInFlight, PipelineConfig, FrameSlotId ✓ | 同左 ✓ | 引用 ✓ |
| FrameContext | 有 ✓ | 有 ✓ | 契约更新 新增 ✓ |
| ViewportDesc | 有 ✓ | 有 ✓ | 契约更新 新增 ✓ |
| ISceneWorld | 有 ✓ | 有 ✓ | 契约更新 新增 ✓ |
| IFrameGraph, IPassBuilder | 有 ✓ | 有 ✓ | 引用 ✓ |
| CullMode, RenderType, PassOutputDesc | 有 ✓ | 有 ✓ | 引用 ✓ |
| PassContext, PassExecuteCallback | 有 ✓ | 有 ✓ | 引用 ✓ |
| IRenderObjectList | 有 ✓ | 有 ✓ | 引用 ✓ |
| ILogicalPipeline, BuildLogicalPipeline | 有 ✓ | 有 ✓ | 引用 ✓ |
| RenderItem, IRenderItemList | 有 ✓ | 有 ✓ | 引用 ✓ |
| CreateRenderItem | 有 ✓ | 有 ✓ | 引用 ✓ |
| PrepareRenderResources | ResultCode 返回 ✓ | ResultCode 返回 ✓ | 契约更新 修改 ✓ |
| PrepareRenderMaterial | ResultCode 返回 ✓ | ResultCode 返回 ✓ | 契约更新 修改 ✓ |
| PrepareRenderMesh | ResultCode 返回 ✓ | ResultCode 返回 ✓ | 契约更新 修改 ✓ |
| CollectRenderItemsParallel, MergeRenderItems | 有 ✓ | 有 ✓ | 引用 ✓ |
| ILogicalCommandBuffer | 有 ✓ | 有 ✓ | 引用 ✓ |
| ConvertToLogicalCommandBuffer | ResultCode + out 参数 ✓ | ResultCode + out 参数 ✓ | 契约更新 修改 ✓ |
| Profiling | TE_*, PassProfilingScope, OnCompileProfiling ✓ | 同左 ✓ | 契约更新 新增 ✓ |

## 2. 一致性结论

**结论**：三文件在符号、命名空间、头文件路径、函数签名上**一致**。

| 检查项 | 状态 |
|--------|------|
| ABI-full 与契约 ABI 符号一致 | ✓ |
| plan 契约更新表覆盖全部新增/修改 | ✓ |
| 命名空间 te::pipelinecore 三处统一 | ✓ |
| 头文件 te/pipelinecore/ 三处统一 | ✓ |
| ResultCode 返回签名一致 | ✓ |
| ConvertToLogicalCommandBuffer out 参数一致 | ✓ |

## 3. public-api 与 ABI 的对应关系

| public-api 类型/能力 | ABI 体现 |
|----------------------|----------|
| PassGraphBuilder | IFrameGraph + IPassBuilder |
| PassContext | PassContext |
| ResourceHandle | 使用 009-RenderCore 的 ResourceHandle |
| SubmitContext | 规约概念；020 与 RHI 对接 |
| DeclareRead, DeclareWrite | 009-RenderCore pass_protocol；019 与 009 对接 |
| AddPass, TopologicalSort, ExecuteOrder | IFrameGraph::AddPass、Compile、Pass 执行 |

## 4. 已知差异与说明

| 项目 | 说明 |
|------|------|
| DeclareRead/DeclareWrite | 规约与 public-api 列出，但 ABI 未单独列出；由 009 PassProtocol 提供，019 在实现中调用 |
| ResourceLifetime（AllocateTransient 等） | 规约子模块能力；首版可通过 Pass 配置与 009 协同实现，Phase 2 可细化 ABI |
| ABI-full 表格式 | 简化列（符号/导出形式/说明）；契约 ABI 为完整 8 列；内容等价 |
