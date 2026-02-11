# 渲染管线完整流程（以代码为准）

本文档按**实际代码**梳理从资源数据到 RHI 命令提交、再到最终绘制的整条渲染管线，包括**线程**、**数据状态**与**调用顺序**。与 [pipeline-rendering-dataflow.md](pipeline-rendering-dataflow.md) 互补，偏实现细节。

---

## 1. 总览：三线程 + 数据阶段

| 线程 | 职责 | 数据输入 | 数据输出 |
|------|------|----------|----------|
| **主线程** | 每帧驱动、准备 FrameContext、投递任务 | 应用/游戏逻辑 | `TriggerRender(ctx)` → Post(Render 任务) |
| **Render 线程** | 管线构造、可渲染物收集 | FrameContext、IFrameGraph、SceneRef、IResourceManager* | IRenderItemList、ILogicalPipeline → Post(Device 任务) |
| **Device 线程（D）** | 所有 RHI：Prepare、Convert、录制、Submit、Present | IRenderItemList、ILogicalPipeline、slot、viewport | GPU 命令提交、Present |
| **IO 线程**（013） | 资源文件加载 | RequestLoadAsync / 流式请求 | 加载完成后入缓存；**仅 GetCached 有值后才可能被收集** |

**数据状态主线**：  
`FrameContext` → `IRenderItemList`（逻辑渲染项）→ **Prepare** → `readyList`（IsDeviceReady 过滤）→ `ILogicalCommandBuffer`（逻辑 Draw 序列）→ **RHI ICommandList**（Begin → 按 Pass → 按 Draw 录制 → End）→ **Submit(queue)** → **Present**。

---

## 2. 数据来源：谁提供「可渲染物」

- **场景结构**：004-Scene（SceneRef、SceneManager::Traverse）、002-Object（Entity、Component）。
- **可渲染实体**：029-World 的 `ModelComponent` 持有 `modelResourceId`（013 ResourceId）。
- **模型解析**：029 `CollectRenderables(SceneRef, IResourceManager*, callback)` 内部：
  - 用 `SceneManager::Traverse(sceneRef, ...)` 遍历节点；
  - 对每个带 `ModelComponent` 的 Entity，用 `resourceManager->GetCached(comp->modelResourceId)` 得到 `IResource*`，再 `dynamic_cast<IModelResource*>`；
  - 仅当 **modelResource 非空**（已加载并进缓存）时，才在 callback 里把该项视为「可渲染」。
- **结论**：**资源必须先被加载并进入 013 缓存**（主线程/IO 回调等），CollectRenderables 时 GetCached 才有值；否则该项被 020 收集逻辑直接跳过（`if (!item.modelResource) return;`）。

---

## 3. 主线程：TriggerRender 入口

**文件**：`Engine/TenEngine-020-pipeline/src/RenderPipeline.cpp`

1. **帧 slot**：`currentSlot_ = (currentSlot_ + 1) % frameInFlightCount_`（主线程只做环增）。
2. **FrameGraph**：若尚未设置，则 `detail::CreateDeferredFrameGraph(ctx.viewport.width, ctx.viewport.height)` 创建 GBuffer + Lighting 两 Pass，并 `Compile()`。
3. **拷贝与投递**：拷贝 `FrameContext`、`device_`、`currentSlot_`、viewport、`resourceManager_`、`graph`，然后：
   - `renderQueue_->Post([...]() { ... });`  
   即把「阶段 A」投递到 **Render 线程**。

主线程不持有 RenderItemList / LogicalPipeline；只准备只读的 FrameContext 并投递任务。

---

## 4. Render 线程：管线构造与收集

**同一 lambda 内**（在 Render 线程执行）：

1. **适配 019 上下文**：`detail::ToPipelinecoreFrameContext(ctxCopy, &sceneAdapter, coreCtx)`，得到 `pipelinecore::FrameContext`（含 scene、camera、viewport、frameSlotId）。
2. **构建逻辑管线**：`pipelinecore::BuildLogicalPipeline(graphCapture, coreCtx)` → `ILogicalPipeline* pipeline`（按 FrameGraph 的 Pass 配置生成，019 实现）。
3. **创建渲染项列表**：`pipelinecore::CreateRenderItemList()` → `IRenderItemList* itemList`。
4. **收集可渲染物**：
   - 解析 `SceneRef`：若 `ctxCopy.sceneRoot` 非空则用其指向的 SceneRef，否则 `WorldManager::GetCurrentLevelScene()`。
   - `CollectRenderablesToRenderItemList(sceneRef, IResourceManager*, itemList, frustum, camera)`（020 实现）：
     - 内部调用 **029** `WorldManager::CollectRenderables(sceneRef, resourceManager, callback)`；
     - 对每个回调的 `(ISceneNode*, RenderableItem)`：视锥剔除（若有）、**若 `!item.modelResource` 则 return**；
     - 从 `IModelResource` 取 mesh/material，LOD 用 012 `SelectLOD`（若传了 cameraPositionWorld）；
     - 构造 `pipelinecore::RenderItem`（mesh/material 为 011/012 资源指针，sortKey 等），`out->Push(ri)`。
5. **流式请求**（可选）：对 itemList 中每项的 mesh 调用 `resourceManager->RequestStreaming(meshRes->GetResourceId(), 0)`，便于后续 LOD/流式。
6. **投递到 Device 线程**：`deviceQueue_->Post([dev, itemList, pipeline, graphCapture, slot, vpW, vpH, this]() { ... });`  
   Render 线程不执行任何 RHI 调用；只产出 **IRenderItemList + ILogicalPipeline** 交给 Device。

---

## 5. Device 线程：Prepare → Convert → 录制 → Submit → Present

**同一 lambda 内**（在 Device 线程执行），**所有 RHI 与 019 Prepare/Convert 均在此完成**。

### 5.1 同步

- 若 `slotFences_[slot]` 存在，则 `te::rhi::Wait(slotFences_[slot])`，等待上一帧该 slot 的 GPU 工作完成。
- **Mesh/Material 的 Ensure** 仅由 019 `PrepareRenderResources` 内 `PrepareRenderMaterial` / `PrepareRenderMesh` 统一完成，020 不再对 mesh 单独做一次 Ensure（避免重复）。

### 5.2 PrepareRenderResources（019）

- `pipelinecore::PrepareRenderResources(itemList, rhiDevice)`：
  - 遍历 itemList，对每项调用：
    - `PrepareRenderMaterial(r->material, device)` → 011 `MaterialResource::SetDevice(device)` + `EnsureDeviceResources()`；
    - `PrepareRenderMesh(r->mesh, device)` → 012 `EnsureDeviceResources(mh, device)`。
  - 即在本线程完成**材质/网格的 GPU 资源创建**（008/009/028 等）；020 不再在此前单独对 mesh 做 Ensure。

### 5.3 IsDeviceReady 过滤

- `pipelinecore::CreateRenderItemList()` 得到 `readyList`，遍历 itemList，仅当 `meshRes->IsDeviceReady() && matRes->IsDeviceReady()` 时 `readyList->Push(*r)`。
- 用于录制时用 `listToConvert = readyList->Size() > 0 ? readyList : itemList`，避免对未就绪资源录制 Draw。

### 5.4 ConvertToLogicalCommandBuffer（019）

- `pipelinecore::ConvertToLogicalCommandBuffer(listToConvert, pipeline, &logicalCB)`：
  - 遍历 listToConvert，每项生成 `LogicalDraw`（mesh、material、submeshIndex、instanceCount 等）；
  - 按 material/mesh/submeshIndex 排序并做**合批**（相同 material+mesh+submesh 合并 instanceCount）；
  - 输出 `ILogicalCommandBuffer* logicalCB`（仅 CPU 侧逻辑 Draw 列表，无 RHI 句柄）。

### 5.5 创建 RHI CommandList 与 Viewport

- `rhiDevice->CreateCommandList()` → `te::rhi::ICommandList* cmd`；
- `te::rhi::Begin(cmd)`；
- 设置 Viewport / Scissor（若 vpW、vpH > 0）。

### 5.6 按 Pass 执行（FrameGraph）与 Draw 在 Pass 内

- 若 `graphCapture->GetPassCount() > 0`：
  - 用 `IRenderObjectList` 适配器包装当前 `itemList`，`passCtx.SetCollectedObjects(&adapter)`；
  - 对每个 Pass 下标 `i`：
    - `cmd->BeginRenderPass(rpDesc)`（当前 rpDesc 为空占位）；
    - `cmd->BeginOcclusionQuery(0)`（占位）；
    - **当 i == 0（主几何 Pass，如 GBuffer）时**：`ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB)`，在该 Pass 的 RenderPass 内录制所有 Draw；
    - `graphCapture->ExecutePass(i, passCtx, cmd)` → 调用该 Pass 的 ExecuteCallback；
    - `cmd->EndOcclusionQuery(0)`；`cmd->EndRenderPass()`。
- **约定**：所有 Draw 必须在某个 `BeginRenderPass` / `EndRenderPass` 之间录制，不能落在 Pass 外；当前实现将 logicalCB 的录制放在第一个 Pass 内。
- 无 FrameGraph 时退化为单 Pass：`BeginRenderPass` → `ExecuteLogicalCommandBufferOnDeviceThread` → `EndRenderPass`。

### 5.7 ExecuteLogicalCommandBufferOnDeviceThread（020）

- `ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB)`：
  - 遍历 `logicalCB->GetDrawCount()`，对每条 `LogicalDraw`：
    - **材质**：若 `d.material` 非空，取 011 `MaterialResource`，`GetUniformBuffer()->Bind(cmd, 0)`（009），`GetGraphicsPSO()` 后 `cmd->SetGraphicsPSO(pso)`（008）；
    - **Mesh**：取 012 MeshHandle、SubmeshDesc，`GetVertexBufferHandle`/`GetIndexBufferHandle`，`GetVertexStride`/`GetIndexFormat`，然后：
      - `cmd->SetVertexBuffer(0, vb, 0, vertexStride)`，`cmd->SetIndexBuffer(ib, 0, indexFormat)`；
      - `cmd->DrawIndexed(sub->count, d.instanceCount, sub->offset, 0, d.firstInstance)`。
  - 即：**所有 RHI 的 SetGraphicsPSO / SetVertexBuffer / SetIndexBuffer / DrawIndexed 均在 Device 线程、且在本 cmd 上录制**。

### 5.8 提交与 Present

- `te::rhi::End(cmd)`；
- `te::rhi::Submit(cmd, queue)`（queue = `rhiDevice->GetQueue(Graphics, 0)`）；
- `rhiDevice->DestroyCommandList(cmd)`；
- `te::rhi::Signal(slotFences_[slot])`；
- 若存在 `swapChain_`，则 `sc->Present()`。

### 5.9 释放本帧资源

- `DestroyRenderItemList(itemList)`，`DestroyLogicalPipeline(pipeline)`；若创建过 readyList 则也已 Destroy。  
- logicalCB 在 Submit 前已 `DestroyLogicalCommandBuffer(logicalCB)`。

---

## 6. 数据状态小结（按阶段）

| 阶段 | 线程 | 数据状态 |
|------|------|----------|
| 入口 | 主线程 | `FrameContext`（sceneRoot, camera, frustum, viewport, deltaTime）；无 RenderItem。 |
| 收集后 | Render 线程 | `IRenderItemList`（仅 model 已加载且通过视锥/LOD 的 RenderItem）；`ILogicalPipeline`（Pass 配置）。 |
| Prepare 后 | Device 线程 | 同一 itemList；011/012/028 的 GPU 资源已 Ensure。 |
| 过滤后 | Device 线程 | `readyList`（IsDeviceReady 为 true 的子集），或退化为 itemList。 |
| Convert 后 | Device 线程 | `ILogicalCommandBuffer`（LogicalDraw 序列，已排序与合批）。 |
| 录制后 | Device 线程 | RHI `ICommandList` 已包含 SetViewport、BeginRenderPass/EndRenderPass、SetGraphicsPSO、SetVertexBuffer/SetIndexBuffer、DrawIndexed。 |
| Submit 后 | Device 线程 | 命令已入 GPU 队列；Fence Signal；Present 到屏幕。 |

---

## 7. 关键约束（与代码一致）

- **RHI Command 仅在 Device 线程**：CreateCommandList、Begin/End、SetViewport、BeginRenderPass/EndRenderPass、SetGraphicsPSO、SetVertexBuffer/SetIndexBuffer、DrawIndexed、Submit、Present 均在 `deviceQueue_` 的 lambda 内执行。
- **019 Prepare* / ConvertToLogicalCommandBuffer**：仅在 Device 线程调用（同上 lambda）。
- **渲染物可见性**：只有 029 CollectRenderables 回调里 `modelResource != nullptr` 的项才会被 020 转为 RenderItem；即**依赖 013 缓存（加载完成）**。
- **帧同步**：每帧 Device 任务先 Wait(slotFences_[slot])，最后 Signal(slotFences_[slot])；Present 在 Signal 之后、同一 Device 任务内。

---

## 8. 与 dataflow 文档的对应

- [pipeline-rendering-dataflow.md](pipeline-rendering-dataflow.md) 中的「线程角色」「数据流概览」「资源加载与可被收集的衔接」「关键边界」与上述实现一致；本文档按**具体函数与调用顺序**展开，便于调试与二次迭代。
