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
| 019-PipelineCore | te::pipelinecore | IPassBuilder | 抽象接口 | Pass 配置 | te/pipelinecore/FrameGraph.h | IPassBuilder::SetScene, SetCullMode, SetObjectTypeFilter, SetRenderType, SetOutput, SetExecuteCallback | `void SetScene(ISceneWorld const* scene);` `void SetCullMode(CullMode mode);` `void SetObjectTypeFilter(...);` `void SetRenderType(RenderType type);` `void SetOutput(PassOutputDesc const& desc);` `void SetExecuteCallback(PassExecuteCallback cb);` |
| 019-PipelineCore | te::pipelinecore | — | 枚举 | 收集/剔除方式 | te/pipelinecore/FrameGraph.h | CullMode | `enum class CullMode { None, FrustumCull, OcclusionCull, FrustumAndOcclusion };` |
| 019-PipelineCore | te::pipelinecore | — | 枚举 | 渲染类型 | te/pipelinecore/FrameGraph.h | RenderType | `enum class RenderType { Opaque, Transparent, Overlay, Custom };` |
| 019-PipelineCore | te::pipelinecore | — | struct | Pass 输出描述 | te/pipelinecore/FrameGraph.h | PassOutputDesc | 渲染目标、深度、多 RT、分辨率、格式等；程序员配置自定义输出 |
| 019-PipelineCore | te::pipelinecore | PassContext | struct/抽象 | Pass 执行时上下文 | te/pipelinecore/FrameGraph.h | PassContext::GetCollectedObjects | `IRenderObjectList const* GetCollectedObjects() const;` 执行回调内拿到本 Pass 收集到的物体列表 |
| 019-PipelineCore | te::pipelinecore | — | 回调类型 | Pass 执行回调 | te/pipelinecore/FrameGraph.h | PassExecuteCallback | `void (*PassExecuteCallback)(PassContext& ctx, ICommandList* cmd);` 在 ctx 中取 collectedObjects 与输出 |
| 019-PipelineCore | te::pipelinecore | — | 接口 | 收集到的物体列表 | te/pipelinecore/FrameGraph.h | IRenderObjectList | 只读列表/迭代器，供 Pass 回调遍历绘制；由 Pipeline 在收集阶段填充 |
| 019-PipelineCore | te::pipelinecore | ILogicalPipeline | 抽象/struct | 逻辑管线描述 | te/pipelinecore/LogicalPipeline.h | ILogicalPipeline | 由 BuildLogicalPipeline 产出；含 Pass 列表、每 Pass 收集配置与输出；供线程 C 消费 |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | 构建逻辑管线（线程 B） | te/pipelinecore/LogicalPipeline.h | BuildLogicalPipeline | `ILogicalPipeline* BuildLogicalPipeline(IFrameGraph const* graph, FrameContext const& ctx);` 仅产出逻辑数据，不录 GPU 命令 |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | **创建逻辑渲染资源**（显式控制位置） | te/pipelinecore/RenderItem.h | CreateRenderItem | `RenderItem* CreateRenderItem(...);` 或由收集阶段产出；创建逻辑上的可渲染项，供后续 CollectCommandBuffer / PrepareRenderResources 使用 |
| 019-PipelineCore | te::pipelinecore | — | struct | 单条可渲染项 | te/pipelinecore/RenderItem.h | RenderItem | 场景模型、UI、材质、排序 key 等；收集阶段产出或 CreateRenderItem 创建 |
| 019-PipelineCore | te::pipelinecore | — | 接口/容器 | RenderItem 列表 | te/pipelinecore/RenderItem.h | IRenderItemList | 合并后的 RenderItem 列表；线程 C 多线程收集后 merge 得到 |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | 多线程收集 RenderItem（线程 C） | te/pipelinecore/CollectPass.h | CollectRenderItemsParallel | `void CollectRenderItemsParallel(ILogicalPipeline const* pipeline, FrameContext const& ctx, IRenderItemList* out);` 多线程并行，内部或调用方 merge |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | 合并 RenderItem 列表 | te/pipelinecore/CollectPass.h | MergeRenderItems | `void MergeRenderItems(IRenderItemList const* partial_lists, size_t count, IRenderItemList* merged);` 线程 C 并行结束后合并 |
| 019-PipelineCore | te::pipelinecore | ILogicalCommandBuffer | 抽象接口 | 逻辑 CommandBuffer | te/pipelinecore/LogicalCommandBuffer.h | ILogicalCommandBuffer | CPU 侧逻辑命令序列；由 ConvertToLogicalCommandBuffer 产出 |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | 线程 D：渲染资源准备（整体） | te/pipelinecore/RenderItem.h | PrepareRenderResources | `ResultCode PrepareRenderResources(IRenderItemList const* items, IDevice* device);` **必须在线程 D 调用**；遇 RHI 失败返回 ResultCode（te::rendercore::ResultCode），由调用方决定跳过/重试/中止 |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | **准备渲染资源：材质**（显式控制位置） | te/pipelinecore/RenderItem.h | PrepareRenderMaterial | `ResultCode PrepareRenderMaterial(IMaterialHandle const* material, IDevice* device);` **必须在线程 D 调用**；遇失败返回 ResultCode |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | **准备渲染资源：网格**（显式控制位置） | te/pipelinecore/RenderItem.h | PrepareRenderMesh | `ResultCode PrepareRenderMesh(IMeshHandle const* mesh, IDevice* device);` **必须在线程 D 调用**；遇失败返回 ResultCode |
| 019-PipelineCore | te::pipelinecore | — | 自由函数/接口 | **创建/收集逻辑上的 CommandBuffer**（显式控制位置） | te/pipelinecore/LogicalCommandBuffer.h | ConvertToLogicalCommandBuffer / CollectCommandBuffer | `ResultCode ConvertToLogicalCommandBuffer(IRenderItemList const* items, ILogicalPipeline const* pipeline, ILogicalCommandBuffer** out);` 别名 CollectCommandBuffer；**必须在线程 D 调用**；遇失败返回 ResultCode |
| 019-PipelineCore | te::pipelinecore | — | 宏 | Profiling 开关 | te/pipelinecore/Profiling.h | TE_PIPELINECORE_PROFILING | 启用 profiling 时定义；开发版可获取 Pass 耗时、Compile 耗时等 |
| 019-PipelineCore | te::pipelinecore | PassProfilingScope | struct | Pass 计时 | te/pipelinecore/Profiling.h | PassProfilingScope | RAII，Pass 开始/结束计时 |
| 019-PipelineCore | te::pipelinecore | — | 回调 | Compile 耗时 | te/pipelinecore/Profiling.h | OnCompileProfiling | Compile 耗时回调；宏或配置控制 |

**ResultCode**：使用 009-RenderCore 的 `te::rendercore::ResultCode`。

*来源：用户故事 US-004（流水线式多帧渲染）、US-rendering-003（FrameGraph AddPass）、US-rendering-004（多线程管线阶段；线程 D = 唯一 GPU/Device 线程，所有 GPU 操作与资源创建须在线程 D）。*

---

## 数据相关 TODO

（本模块上游：008-RHI、009-RenderCore。）

### 数据

- [ ] **RenderItem**：含 Model/Mesh/Material 句柄引用、排序 key 等
- [ ] **IRenderItemList**：RenderItem 列表

### 需提供的对外接口

| 接口 | 说明 |
|------|------|
| [ ] `CollectRenderItemsParallel(pipeline, ctx, out)` | 从 FrameContext.scene 收集可渲染项，产出 IRenderItemList |
| [ ] `PrepareRenderMaterial(handle, device) → ResultCode` | 在线程 D 调用；确保 Material 句柄具备 DResource |
| [ ] `PrepareRenderMesh(handle, device) → ResultCode` | 在线程 D 调用；确保 Mesh 句柄具备 DResource |
| [ ] `ConvertToLogicalCommandBuffer(items, pipeline, out)` | 产出 ILogicalCommandBuffer；Draw 录制时绑定 Uniform 等 |

### 需调用上游

| 场景 | 调用 008 / 009 接口 |
|------|---------------------|
| CollectRenderItemsParallel | 使用 FrameContext.scene（ISceneWorld）遍历 |
| PrepareRenderMaterial/Mesh | 若句柄提供 IsDeviceReady 且为 false → 返回 ResultCode（未就绪）；否则触发句柄的 EnsureDeviceResources（011/012） |
| ConvertToLogicalCommandBuffer | 009.`IUniformBuffer::Bind(cmd, slot)`；008.`SetUniformBuffer` 等 |

### 调用流程

1. 020 调用 CollectRenderItemsParallel → 019 从 ctx.scene 遍历 → 产出 RenderItem 列表（含 IModelResource* 等）
2. 020 在线程 D 调用 PrepareRenderMaterial/Mesh → 019 确保句柄就绪；若 IsDeviceReady 为 false 返回未就绪
3. 020 调用 ConvertToLogicalCommandBuffer → 019 录制 Draw；绑定材质 Uniform（009.Bind）
