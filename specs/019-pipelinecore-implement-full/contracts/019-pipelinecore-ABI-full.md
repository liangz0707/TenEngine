# 019-PipelineCore 模块 ABI（全量，供实现使用）

> **说明**：本文件为 plan 阶段生成的全量 ABI 内容，供 tasks 与 implement 使用。契约写回仅更新 `specs/_contracts/019-pipelinecore-ABI.md` 中的新增/修改部分，见 plan.md「契约更新」小节。
>
> **实现必须基于本全量内容**，不得仅实现变化部分。

- **契约**：019-pipelinecore-public-api.md
- **依赖**：008-RHI、009-RenderCore
- **命名空间**：**te::pipelinecore**（与 te::rhi、te::rendercore 风格一致）
- **头文件路径**：**te/pipelinecore/**（与 te/rhi/、te/rendercore/ 一致）
- **CMake Target**：**te_pipelinecore**

## 1. Config（te/pipelinecore/Config.h）

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| kMaxFramesInFlight | 常量 | 建议 2～4；实现可配置 |
| PipelineConfig | struct | frameInFlightCount（2～4） |
| FrameSlotId | 类型别名 | uint32_t，[0, frameInFlightCount) |

## 2. FrameContext（te/pipelinecore/FrameContext.h）— 新增

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| FrameContext | struct | scene (ISceneWorld const*), camera (void const*), viewport (ViewportDesc), frameSlotId (FrameSlotId) 等 |
| ViewportDesc | struct | width, height 等视口参数 |

## 3. ISceneWorld（te/pipelinecore/FrameGraph.h）— 新增

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| ISceneWorld | 抽象接口 | 最小接口；020/004 实现；SetScene 绑定 |

## 4. FrameGraph（te/pipelinecore/FrameGraph.h）

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| IFrameGraph::AddPass | 成员 | `IPassBuilder* AddPass(char const* name);` |
| IFrameGraph::Compile | 成员 | `bool Compile();` |
| IPassBuilder::SetScene | 成员 | `void SetScene(ISceneWorld const* scene);` |
| IPassBuilder::SetCullMode | 成员 | `void SetCullMode(CullMode mode);` |
| IPassBuilder::SetObjectTypeFilter | 成员 | `void SetObjectTypeFilter(...);` |
| IPassBuilder::SetRenderType | 成员 | `void SetRenderType(RenderType type);` |
| IPassBuilder::SetOutput | 成员 | `void SetOutput(PassOutputDesc const& desc);` |
| IPassBuilder::SetExecuteCallback | 成员 | `void SetExecuteCallback(PassExecuteCallback cb);` |
| CullMode | enum | None, FrustumCull, OcclusionCull, FrustumAndOcclusion |
| RenderType | enum | Opaque, Transparent, Overlay, Custom |
| PassOutputDesc | struct | 渲染目标、深度、多 RT、分辨率、格式等 |
| PassContext | struct | GetCollectedObjects() -> IRenderObjectList const* |
| PassExecuteCallback | 回调 | `void (*)(PassContext& ctx, ICommandList* cmd);` |
| IRenderObjectList | 接口 | 只读列表/迭代器 |

## 5. LogicalPipeline（te/pipelinecore/LogicalPipeline.h）

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| ILogicalPipeline | 抽象/struct | 含 Pass 列表、每 Pass 收集配置与输出 |
| BuildLogicalPipeline | 自由函数 | `ILogicalPipeline* BuildLogicalPipeline(IFrameGraph const* graph, FrameContext const& ctx);` |

## 6. RenderItem（te/pipelinecore/RenderItem.h）

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| RenderItem | struct | 场景模型、UI、材质、排序 key 等 |
| IRenderItemList | 接口 | 合并后的 RenderItem 列表 |
| CreateRenderItem | 自由函数 | `RenderItem* CreateRenderItem(...);` |
| PrepareRenderResources | 自由函数 | **修改**：`ResultCode PrepareRenderResources(IRenderItemList const* items, IDevice* device);` 必须在线程 D |
| PrepareRenderMaterial | 自由函数 | **修改**：`ResultCode PrepareRenderMaterial(IMaterialHandle const* material, IDevice* device);` 必须在线程 D |
| PrepareRenderMesh | 自由函数 | **修改**：`ResultCode PrepareRenderMesh(IMeshHandle const* mesh, IDevice* device);` 必须在线程 D |

注：IMaterialHandle、IMeshHandle 为前向声明；020/011/012 提供具体类型。

## 7. CollectPass（te/pipelinecore/CollectPass.h）

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| CollectRenderItemsParallel | 自由函数 | `void CollectRenderItemsParallel(ILogicalPipeline const* pipeline, FrameContext const& ctx, IRenderItemList* out);` |
| MergeRenderItems | 自由函数 | `void MergeRenderItems(IRenderItemList const* partial_lists, size_t count, IRenderItemList* merged);` |

## 8. LogicalCommandBuffer（te/pipelinecore/LogicalCommandBuffer.h）

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| ILogicalCommandBuffer | 抽象接口 | CPU 侧逻辑命令序列 |
| ConvertToLogicalCommandBuffer | 自由函数 | **修改**：`ResultCode ConvertToLogicalCommandBuffer(IRenderItemList const* items, ILogicalPipeline const* pipeline, ILogicalCommandBuffer** out);` 必须在线程 D |
| CollectCommandBuffer | 别名 | 同 ConvertToLogicalCommandBuffer |

## 9. Profiling（te/pipelinecore/Profiling.h）— 新增

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| TE_PIPELINECORE_PROFILING | 宏 | 启用 profiling 时定义 |
| PassProfilingScope | struct | RAII，Pass 开始/结束计时 |
| OnCompileProfiling | 回调/宏 | Compile 耗时回调 |

---

**ResultCode**：使用 009-RenderCore 的 `te::rendercore::ResultCode`（Success, InvalidHandle, ValidationFailed, Unknown 等）。
