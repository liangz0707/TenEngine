# 019-PipelineCore 模块 ABI

- **契约**：[019-pipelinecore-public-api.md](./019-pipelinecore-public-api.md)（能力与类型描述）
- **本文件**：019-PipelineCore 对外 ABI 显式表。
- **CMake Target 名称**：**`te_pipelinecore`**（与 `te_rhi`、`te_rendercore` 命名风格一致）。下游在 `target_link_libraries` 中应使用 **`te_pipelinecore`**。依赖 `te_rhi`、`te_rendercore`。
- **命名空间**：**`te::pipelinecore`**（与 `te::rhi`、`te::rendercore` 风格一致）。实现文件统一使用此命名空间。
- **头文件路径**：**`te/pipelinecore/`**（与 `te/rhi/`、`te/rendercore/` 一致）。
- **渲染资源显式控制位置**：**创建逻辑渲染资源**（CreateRenderItem）；**创建/收集逻辑上的 CommandBuffer**（CollectCommandBuffer，即 convertToLogicalCommandBuffer）；**准备渲染资源**（PrepareRenderMaterial、PrepareRenderMesh 等，即 prepareRenderResources 或细粒度 API）；**提交到实际 GPU Command** 见 020-Pipeline/008-RHI（SubmitCommandBuffer）；**准备/创建/更新 GPU 资源**（CreateDeviceResource、UpdateDeviceResource）见 008-RHI。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 019-PipelineCore | te::pipelinecore | — | 常量 | 最大在途帧数 | te/pipelinecore/Config.h | kMaxFramesInFlight | 建议 2～4；实现可配置 |
| 019-PipelineCore | te::pipelinecore | — | struct | 管线配置 | te/pipelinecore/Config.h | PipelineConfig | 含 frameInFlightCount（2～4）；创建 Pipeline/SwapChain 时传入 |
| 019-PipelineCore | te::pipelinecore | — | 类型别名 | 帧 slot 索引 | te/pipelinecore/Config.h | FrameSlotId | uint32_t，范围 [0, frameInFlightCount) |
| 019-PipelineCore | te::pipelinecore | — | struct | FrameContext | te/pipelinecore/FrameContext.h | FrameContext | scene (ISceneWorld const*), camera, viewport (ViewportDesc), frameSlotId 等；020 构造并传入 BuildLogicalPipeline、CollectRenderItemsParallel |
| 019-PipelineCore | te::pipelinecore | — | struct | 视口描述 | te/pipelinecore/FrameContext.h | ViewportDesc | width, height 等视口参数 |
| 019-PipelineCore | te::pipelinecore | ISceneWorld | 抽象接口 | 场景世界最小接口 | te/pipelinecore/FrameGraph.h | ISceneWorld | 020/004 实现；SetScene 绑定；提供查询可见实体、场景根等 |
| 019-PipelineCore | te::pipelinecore | IFrameGraph | 抽象接口 | FrameGraph 入口 | te/pipelinecore/FrameGraph.h | IFrameGraph::AddPass | `IPassBuilder* AddPass(char const* name);` 将 Pass 加入图，返回 Builder 供配置 |
| 019-PipelineCore | te::pipelinecore | IFrameGraph | 抽象接口 | 编译图 | te/pipelinecore/FrameGraph.h | IFrameGraph::Compile | `bool Compile();` 编译为可执行图；依赖与顺序由实现解析 |
| 019-PipelineCore | te::pipelinecore | IFrameGraph | 抽象接口 | Pass 数量与按序执行 | te/pipelinecore/FrameGraph.h | IFrameGraph::GetPassCount, ExecutePass | `size_t GetPassCount() const = 0;` `void ExecutePass(size_t executionOrder, PassContext& ctx, te::rhi::ICommandList* cmd) = 0;` 020 在 Device 任务内按 GetPassCount 循环调用 ExecutePass |
| 019-PipelineCore | te::pipelinecore | IPassBuilder | 抽象接口 | Pass 配置 | te/pipelinecore/FrameGraph.h | IPassBuilder::SetScene, SetCullMode, SetObjectTypeFilter, SetRenderType, SetOutput, SetExecuteCallback | `void SetScene(ISceneWorld const* scene);` `void SetCullMode(CullMode mode);` `void SetObjectTypeFilter(...);` `void SetRenderType(RenderType type);` `void SetOutput(PassOutputDesc const& desc);` `void SetExecuteCallback(PassExecuteCallback cb);` |
| 019-PipelineCore | te::pipelinecore | — | 枚举 | 收集/剔除方式 | te/pipelinecore/FrameGraph.h | CullMode | `enum class CullMode { None, FrustumCull, OcclusionCull, FrustumAndOcclusion };` |
| 019-PipelineCore | te::pipelinecore | — | 枚举 | 渲染类型 | te/pipelinecore/FrameGraph.h | RenderType | `enum class RenderType { Opaque, Transparent, Overlay, Custom };` |
| 019-PipelineCore | te::pipelinecore | — | struct | Pass 输出描述 | te/pipelinecore/FrameGraph.h | PassOutputDesc | 渲染目标、深度、多 RT、分辨率、格式等；程序员配置自定义输出 |
| 019-PipelineCore | te::pipelinecore | PassContext | struct/抽象 | Pass 执行时上下文 | te/pipelinecore/FrameGraph.h | PassContext::GetCollectedObjects, SetCollectedObjects | `IRenderObjectList const* GetCollectedObjects() const;` `void SetCollectedObjects(IRenderObjectList const* o);` 020 按 Pass 填充 collectedObjects 后调用 ExecutePass |
| 019-PipelineCore | te::pipelinecore | — | 回调类型 | Pass 执行回调 | te/pipelinecore/FrameGraph.h | PassExecuteCallback | `void (*PassExecuteCallback)(PassContext& ctx, ICommandList* cmd);` 在 ctx 中取 collectedObjects 与输出 |
| 019-PipelineCore | te::pipelinecore | — | 接口 | 收集到的物体列表 | te/pipelinecore/FrameGraph.h | IRenderObjectList | 只读列表/迭代器，供 Pass 回调遍历绘制；由 Pipeline 在收集阶段填充 |
| 019-PipelineCore | te::pipelinecore | ILogicalPipeline | 抽象/struct | 逻辑管线描述 | te/pipelinecore/LogicalPipeline.h | ILogicalPipeline | 由 BuildLogicalPipeline 产出；含 Pass 列表、每 Pass 收集配置与输出；供线程 C 消费 |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | 构建逻辑管线（线程 B） | te/pipelinecore/LogicalPipeline.h | BuildLogicalPipeline | `ILogicalPipeline* BuildLogicalPipeline(IFrameGraph const* graph, FrameContext const& ctx);` 仅产出逻辑数据，不录 GPU 命令 |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | **创建逻辑渲染资源**（显式控制位置） | te/pipelinecore/RenderItem.h | CreateRenderItem | `RenderItem* CreateRenderItem(...);` 或由收集阶段产出；创建逻辑上的可渲染项，供后续 CollectCommandBuffer / PrepareRenderResources 使用 |
| 019-PipelineCore | te::pipelinecore | — | struct | 单条可渲染项 | te/pipelinecore/RenderItem.h | RenderItem | mesh, material, sortKey；扩展 **transform**（void*）、**bounds**（RenderItemBounds：min[3]/max[3]）；收集阶段产出或 CreateRenderItem 创建 |
| 019-PipelineCore | te::pipelinecore | — | 接口/容器 | RenderItem 列表 | te/pipelinecore/RenderItem.h | IRenderItemList | 合并后的 RenderItem 列表；线程 C 多线程收集后 merge 得到 |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | 多线程收集 RenderItem（线程 C） | te/pipelinecore/CollectPass.h | CollectRenderItemsParallel | `void CollectRenderItemsParallel(ILogicalPipeline const* pipeline, FrameContext const& ctx, IRenderItemList* out);` 多线程并行，内部或调用方 merge |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | 合并 RenderItem 列表 | te/pipelinecore/CollectPass.h | MergeRenderItems | `void MergeRenderItems(IRenderItemList const* partial_lists, size_t count, IRenderItemList* merged);` 线程 C 并行结束后合并 |
| 019-PipelineCore | te::pipelinecore | ILogicalCommandBuffer | 抽象接口 | 逻辑 CommandBuffer | te/pipelinecore/LogicalCommandBuffer.h | ILogicalCommandBuffer | CPU 侧逻辑命令序列；由 ConvertToLogicalCommandBuffer 产出 |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | 线程 D：渲染资源准备（整体） | te/pipelinecore/RenderItem.h | PrepareRenderResources | `ResultCode PrepareRenderResources(IRenderItemList const* items, IDevice* device);` **必须在线程 D 调用**；遇 RHI 失败返回 ResultCode（te::rendercore::ResultCode），由调用方决定跳过/重试/中止 |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | **准备渲染资源：材质**（显式控制位置） | te/pipelinecore/RenderItem.h | PrepareRenderMaterial | `ResultCode PrepareRenderMaterial(IMaterialHandle const* material, IDevice* device);` **必须在线程 D 调用**；遇失败返回 ResultCode |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | **准备渲染资源：网格**（显式控制位置） | te/pipelinecore/RenderItem.h | PrepareRenderMesh | `ResultCode PrepareRenderMesh(IMeshHandle const* mesh, IDevice* device);` **必须在线程 D 调用**；遇失败返回 ResultCode |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | **创建/收集逻辑上的 CommandBuffer**（显式控制位置） | te/pipelinecore/LogicalCommandBuffer.h | ConvertToLogicalCommandBuffer / CollectCommandBuffer | `ResultCode ConvertToLogicalCommandBuffer(IRenderItemList const* items, ILogicalPipeline const* pipeline, ILogicalCommandBuffer** out);` 别名 CollectCommandBuffer；**必须在线程 D 调用**；遇失败返回 ResultCode；实现按 (material, mesh, submeshIndex) 排序并合并为 instanced draw（同组 instanceCount 累加）；LogicalDraw 含 indexCount、firstIndex（来自 012 GetSubmesh） |
| 019-PipelineCore | te::pipelinecore | — | 宏 | Profiling 开关 | te/pipelinecore/Profiling.h | TE_PIPELINECORE_PROFILING | 启用 profiling 时定义；开发版可获取 Pass 耗时、Compile 耗时等 |
| 019-PipelineCore | te::pipelinecore | PassProfilingScope | struct | Pass 计时 | te/pipelinecore/Profiling.h | PassProfilingScope | RAII，Pass 开始/结束计时 |
| 019-PipelineCore | te::pipelinecore | — | 回调 | Compile 耗时 | te/pipelinecore/Profiling.h | OnCompileProfiling | Compile 耗时回调；宏或配置控制 |

**ResultCode**：使用 009-RenderCore 的 `te::rendercore::ResultCode`。

*来源：用户故事 US-004（流水线式多帧渲染）、US-rendering-003（FrameGraph AddPass）、US-rendering-004（多线程管线阶段；线程 D = 唯一 GPU/Device 线程，所有 GPU 操作与资源创建须在线程 D）。*

---

数据与接口 TODO 已迁移至本模块契约 [019-pipelinecore-public-api.md](./019-pipelinecore-public-api.md) 的 TODO 列表；本文件仅保留 ABI 表与实现说明。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| 2026-02-10 | ABI 同步：IFrameGraph 增加 GetPassCount、ExecutePass；PassContext 增加 SetCollectedObjects；RenderItem 扩展 transform、bounds；ConvertToLogicalCommandBuffer 排序与 instanced 合批 |
| 2026-02-11 | FrameGraph 扩展：PassKind、PassContentSource、PassAttachmentDesc；IFrameGraph::AddPass(name, PassKind)、GetPassCollectConfig；IPassBuilder SetPassKind/SetContentSource/AddColorAttachment/SetDepthStencilAttachment；IScenePassBuilder、ILightPassBuilder、IPostProcessPassBuilder、IEffectPassBuilder；PassContext GetRenderItemList(slot)、GetLightItemList、SetLightItemList；ILogicalPipeline::GetPassConfig(index, PassCollectConfig*)；RenderItem.h 增加 LightItem、ILightItemList、CameraItem、ICameraItemList、ReflectionProbeItem、IReflectionProbeItemList、DecalItem、IDecalItemList 及 Create/Destroy |
