# 019-PipelineCore 模块 ABI

- **契约**：[019-pipelinecore-public-api.md](./019-pipelinecore-public-api.md)（能力与类型描述）
- **本文件**：019-PipelineCore 对外 ABI 显式表。
- **渲染资源显式控制位置**：**创建逻辑渲染资源**（CreateRenderItem）；**创建/收集逻辑上的 CommandBuffer**（CollectCommandBuffer，即 convertToLogicalCommandBuffer）；**准备渲染资源**（PrepareRenderMaterial、PrepareRenderMesh 等，即 prepareRenderResources 或细粒度 API）；**提交到实际 GPU Command** 见 020-Pipeline/008-RHI（SubmitCommandBuffer）；**准备/创建/更新 GPU 资源**（CreateDeviceResource、UpdateDeviceResource）见 008-RHI。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 019-PipelineCore | TenEngine::pipelinecore | — | 常量 | 最大在途帧数 | TenEngine/pipelinecore/Config.h | kMaxFramesInFlight | 建议 2～4；实现可配置 |
| 019-PipelineCore | TenEngine::pipelinecore | — | struct | 管线配置 | TenEngine/pipelinecore/Config.h | PipelineConfig | 含 frameInFlightCount（2～4）；创建 Pipeline/SwapChain 时传入 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 类型别名 | 帧 slot 索引 | TenEngine/pipelinecore/Config.h | FrameSlotId | uint32_t，范围 [0, frameInFlightCount) |
| 019-PipelineCore | TenEngine::pipelinecore | IFrameGraph | 抽象接口 | FrameGraph 入口 | TenEngine/pipelinecore/FrameGraph.h | IFrameGraph::AddPass | `IPassBuilder* AddPass(char const* name);` 将 Pass 加入图，返回 Builder 供配置 |
| 019-PipelineCore | TenEngine::pipelinecore | IFrameGraph | 抽象接口 | 编译图 | TenEngine/pipelinecore/FrameGraph.h | IFrameGraph::Compile | `bool Compile();` 编译为可执行图；依赖与顺序由实现解析 |
| 019-PipelineCore | TenEngine::pipelinecore | IPassBuilder | 抽象接口 | Pass 配置 | TenEngine/pipelinecore/FrameGraph.h | IPassBuilder::SetScene, SetCullMode, SetObjectTypeFilter, SetRenderType, SetOutput, SetExecuteCallback | `void SetScene(ISceneWorld const* scene);` `void SetCullMode(CullMode mode);` `void SetObjectTypeFilter(...);` `void SetRenderType(RenderType type);` `void SetOutput(PassOutputDesc const& desc);` `void SetExecuteCallback(PassExecuteCallback cb);` |
| 019-PipelineCore | TenEngine::pipelinecore | — | 枚举 | 收集/剔除方式 | TenEngine/pipelinecore/FrameGraph.h | CullMode | `enum class CullMode { None, FrustumCull, OcclusionCull, FrustumAndOcclusion };` |
| 019-PipelineCore | TenEngine::pipelinecore | — | 枚举 | 渲染类型 | TenEngine/pipelinecore/FrameGraph.h | RenderType | `enum class RenderType { Opaque, Transparent, Overlay, Custom };` |
| 019-PipelineCore | TenEngine::pipelinecore | — | struct | Pass 输出描述 | TenEngine/pipelinecore/FrameGraph.h | PassOutputDesc | 渲染目标、深度、多 RT、分辨率、格式等；程序员配置自定义输出 |
| 019-PipelineCore | TenEngine::pipelinecore | PassContext | struct/抽象 | Pass 执行时上下文 | TenEngine/pipelinecore/FrameGraph.h | PassContext::GetCollectedObjects | `IRenderObjectList const* GetCollectedObjects() const;` 执行回调内拿到本 Pass 收集到的物体列表 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 回调类型 | Pass 执行回调 | TenEngine/pipelinecore/FrameGraph.h | PassExecuteCallback | `void (*PassExecuteCallback)(PassContext& ctx, ICommandList* cmd);` 在 ctx 中取 collectedObjects 与输出 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 接口 | 收集到的物体列表 | TenEngine/pipelinecore/FrameGraph.h | IRenderObjectList | 只读列表/迭代器，供 Pass 回调遍历绘制；由 Pipeline 在收集阶段填充 |
| 019-PipelineCore | TenEngine::pipelinecore | ILogicalPipeline | 抽象/struct | 逻辑管线描述 | TenEngine/pipelinecore/LogicalPipeline.h | ILogicalPipeline | 由 BuildLogicalPipeline 产出；含 Pass 列表、每 Pass 收集配置与输出；供线程 C 消费 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | 构建逻辑管线（线程 B） | TenEngine/pipelinecore/LogicalPipeline.h | BuildLogicalPipeline | `ILogicalPipeline* BuildLogicalPipeline(IFrameGraph const* graph, FrameContext const& ctx);` 仅产出逻辑数据，不录 GPU 命令 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | **创建逻辑渲染资源**（显式控制位置） | TenEngine/pipelinecore/RenderItem.h | CreateRenderItem | `RenderItem* CreateRenderItem(...);` 或由收集阶段产出；创建逻辑上的可渲染项，供后续 CollectCommandBuffer / PrepareRenderResources 使用 |
| 019-PipelineCore | TenEngine::pipelinecore | — | struct | 单条可渲染项 | TenEngine/pipelinecore/RenderItem.h | RenderItem | 场景模型、UI、材质、排序 key 等；收集阶段产出或 CreateRenderItem 创建 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 接口/容器 | RenderItem 列表 | TenEngine/pipelinecore/RenderItem.h | IRenderItemList | 合并后的 RenderItem 列表；线程 C 多线程收集后 merge 得到 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | 多线程收集 RenderItem（线程 C） | TenEngine/pipelinecore/CollectPass.h | CollectRenderItemsParallel | `void CollectRenderItemsParallel(ILogicalPipeline const* pipeline, FrameContext const& ctx, IRenderItemList* out);` 多线程并行，内部或调用方 merge |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | 合并 RenderItem 列表 | TenEngine/pipelinecore/CollectPass.h | MergeRenderItems | `void MergeRenderItems(IRenderItemList const* partial_lists, size_t count, IRenderItemList* merged);` 线程 C 并行结束后合并 |
| 019-PipelineCore | TenEngine::pipelinecore | ILogicalCommandBuffer | 抽象接口 | 逻辑 CommandBuffer | TenEngine/pipelinecore/LogicalCommandBuffer.h | ILogicalCommandBuffer | CPU 侧逻辑命令序列；由 ConvertToLogicalCommandBuffer 产出 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | 线程 D：渲染资源准备（整体） | TenEngine/pipelinecore/RenderItem.h | PrepareRenderResources | `void PrepareRenderResources(IRenderItemList const* items, IDevice* device);` **必须在线程 D 调用**；为 RenderItem 创建/准备 GPU 渲染资源（纹理、缓冲、PSO 等） |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | **准备渲染资源：材质**（显式控制位置） | TenEngine/pipelinecore/RenderItem.h | PrepareRenderMaterial | `void PrepareRenderMaterial(IMaterialHandle const* material, IDevice* device);` **必须在线程 D 调用**；准备材质对应的 GPU 资源（PSO、纹理绑定等） |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | **准备渲染资源：网格**（显式控制位置） | TenEngine/pipelinecore/RenderItem.h | PrepareRenderMesh | `void PrepareRenderMesh(IMeshHandle const* mesh, IDevice* device);` **必须在线程 D 调用**；准备网格对应的 GPU 资源（顶点/索引缓冲、布局等） |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | **创建/收集逻辑上的 CommandBuffer**（显式控制位置） | TenEngine/pipelinecore/LogicalCommandBuffer.h | ConvertToLogicalCommandBuffer / CollectCommandBuffer | `ILogicalCommandBuffer* ConvertToLogicalCommandBuffer(IRenderItemList const* items, ILogicalPipeline const* pipeline);` 别名 CollectCommandBuffer；**必须在线程 D 调用**；由 RenderItem 列表产出逻辑 CB |

*来源：用户故事 US-004（流水线式多帧渲染）、US-rendering-003（FrameGraph AddPass）、US-rendering-004（多线程管线阶段；线程 D = 唯一 GPU/Device 线程，所有 GPU 操作与资源创建须在线程 D）。*
