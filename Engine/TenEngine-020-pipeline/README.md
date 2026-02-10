# TenEngine-020-Pipeline

020-Pipeline 模块：渲染管线（场景收集、剔除、DrawCall、命令缓冲生成与提交）。契约见 `specs/_contracts/020-pipeline-ABI.md`、`020-pipeline-public-api.md`、`pipeline-to-rci.md`。

## 当前实现状态

- **骨架**：CMake 目标 `te_pipeline`，依赖 001/004/005/029/019/009/010/011/012/028/013/008。
- **ABI 头文件**：`te/pipeline/RenderPipeline.h`、`FrameContext.h`、`RenderingConfig.h`。
- **实现**：
  - `RenderPipelineImpl`、`CreateRenderPipeline`；Device 线程任务队列（`DeviceThreadQueue`），所有 RHI 提交经此队列在单一 Device 线程执行。
  - **资源加载过滤**：仅当 `RenderableItem.modelResource` 非空（已加载）时转为 `RenderItem` 并加入列表（`RenderableCollector.cpp`）。
  - **Deferred FrameGraph**：`detail::CreateDeferredFrameGraph` 配置 GBuffer + Lighting 两 Pass 并 Compile；无自定义 graph 时自动创建。
  - **RenderableItem→RenderItem**：029 `CollectRenderables(SceneRef, IResourceManager*, callback)` 回调中从 `IModelResource` 取 mesh/material，填 `RenderItem`（mesh/material 以 011/012 资源指针作 handle），仅已加载项入列表。
  - **Device 阶段**：在 Device 线程依次调用 019 `PrepareRenderResources`、`ConvertToLogicalCommandBuffer`、`ExecuteLogicalCommandBufferOnDeviceThread`；后者已对接 008-RHI：CreateCommandList → Begin → End → Submit(cmd, queue) → DestroyCommandList（019 扩展 ILogicalCommandBuffer 后可在此遍历录制 Draw 等）。
  - **TriggerRender 串联**：帧 slot 环增 → 构建 pipelinecore::FrameContext → BuildLogicalPipeline → CreateRenderItemList → CollectRenderablesToRenderItemList → Post(Prepare→Convert→Submit)，列表与 pipeline 在 Device 任务结束时释放。
- **数据流文档**：`docs/pipeline-rendering-dataflow.md`（线程分工、数据流、二次迭代项）。
- **已完成（按 TODO）**：020 侧对每项 mesh 调用 `mesh::EnsureDeviceResources`；019 `ILogicalCommandBuffer` 扩展为携带 `LogicalDraw` 列表，020 映射为 `DrawIndexed` 录制；按 slot 的 Fence 创建/Wait/Signal。
- **待办**：SetViewport、BeginRenderPass、PSO/材质绑定；019 PrepareRenderMaterial 触发材质/贴图 Ensure（若 011 提供接口）；可选 Render 线程投递。

## 构建

从仓库根或 Engine 目录用统一 CMake 配置构建时，将 020 作为子项目加入即可。若单独配置本目录，需设置依赖路径，例如：

```bash
cmake .. -DTENENGINE_ROOT=<Engine 目录> \
  -DTENENGINE_019_PIPELINECORE_DIR=<Engine>/TenEngine-019-pipeline-core
```

（部分模块目录名与 CMake `mod_id` 不一致时需设置对应 `TENENGINE_*_DIR`。）

## 线程模型

- **主线程**：`RenderFrame` / `TriggerRender`；准备 `FrameContext`，投递本帧任务。
- **Render 线程**（规划）：`BuildLogicalPipeline`、`CollectRenderables` → RenderItem、`CollectRenderItemsParallel`、`MergeRenderItems`。
- **Device 线程**：`PrepareRenderResources`、`ConvertToLogicalCommandBuffer`、`SubmitLogicalCommandBuffer`（RHI 录制与提交）；由 `DeviceThreadQueue` 单线程执行。
