# 020-Pipeline 模块 ABI

- **契约**：[020-pipeline-public-api.md](./020-pipeline-public-api.md)（能力与类型描述）
- **本文件**：020-Pipeline 对外 ABI 显式表。
- **渲染模式**：渲染支持 **Debug**、**Hybrid**、**Resource** 三种模式（可通过编译选项或运行时配置选择）；容易错误处使用统一 CheckWarning/CheckError 宏，不使用异常捕获。
- **渲染资源显式控制位置**：**创建逻辑渲染资源**（CreateRenderItem）见 019-PipelineCore；**创建/收集逻辑 CommandBuffer**（CollectCommandBuffer，即 convertToLogicalCommandBuffer）见 019-PipelineCore；**提交到实际 GPU Command**（**SubmitCommandBuffer**，即 submitLogicalCommandBuffer / IDevice::executeLogicalCommandBuffer）见本模块与 008-RHI；**准备渲染资源**（PrepareRenderMaterial、PrepareRenderMesh、prepareRenderResources）见 019-PipelineCore；**创建/更新 GPU 资源**（CreateDeviceResource、UpdateDeviceResource）见 008-RHI。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 020-Pipeline | te::pipeline | IRenderPipeline | 抽象接口 | 一帧渲染入口 | te/pipeline/RenderPipeline.h | IRenderPipeline::RenderFrame | `void RenderFrame(FrameContext const& ctx);` 由主循环或 Editor 每帧调用 |
| 020-Pipeline | te::pipeline | — | struct | 帧上下文 | te/pipeline/FrameContext.h | FrameContext | 含 scene_root, camera, render_target, viewport, delta_time；只读，由调用方填充 |
| 020-Pipeline | te::pipeline | — | 自由函数 | 创建默认渲染管线 | te/pipeline/RenderPipeline.h | CreateRenderPipeline | `IRenderPipeline* CreateRenderPipeline(RenderPipelineDesc const& desc);` 或 `IRenderPipeline* CreateRenderPipeline(IDevice* device);` 调用方或引擎管理生命周期 |
| 020-Pipeline | te::pipeline | IRenderPipeline | 抽象接口 | 推进流水线一帧（多阶段） | te/pipeline/RenderPipeline.h | IRenderPipeline::TickPipeline | `void TickPipeline(FrameContext const& ctx);` 按 slot 执行：等待 slot 可复用 → 阶段 A 收集物体 → 阶段 B 录制并 submit → 若轮到则 present |
| 020-Pipeline | te::pipeline | IRenderPipeline | 抽象接口 | 获取当前帧 slot | te/pipeline/RenderPipeline.h | IRenderPipeline::GetCurrentSlot | `FrameSlotId GetCurrentSlot() const;` 本帧使用的 slot |
| 020-Pipeline | te::pipeline | — | struct | 创建管线时的配置 | te/pipeline/RenderPipeline.h | RenderPipelineDesc | 含 frameInFlightCount、IDevice*、ISwapChain* 等 |
| 020-Pipeline | te::pipeline | IRenderPipeline | 抽象接口 | 设置/获取渲染配置 | te/pipeline/RenderPipeline.h | IRenderPipeline::SetRenderingConfig, GetRenderingConfig | `void SetRenderingConfig(RenderingConfig const& config);` `RenderingConfig GetRenderingConfig() const;` 编辑器下发配置，下一帧生效 |
| 020-Pipeline | te::pipeline | — | struct | 渲染配置（与 Editor 共用） | te/pipeline/RenderingConfig.h | RenderingConfig | 渲染路径(Deferred/Forward/FullCompute)、灯光/阴影/太阳/IBL/DOF/抗锯齿等；Editor 与 Pipeline 共用 |
| 020-Pipeline | te::pipeline | — | 枚举 | 渲染模式 | te/pipeline/RenderPipeline.h | RenderMode | `enum class RenderMode { Debug, Hybrid, Resource };` Debug=全量校验，Hybrid=部分校验，Resource=发布/最小校验 |
| 020-Pipeline | te::pipeline | IRenderPipeline | 抽象接口 | 获取/设置 FrameGraph | te/pipeline/RenderPipeline.h | IRenderPipeline::GetFrameGraph, SetFrameGraph | `IFrameGraph* GetFrameGraph();` `void SetFrameGraph(IFrameGraph* graph);` 每帧执行时编译并执行 graph 中的 Pass |
| 020-Pipeline | te::pipeline | — | 自由函数 | 创建 FrameGraph | te/pipeline/FrameGraph.h | CreateFrameGraph | `IFrameGraph* CreateFrameGraph();` 或由 Pipeline 内部创建；调用方或 Pipeline 管理生命周期 |
| 020-Pipeline | te::pipeline | IRenderPipeline | 抽象接口 | 触发一次渲染（多阶段调度） | te/pipeline/RenderPipeline.h | IRenderPipeline::TriggerRender | `void TriggerRender(FrameContext const& ctx);` 内部：BuildLogicalPipeline(B) → CollectRenderItemsParallel + Merge(C) → 在线程 D：PrepareRenderResources、ConvertToLogicalCommandBuffer、SubmitLogicalCommandBuffer |
| 020-Pipeline | te::pipeline | IRenderPipeline | 抽象接口 | **提交到实际 GPU Command**（SubmitCommandBuffer，须在线程 D 调用） | te/pipeline/RenderPipeline.h | IRenderPipeline::SubmitLogicalCommandBuffer | `void SubmitLogicalCommandBuffer(ILogicalCommandBuffer* logical_cb);` 即 SubmitCommandBuffer；**必须在线程 D 调用**；将 logical_cb 交给 RHI 在同一线程 D 录制并 submit 到实际 GPU |

*来源：用户故事 US-002（一帧渲染）、US-004（流水线式多帧渲染）、US-editor-001（编辑器内配置渲染设置并保存）、US-rendering-003（FrameGraph AddPass）、US-rendering-004（多线程管线阶段）。*

---

数据与接口 TODO 已迁移至本模块契约 [020-pipeline-public-api.md](./020-pipeline-public-api.md) 的 TODO 列表；本文件仅保留 ABI 表与实现说明。
