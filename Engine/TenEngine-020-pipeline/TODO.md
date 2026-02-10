# 020-Pipeline 待完成内容

本文档整理当前 020-Pipeline 模块**仍需完成或可迭代**的内容，便于按优先级推进。已实现部分见 [README.md](README.md)、[docs/pipeline-rendering-dataflow.md](../../docs/pipeline-rendering-dataflow.md)。

---

## 一、高优先级（管线闭环与正确性）

### 1. 019 ILogicalCommandBuffer 扩展并映射到 RHI 录制 — 已完成

- **实现**：019 增加 `LogicalDraw`（mesh/material/submeshIndex）与 `ILogicalCommandBuffer::GetDrawCount/GetDraw`；`ConvertToLogicalCommandBuffer` 根据 `IRenderItemList` 填充 draw 列表。020 在 `ExecuteLogicalCommandBufferOnDeviceThread` 内遍历 logicalCB，从 mesh 取 SubmeshDesc 与 vertex/index buffer，录制 `cmd->DrawIndexed(count, instanceCount, offset, 0, firstInstance)`，再 End、Submit。
- **已补充**：录制前根据 FrameContext 传入的 viewport 调用 `cmd->SetViewport` 与 `cmd->SetScissor`（在 TriggerRender 的 Device 任务中捕获 viewport 宽高并传入 ExecuteLogicalCommandBufferOnDeviceThread）。
- **待扩展**：BeginRenderPass、PSO/材质绑定、SetUniformBuffer、SetVertexBuffer/SetIndexBuffer（008 当前无此接口）等尚未接入。

### 2. Mesh EnsureDeviceResources（020 侧）— 已完成

- **实现**：020 在 Device 任务中、调用 `PrepareRenderResources` 前，遍历 `IRenderItemList`，对每项将 mesh 转为 `mesh::MeshResource const*` 并取 `GetMeshHandle()`，调用 `mesh::EnsureDeviceResources(mh, rhiDevice)`。
- **已补充**：019 的 `PrepareRenderResources` 已遍历 `IRenderItemList` 并逐项调用 `PrepareRenderMaterial`/`PrepareRenderMesh`，为后续接入 011/012 Ensure 预留单点调用。
- **待做**：019 的 `PrepareRenderMaterial`/`PrepareRenderMesh` 实现仍为空；若 011 提供带 device 的 Ensure 或 SetDevice，可在 019 内补充材质/贴图 Ensure；020 侧 mesh Ensure 保留（019 未依赖 012）。

### 3. 按 slot 的 Fence 同步 — 已完成

- **实现**：020 在构造时按 `frameInFlightCount_` 创建 `slotFences_`（`CreateFence(i==0)` 首帧已 signal）；Device 任务内先 `Wait(slotFences_[slot])`，再执行 Prepare/Convert/Execute，最后 `Signal(slotFences_[slot])`；任务入队时传入本帧 `currentSlot_`。
- **参考**：019 Config 的 `kMaxFramesInFlight`、020 的 `frameInFlightCount_`。

---

## 二、中优先级（能力与契约对齐）

### 4. 剔除与批次（Culling / Batching）— 视锥剔除已接入

- **契约**：020-public-api 能力 1/2：CollectVisible、FrustumCull、OcclusionQuery（可选）、SelectLOD；BuildBatches、MaterialSlot、MeshSlot、Transform、Instancing、MergeBatch。
- **已实现（视锥剔除）**：`FrameContext` 增加可选 `frustum`（`te::scene::Frustum const*`）；`CollectRenderablesToRenderItemList` 增加可选参数 `frustum`，当非空且节点实现 `HasAABB()`/`GetAABB()` 时，使用 `te::scene::SpatialQuery::FrustumIntersectsAABB` 剔除视锥外物体；未提供 AABB 的节点不剔除。
- **待做**：LOD 选择（012 SelectLOD）、合批与实例化（按 material/mesh 分组、instanced draw）；可选 OcclusionQuery。

### 5. Pass 执行与 Present — 部分完成

- **已实现**：
  - **Present**：020 在 Device 任务内、Signal(slotFences_[slot]) 之后，若 `swapChain_` 非空则调用 `te::rhi::ISwapChain::Present()`，与当前帧提交同线程。
  - **Pass ExecuteCallback 占位**：`CreateDeferredFrameGraph` 中为 GBuffer、Lighting 两 Pass 均调用 `SetExecuteCallback(NoOpPassExecute)`；占位回调为空，实际绘制仍由 `ExecuteLogicalCommandBufferOnDeviceThread` 统一录制。
- **待做**：在 019 或 020 中按 Pass 执行顺序调用各 Pass 的 ExecuteCallback（传入 PassContext 与 ICommandList），在回调内根据 PassContext::GetCollectedObjects() 或传入的 IRenderItemList 录制该 Pass 的绘制；与 BeginRenderPass/EndRenderPass、RT 绑定配合后可实现真正的按 Pass 渲染。

### 6. Render 线程投递（可选）

- **现状**：BuildLogicalPipeline、CollectRenderablesToRenderItemList 均在调用 TriggerRender 的线程（通常主线程）执行。
- **待做**：将「BuildLogicalPipeline + CollectRenderablesToRenderItemList」投递到独立 Render 线程，完成后再将 IRenderItemList 与 ILogicalPipeline 投递到 Device 线程，以符合「管线构造与收集在 Render 线程」的规划。

---

## 三、契约 TODO 与文档未闭环项

### 7. 020 契约 TODO（020-pipeline-public-api.md）

- **资源解析**：经 013 LoadSync/GetCached 解析为 IModelResource*；当前通过 029 CollectRenderables(SceneRef, IResourceManager*, callback) 已由 029/013 解析，若需 020 侧显式 LoadSync/RequestLoadAsync 可再补。
- **EnsureDeviceResources**：提交绘制前对资源调用 EnsureDeviceResources；当前依赖 019 PrepareRenderResources 触发，待 019 实现真实 Ensure 调用后即闭环。
- **数据与流程**：013 IsDeviceReady、009 Bind/008 SetUniformBuffer、013 RequestStreaming/SetStreamingPriority 等尚未在 020 或 019 中串联，可按需在 Prepare* 或 Pass 执行中接入。

### 8. 019 契约 TODO（019-pipelinecore-public-api.md）

- **数据**：RenderItem 含 Model/Mesh/Material 句柄引用、排序 key；IRenderItemList 已存在，RenderItem 已有 mesh/material/sortKey，可扩展 transform、bounds 等。
- **接口**：CollectRenderItemsParallel 当前仅清空 out；若 019 希望承担「按 Pass 并行收集」，可扩展为根据 ILogicalPipeline 的 Pass 配置从 ISceneWorld 取数并填充 out，与 020 的 CollectRenderablesToRenderItemList 分工（例如 020 负责从 029 取 RenderableItem→RenderItem，019 负责按 Pass 分组/排序）。

---

## 四、低优先级与扩展

- **LOD / 流式**：013 RequestStreaming、SetStreamingPriority 在收集或 Prepare 前按需调用；012 SelectLOD 在收集阶段按距离/屏占比选择 LOD。
- **遮挡剔除**：OcclusionQuery 或 GPU 遮挡查询在收集阶段或单独 Pass 中接入。
- **RenderingConfig 与 Editor**：RenderingConfig 已存在，与 Editor 的配置热更、保存加载可再对接。
- **XRSubmit / PresentTarget**：与 027-XR、SwapChain 的 Present 对接，明确视口与交换链归属。
- **渲染模式**：Debug/Hybrid/Resource 三种模式的 CheckWarning/CheckError 使用程度与校验点。
- **单元/集成测试**：最小场景（加载 Level + Model，TriggerRender 一帧，校验 Device 线程调用与 RHI Submit）；或依赖 019 的 mock 做 020 单测。

---

## 五、建议完成顺序

1. **019 Prepare* 真实 Ensure** + **019 ILogicalCommandBuffer 扩展与 020 映射录制**：保证「有 Draw、有资源」的一帧能正确执行。
2. **按 slot 的 Fence 同步**：保证多帧在途时无资源冲突。
3. **视锥剔除与 LOD**：减少无效绘制与带宽。
4. **Pass ExecuteCallback 与 Present**：完整一帧到屏幕。
5. 其余（Render 线程、合批、遮挡、流式、Editor/XR）按需求排期。
