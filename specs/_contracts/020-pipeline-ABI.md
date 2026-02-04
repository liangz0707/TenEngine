# 020-Pipeline 模块 ABI

- **契约**：[020-pipeline-public-api.md](./020-pipeline-public-api.md)（能力与类型描述）
- **本文件**：020-Pipeline 对外 ABI 显式表。
- **渲染模式**：渲染支持 **Debug**、**Hybrid**、**Resource** 三种模式（可通过编译选项或运行时配置选择）；容易错误处使用统一 CheckWarning/CheckError 宏，不使用异常捕获。
- **渲染资源显式控制位置**：**创建逻辑渲染资源**（CreateRenderItem）见 019-PipelineCore；**创建/收集逻辑 CommandBuffer**（CollectCommandBuffer，即 convertToLogicalCommandBuffer）见 019-PipelineCore；**提交到实际 GPU Command**（**SubmitCommandBuffer**，即 submitLogicalCommandBuffer / IDevice::executeLogicalCommandBuffer）见本模块与 008-RHI；**准备渲染资源**（PrepareRenderMaterial、PrepareRenderMesh、prepareRenderResources）见 019-PipelineCore；**创建/更新 GPU 资源**（CreateDeviceResource、UpdateDeviceResource）见 008-RHI。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 一帧渲染入口 | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::RenderFrame | `void RenderFrame(FrameContext const& ctx);` 由主循环或 Editor 每帧调用 |
| 020-Pipeline | TenEngine::pipeline | — | struct | 帧上下文 | TenEngine/pipeline/FrameContext.h | FrameContext | 含 scene_root, camera, render_target, viewport, delta_time；只读，由调用方填充 |
| 020-Pipeline | TenEngine::pipeline | — | 自由函数 | 创建默认渲染管线 | TenEngine/pipeline/RenderPipeline.h | CreateRenderPipeline | `IRenderPipeline* CreateRenderPipeline(RenderPipelineDesc const& desc);` 或 `IRenderPipeline* CreateRenderPipeline(IDevice* device);` 调用方或引擎管理生命周期 |
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 推进流水线一帧（多阶段） | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::TickPipeline | `void TickPipeline(FrameContext const& ctx);` 按 slot 执行：等待 slot 可复用 → 阶段 A 收集物体 → 阶段 B 录制并 submit → 若轮到则 present |
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 获取当前帧 slot | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::GetCurrentSlot | `FrameSlotId GetCurrentSlot() const;` 本帧使用的 slot |
| 020-Pipeline | TenEngine::pipeline | — | struct | 创建管线时的配置 | TenEngine/pipeline/RenderPipeline.h | RenderPipelineDesc | 含 frameInFlightCount、IDevice*、ISwapChain* 等 |
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 设置/获取渲染配置 | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::SetRenderingConfig, GetRenderingConfig | `void SetRenderingConfig(RenderingConfig const& config);` `RenderingConfig GetRenderingConfig() const;` 编辑器下发配置，下一帧生效 |
| 020-Pipeline | TenEngine::pipeline | — | struct | 渲染配置（与 Editor 共用） | TenEngine/pipeline/RenderingConfig.h | RenderingConfig | 渲染路径(Deferred/Forward/FullCompute)、灯光/阴影/太阳/IBL/DOF/抗锯齿等；Editor 与 Pipeline 共用 |
| 020-Pipeline | TenEngine::pipeline | — | 枚举 | 渲染模式 | TenEngine/pipeline/RenderPipeline.h | RenderMode | `enum class RenderMode { Debug, Hybrid, Resource };` Debug=全量校验，Hybrid=部分校验，Resource=发布/最小校验 |
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 获取/设置 FrameGraph | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::GetFrameGraph, SetFrameGraph | `IFrameGraph* GetFrameGraph();` `void SetFrameGraph(IFrameGraph* graph);` 每帧执行时编译并执行 graph 中的 Pass |
| 020-Pipeline | TenEngine::pipeline | — | 自由函数 | 创建 FrameGraph | TenEngine/pipeline/FrameGraph.h | CreateFrameGraph | `IFrameGraph* CreateFrameGraph();` 或由 Pipeline 内部创建；调用方或 Pipeline 管理生命周期 |
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 触发一次渲染（多阶段调度） | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::TriggerRender | `void TriggerRender(FrameContext const& ctx);` 内部：BuildLogicalPipeline(B) → CollectRenderItemsParallel + Merge(C) → 在线程 D：PrepareRenderResources、ConvertToLogicalCommandBuffer、SubmitLogicalCommandBuffer |
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | **提交到实际 GPU Command**（SubmitCommandBuffer，须在线程 D 调用） | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::SubmitLogicalCommandBuffer | `void SubmitLogicalCommandBuffer(ILogicalCommandBuffer* logical_cb);` 即 SubmitCommandBuffer；**必须在线程 D 调用**；将 logical_cb 交给 RHI 在同一线程 D 录制并 submit 到实际 GPU |

*来源：用户故事 US-002（一帧渲染）、US-004（流水线式多帧渲染）、US-editor-001（编辑器内配置渲染设置并保存）、US-rendering-003（FrameGraph AddPass）、US-rendering-004（多线程管线阶段）。*

---

## 数据相关 TODO

（本模块上游：001-Core、004-Scene、005-Entity、019-PipelineCore、009-RenderCore、010-Shader、011-Material、012-Mesh、013-Resource、021-Effects 等。）

### 数据

- [ ] **FrameContext**：含 scene、camera、viewport、frameSlotId
- [ ] 待渲染项来源：004 节点 modelGuid、005 实体 ModelComponent.modelGuid

### 需提供的对外接口

| 接口 | 说明 |
|------|------|
| [ ] `IRenderPipeline::RenderFrame(ctx)` / `TriggerRender(ctx)` | 一帧渲染入口 |
| [ ] `IRenderPipeline::SubmitLogicalCommandBuffer(logical_cb)` | 在线程 D 提交到 GPU |

### 需调用上游

| 场景 | 调用上游接口 |
|------|--------------|
| 取待渲染项 | 004.`GetNodeModelGuid(node)`；005.`GetModelGuid(entity)` |
| 按 GUID 加载 Model | 013.`LoadSync(ResourceId, Model)` 或 `RequestLoadAsync` |
| 收集 RenderItem | 019.`CollectRenderItemsParallel` |
| 准备资源 | 019.`PrepareRenderMaterial`, `PrepareRenderMesh` |
| 提交绘制前 | 013.`IsDeviceReady(resource)`；false 则跳过/等待/占位 |
| Draw 时绑定 Uniform | 009.`IUniformBuffer::Bind(cmd, slot)` 或 008.`SetUniformBuffer` |
| 流式加载（可选） | 013.`RequestStreaming`, `SetStreamingPriority` |

### 调用流程

1. **TriggerRender(ctx)** → 从 004/005 取 modelGuid → 013.LoadSync(modelGuid) 取得 IModelResource* → 构造 FrameContext.scene
2. **019.CollectRenderItemsParallel** → 产出 RenderItem 列表
3. **线程 D**：019.PrepareRenderMaterial/Mesh → 019.ConvertToLogicalCommandBuffer → 录制 Draw（009.Bind / 008.SetUniformBuffer）→ SubmitLogicalCommandBuffer
4. **Draw 前**：013.IsDeviceReady 为 false 则跳过该次绘制或使用占位
