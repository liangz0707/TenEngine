# 020-Pipeline 待办与可改进项

本文档整合 **020 相关所有文档** 中的待办与可改进项，便于按优先级推进。已实现部分见 [README.md](README.md)、[docs/pipeline-rendering-dataflow.md](../../docs/pipeline-rendering-dataflow.md)。契约见 `specs/_contracts/020-pipeline-public-api.md`、`020-pipeline-ABI.md`、`pipeline-to-rci.md`。

---

## 一、待办（TODO）

### 1. 高优先级 — 管线闭环与正确性

| 编号 | 项 | 来源 | 说明 |
|------|----|------|------|
| T1.1 | **019 Prepare* 真实 Ensure** | README, TODO.md §1.2 | 019 的 `PrepareRenderMaterial` / `PrepareRenderMesh` 当前为空；若 011 提供带 device 的 Ensure 或 SetDevice，在 019 内补充材质/贴图 Ensure；020 侧 mesh Ensure 已保留。 |
| T1.2 | **BeginRenderPass / PSO / 材质绑定** | README, TODO.md §1.1 | BeginRenderPass、PSO 绑定、SetUniformBuffer、SetVertexBuffer/SetIndexBuffer（008 当前无此接口时先占位）；每条 Draw 前 009 Bind、008 SetGraphicsPSO（材质可后续暴露 GetGraphicsPSO）。 |
| T1.3 | **按 Pass 执行 ExecuteCallback** | TODO.md §2.5 | 在 019 或 020 中按 Pass 顺序调用各 Pass 的 ExecuteCallback（传入 PassContext 与 ICommandList），在回调内根据 GetCollectedObjects() 或 IRenderItemList 录制该 Pass 的绘制；与 BeginRenderPass/EndRenderPass、RT 绑定配合。 |

### 2. 中优先级 — 能力与契约对齐

| 编号 | 项 | 来源 | 说明 |
|------|----|------|------|
| T2.1 | **LOD 选择** | TODO.md §2.4, 契约能力 1 | 012 SelectLOD 在收集阶段按距离/屏占比选择 LOD；`CollectRenderablesToRenderItemList` 已支持可选 `cameraPositionWorld`，待在 012 侧接入。 |
| T2.2 | **合批与实例化** | TODO.md §2.4, 契约能力 2 | BuildBatches、MaterialSlot、MeshSlot、Transform、Instancing、MergeBatch；按 material/mesh 分组、instanced draw。 |
| T2.3 | **遮挡剔除（可选）** | TODO.md §2.4, 契约能力 1 | OcclusionQuery 或 GPU 遮挡查询在收集阶段或单独 Pass 中接入；当前按 Pass 有 BeginOcclusionQuery/EndOcclusionQuery 占位。 |
| T2.4 | **Render 线程投递** | README, TODO.md §2.6, dataflow | 将「BuildLogicalPipeline + CollectRenderablesToRenderItemList」投递到独立 Render 线程，完成后再将 IRenderItemList 与 ILogicalPipeline 投递到 Device 线程。 |

### 3. 契约与文档闭环

| 编号 | 项 | 来源 | 说明 |
|------|----|------|------|
| T3.1 | **001-Core 契约适配** | docs/module-specs/020-pipeline.md 待办 | 随 001-Core 契约变更做适配（契约变更日期：2026-01-29）。 |
| T3.2 | **004-Scene 契约适配** | docs/module-specs/020-pipeline.md 待办 | 随 004-Scene 契约变更做适配（契约变更日期：2026-01-29）。 |
| T3.3 | **数据与流程串联** | TODO.md §3.7 | 013 IsDeviceReady、009 Bind/008 SetUniformBuffer、013 RequestStreaming/SetStreamingPriority 等在 020 或 019 中按需在 Prepare* 或 Pass 执行中接入。 |

### 4. 019-PipelineCore 侧（与 020 协同）

| 编号 | 项 | 来源 | 说明 |
|------|----|------|------|
| T4.1 | **RenderItem / IRenderItemList 扩展** | TODO.md §3.8 | RenderItem 可扩展 transform、bounds 等；CollectItemsParallel 若 019 承担「按 Pass 并行收集」，可扩展为根据 ILogicalPipeline 的 Pass 配置从 ISceneWorld 取数并填充 out。 |

### 5. 低优先级与扩展

| 编号 | 项 | 来源 | 说明 |
|------|----|------|------|
| T5.1 | **LOD / 流式** | TODO.md §4 | 013 RequestStreaming、SetStreamingPriority 在收集或 Prepare 前按需调用。 |
| T5.2 | **RenderingConfig 与 Editor** | TODO.md §4 | RenderingConfig 已存在，与 Editor 的配置热更、保存加载对接。 |
| T5.3 | **XRSubmit / PresentTarget** | TODO.md §4, 契约 | 与 027-XR、SwapChain 的 Present 对接，明确视口与交换链归属。 |
| T5.4 | **渲染模式与 Check 宏** | TODO.md §4 | Debug/Hybrid/Resource 三种模式的 CheckWarning/CheckError 使用程度与校验点。 |
| T5.5 | **单元/集成测试** | TODO.md §4 | 最小场景（加载 Level + Model，TriggerRender 一帧，校验 Device 线程调用与 RHI Submit）；或依赖 019 的 mock 做 020 单测。 |

---

## 二、可改进项（Improvements）

### 实现与结构

| 编号 | 项 | 说明 |
|------|----|------|
| I1 | **008 SetVertexBuffer/SetIndexBuffer** | 008 当前无此接口时，020 已用现有 API 录制 Draw；若 008 后续提供，可统一走 SetVertexBuffer/SetIndexBuffer。 |
| I2 | **材质 GetGraphicsPSO** | 材质可后续暴露 GetGraphicsPSO()，便于在 ExecuteLogicalCommandBufferOnDeviceThread 内统一 PSO 绑定。 |
| I3 | **012 GetVertexStride / GetIndexFormat** | 020 中 vertexStride/indexFormat 目前写死默认值；012 可后续提供 GetVertexStride(MeshHandle)、索引格式查询。 |

### 文档与契约

| 编号 | 项 | 说明 |
|------|----|------|
| I4 | **020-public-api TODO 与实现一致** | 契约中 TODO 已标为 [x] 完成；若实现尚未完全达到（如 Prepare* 真实 Ensure、按 Pass ExecuteCallback），可在契约中拆分为子项或标注「部分实现」。 |
| I5 | **module-specs 上下游描述** | docs/module-specs/020-pipeline.md §6 写「场景遍历入口在 004」；实际收集入口为 029 CollectRenderables。可统一表述为「由 029 提供 RenderableItem，020 不直接调用 004 Traverse」。 |

### 性能与扩展

| 编号 | 项 | 说明 |
|------|----|------|
| I6 | **CollectRenderItemsParallel** | 019 若承担按 Pass 并行收集，可扩展 CollectRenderItemsParallel 与 020 的 CollectRenderablesToRenderItemList 分工（020 负责 029→RenderItem，019 负责按 Pass 分组/排序）。 |
| I7 | **帧同步与 Fence** | 当前已按 slot Wait/Signal；可文档化「完整多帧 Fence 策略」与可选三重缓冲/显式 Present 同步。 |

---

## 三、建议完成顺序

1. **T1.1 + T1.2**：019 Prepare* 真实 Ensure + BeginRenderPass/PSO/材质绑定 → 保证「有 Draw、有资源」的一帧正确执行。
2. **T1.3**：按 Pass ExecuteCallback 与 Present → 完整一帧到屏幕。
3. **T2.1、T2.2、T2.3**：视锥剔除（已接入）、LOD、合批、遮挡 → 减少无效绘制与带宽。
4. **T2.4**：Render 线程投递 → 符合管线构造与收集在 Render 线程的规划。
5. **T3.1、T3.2**：001/004 契约适配。
6. 其余（Editor/XR、测试、流式等）按需求排期。

---

## 四、文档索引（020 相关）

| 文档 | 路径 | 用途 |
|------|------|------|
| README | Engine/TenEngine-020-pipeline/README.md | 模块说明、实现状态、构建与线程模型 |
| 契约-公开 API | specs/_contracts/020-pipeline-public-api.md | 能力列表、类型、TODO（已勾选）、变更记录 |
| 契约-ABI | specs/_contracts/020-pipeline-ABI.md | ABI 表、实现说明、TriggerRender 阶段描述 |
| 模块描述 | docs/module-specs/020-pipeline.md | 功能、子模块、上下游、待办（001/004 适配） |
| 数据流 | docs/pipeline-rendering-dataflow.md | 线程角色、数据流图、二次迭代项 |
| 依赖图 | specs/_contracts/000-module-dependency-map.md | 020 依赖与下游 |
