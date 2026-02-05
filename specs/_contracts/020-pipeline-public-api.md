# 契约：020-Pipeline 模块对外 API

## 适用模块

- **实现方**：020-Pipeline（L3；场景收集、剔除、DrawCall、命令缓冲生成、提交；015-Animation 可选）
- **对应规格**：`docs/module-specs/020-pipeline.md`
- **依赖**：001-Core、004-Scene、005-Entity、019-PipelineCore、009-RenderCore、010-Shader、011-Material、012-Mesh、028-Texture、013-Resource、021-Effects；015-Animation（可选）

## 消费者

- 021-Effects、022-2D、023-Terrain、024-Editor、027-XR

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| PipelineContext | 管线上下文；可见集、批次、Pass 图、与 PipelineCore 对接 | 单帧或单次渲染 |
| RenderTargetHandle | 渲染目标句柄；PresentTarget、与 RHI/SwapChain/XR 对接 | 由 Pipeline 或调用方管理 |
| DrawCall 接口 | 批次、材质/网格/变换、实例化与合批；与 Material/Mesh/Shader 对接 | 单次 Pass 或帧 |
| VisibleSet / BatchList | 可见实体/组件、批次列表；CollectVisible、FrustumCull、SelectLOD、BuildBatches | 单帧 |

收集：从 004-Scene/005-Entity 收集可见节点或实体（ResourceId/句柄）；经 013 解析为 IModelResource*；EnsureDeviceResources 触发 011/012/028 创建 DResource；与 019 PrepareRenderMaterial/PrepareRenderMesh 对接。命令缓冲与 RHI 提交见 `pipeline-to-rci.md`。

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | Culling | CollectVisible、FrustumCull、OcclusionQuery（可选）、SelectLOD；与 Scene/Entity 对接 |
| 2 | Batching | BuildBatches、MaterialSlot、MeshSlot、Transform、Instancing、MergeBatch；经 013 解析取 Mesh/Material |
| 3 | PassExecution | ExecutePass、GBuffer、Lighting、PostProcess；与 PipelineCore 图对接 |
| 4 | Submit | BuildCommandBuffer、SubmitToRHI、Present、XRSubmit（可选）；与 RHI/SwapChain/XR 对接 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在所有上游模块初始化之后使用。与 RHI 的提交格式与时机见 `pipeline-to-rci.md`；与 Editor/XR 的视口与交换链对接须明确。渲染模式（Debug/Hybrid/Resource）可通过编译或运行时配置；校验使用 001-Core 的 CheckWarning/CheckError。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 020-Pipeline 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
