# 渲染管线数据流与线程分工

本文档描述 020-Pipeline 从资源加载到 RHI 提交的**端到端数据流**与**线程分工**，便于二次迭代与扩展。契约见 `specs/_contracts/020-pipeline-ABI.md`、`pipeline-to-rci.md`。

---

## 1. 线程角色

| 线程 | 职责 | 关键 API / 位置 |
|------|------|------------------|
| **主线程** | 游戏逻辑、每帧驱动、接收加载回调、配置更新 | `IRenderPipeline::RenderFrame(ctx)` / `TriggerRender(ctx)`；准备 `FrameContext` |
| **Render 线程**（可选） | 管线构造、渲染物体收集 | `BuildLogicalPipeline`、`CollectRenderablesToRenderItemList`、`CollectRenderItemsParallel`；当前可与主线程合并 |
| **Device 线程（D）** | 所有 RHI 相关：资源准备、命令录制与提交 | `PrepareRenderResources`、`ConvertToLogicalCommandBuffer`、`ExecuteLogicalCommandBufferOnDeviceThread`（RHI Begin/End/Submit） |
| **IO 线程** | 资源加载 | 013 `RequestLoadAsync` 在后台执行 Load；**仅当加载完成回调成功**后该资源才可被收集 |

---

## 2. 数据流概览

```
主线程准备 FrameContext
       │
       ▼
TriggerRender(ctx)
       │
       ├─ 帧 slot 推进 (currentSlot_)
       ├─ 构建 pipelinecore::FrameContext、Deferred FrameGraph（若无）
       ├─ BuildLogicalPipeline(graph, coreCtx)  → ILogicalPipeline*
       ├─ CreateRenderItemList() + CollectRenderablesToRenderItemList(SceneRef, IResourceManager*, list)
       │     └─ 029 CollectRenderables；仅 modelResource 非空项转为 RenderItem 入列表
       │
       └─ Post(Device 任务)
              │
              ▼ Device 线程
              ├─ PrepareRenderResources(itemList, device)
              ├─ ConvertToLogicalCommandBuffer(itemList, pipeline, &logicalCB)
              ├─ ExecuteLogicalCommandBufferOnDeviceThread(logicalCB)
              │     └─ IDevice::CreateCommandList → Begin → [未来：遍历 logicalCB 录制 Draw 等] → End → Submit(cmd, queue) → DestroyCommandList
              └─ Destroy logicalCB / itemList / pipeline
```

---

## 3. 资源加载与「可被收集」的衔接

- **加载**：013 `RequestLoadAsync` / `LoadSync`；029 Level 加载后通过 013 解析 Model；带 `IResourceManager*` 的 `CollectRenderables` 会将 `modelResourceId` 解析为 `IModelResource*` 并写入 `RenderableItem.modelResource`。
- **约定**：仅当 **modelResource 非空**（已加载/已解析）时，该项才在 `CollectRenderablesToRenderItemList` 的回调中被转为 `RenderItem` 并加入列表；未加载项被跳过。
- **DResource**：Load 阶段不创建 GPU 资源；在 **Device 线程** 的 `PrepareRenderResources` 中通过 019 触发 011/012/028 的 `EnsureDeviceResources`。

---

## 4. 关键边界

- **RHI 提交**：所有 `ICommandList::Begin/End`、`Submit(cmd, queue)`、`CreateCommandList`/`DestroyCommandList` 仅在 **Device 线程** 执行（通过 `DeviceThreadQueue` 串行执行）。
- **019 Prepare* / ConvertToLogicalCommandBuffer**：契约规定必须在线程 D 调用；020 仅在投递到 Device 队列的任务内调用。
- **帧 slot**：`currentSlot_` 每帧环增，与 `frameInFlightCount_` 一致；完整的多帧 Fence 等待可由调用方或后续在 Device 任务前插入。

---

## 5. 第二次迭代可做项

- **019 ILogicalCommandBuffer**：扩展为携带 Draw/Viewport/RenderPass 等逻辑命令，在 `ExecuteLogicalCommandBufferOnDeviceThread` 内遍历并映射到 RHI `ICommandList` 调用。
- **帧同步**：按 slot 使用 `IFence` 等待上一帧完成后再提交本帧，避免 GPU 写读冲突。
- **Render 线程**：将「BuildLogicalPipeline + CollectRenderablesToRenderItemList」投递到独立 Render 线程，再将其结果投递到 Device 线程。
- **LOD / 流式 / 遮挡剔除**：在收集阶段或 Pass 配置中接入 013 RequestStreaming、遮挡查询等。
