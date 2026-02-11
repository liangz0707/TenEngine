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
| 020-Pipeline | te::pipeline | — | struct | 渲染配置（与 Editor 共用） | te/pipeline/RenderingConfig.h | RenderingConfig | 渲染路径(Deferred/Forward/FullCompute)、**validationLevel**（ValidationLevel：Debug/Hybrid/Resource）、灯光/阴影/太阳/IBL/DOF/抗锯齿等；Editor 与 Pipeline 共用 |
| 020-Pipeline | te::pipeline | — | 枚举 | 校验程度 | te/pipeline/RenderingConfig.h | ValidationLevel | `enum class ValidationLevel { Debug, Hybrid, Resource };` 与 RenderMode 对应；控制 CheckWarning/CheckError 行为 |
| 020-Pipeline | te::pipeline | — | 自由函数 | 按配置报告警告/错误 | te/pipeline/RenderingConfig.h | CheckWarning, CheckError | `void CheckWarning(RenderingConfig const& config, char const* msg);` `void CheckError(RenderingConfig const& config, char const* msg);` Debug/Hybrid 可打日志 |
| 020-Pipeline | te::pipeline | — | 枚举 | 渲染模式 | te/pipeline/RenderPipeline.h | RenderMode | `enum class RenderMode { Debug, Hybrid, Resource };` Debug=全量校验，Hybrid=部分校验，Resource=发布/最小校验 |
| 020-Pipeline | te::pipeline | IRenderPipeline | 抽象接口 | 获取/设置 FrameGraph | te/pipeline/RenderPipeline.h | IRenderPipeline::GetFrameGraph, SetFrameGraph | `IFrameGraph* GetFrameGraph();` `void SetFrameGraph(IFrameGraph* graph);` 每帧执行时编译并执行 graph 中的 Pass |
| 020-Pipeline | te::pipeline | — | 自由函数 | 创建 FrameGraph | te/pipeline/FrameGraph.h | CreateFrameGraph | `IFrameGraph* CreateFrameGraph();` 或由 Pipeline 内部创建；调用方或 Pipeline 管理生命周期 |
| 020-Pipeline | te::pipeline | IRenderPipeline | 抽象接口 | 触发一次渲染（多阶段调度） | te/pipeline/RenderPipeline.h | IRenderPipeline::TriggerRender | `void TriggerRender(FrameContext const& ctx);` 主线程推进 slot、获取/创建 FrameGraph 后，**阶段 A 投递到 Render 线程**：BuildLogicalPipeline、CreateRenderItemList、CollectRenderablesToRenderItemList（可选 cameraPositionWorld 用于 012 SelectLOD）、RequestStreaming；**阶段 B 由 Render 线程投递到 Device 线程**：PrepareRenderResources、IsDeviceReady 过滤、ConvertToLogicalCommandBuffer、按 Pass BeginRenderPass/ExecutePass/EndRenderPass、OcclusionQuery 占位、录制 Draw、Submit、Present |
| 020-Pipeline | te::pipeline | IRenderPipeline | 抽象接口 | **提交到实际 GPU Command**（SubmitCommandBuffer，须在线程 D 调用） | te/pipeline/RenderPipeline.h | IRenderPipeline::SubmitLogicalCommandBuffer | `void SubmitLogicalCommandBuffer(ILogicalCommandBuffer* logical_cb);` 即 SubmitCommandBuffer；**必须在线程 D 调用**；将 logical_cb 交给 RHI 在同一线程 D 录制并 submit 到实际 GPU |

*来源：用户故事 US-002（一帧渲染）、US-004（流水线式多帧渲染）、US-editor-001（编辑器内配置渲染设置并保存）、US-rendering-003（FrameGraph AddPass）、US-rendering-004（多线程管线阶段）。*

---

## 实现说明

- **待渲染项来源**：由 **029-World WorldManager::CollectRenderables** 提供；Pipeline 经此接口获取 RenderableItem 列表，不再直接使用 004 GetNodeModelGuid / 005 GetModelGuid。依赖 029-World 模块。
- **收集与 LOD**：`CollectRenderablesToRenderItemList(sceneRef, resourceManager, out, frustum, cameraPositionWorld)`（te/pipeline/detail/RenderableCollector.h）；**cameraPositionWorld** 为可选 `float const*`（世界坐标），非空时用于 012 SelectLOD，仅对选中 LOD 的 submesh 生成 RenderItem。
- **数据流**：Render 任务收集后对每项 mesh 调用 013 RequestStreaming(mesh->GetResourceId(), 0)；Device 任务 Prepare 后按 IResource::IsDeviceReady 过滤，仅对 ready 项 Convert 并录制；每条 Draw 前按材质调用 **matRes->UpdateDescriptorSetForFrame(rhiDevice, frameSlot)**、008 **SetGraphicsPSO(matRes->GetGraphicsPSO())**、008 **BindDescriptorSet(matRes->GetDescriptorSet())**（UB 已写入 descriptor set，不再单独 ub->Bind）；ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB, **frameSlot**) 与 SubmitLogicalCommandBuffer 传入当前帧 slot；按 Pass 调用 008 BeginOcclusionQuery/EndOcclusionQuery（占位）。
- 数据与接口 TODO 已迁移至本模块契约 [020-pipeline-public-api.md](./020-pipeline-public-api.md) 的 TODO 列表；本文件仅保留 ABI 表与实现说明。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| 2026-02-10 | ABI 同步：RenderingConfig 增加 ValidationLevel、CheckWarning/CheckError；TriggerRender 改为 Render 线程→Device 线程投递；实现说明补充 CollectRenderablesToRenderItemList(cameraPositionWorld)、013/009 数据流、OcclusionQuery 占位 |
| 2026-02-10 | 渲染管线完备化：ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB, frameSlot)；每 draw 调用 UpdateDescriptorSetForFrame、SetGraphicsPSO、BindDescriptorSet；SubmitLogicalCommandBuffer 传入 currentSlot_ 执行 |
| 2026-02-11 | BuiltinMeshes（te/pipeline/BuiltinMeshes.h）、BuiltinMaterials（te/pipeline/BuiltinMaterials.h）；RenderableCollector 增加 CollectLightsToLightItemList、CollectCamerasToCameraItemList、CollectReflectionProbesToReflectionProbeItemList、CollectDecalsToDecalItemList；TriggerRender 收集 LightItemList、PassContext SetLightItemList、按 PassKind 仅 Scene Pass 录制 logicalCB、LightItemList 生命周期 DestroyLightItemList |
