# 契约：020-Pipeline 模块对外 API

## 适用模块

- **实现方**：**020-Pipeline**（渲染管线实现：场景到屏幕、剔除、DrawCall、命令缓冲与提交）
- **对应规格**：`docs/module-specs/020-pipeline.md`
- **依赖**：Core、Scene、Entity、PipelineCore、RenderCore、Shader、Material、Mesh、Resource（见 `000-module-dependency-map.md`）

## 消费者（T0 下游）

- 021-Effects（将 Effects Pass 纳入管线）
- 022-2D（2D Pass、排序与绘制）
- 023-Terrain（地形 Pass、DrawCall）
- 024-Editor（视口渲染、Gizmo、离屏/窗口 RT）
- 027-XR（提交到 XR 交换链）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| PipelineContext | 管线上下文；可见集、批次、Pass 图、与 PipelineCore 对接 | 单帧或单次渲染 |
| RenderTargetHandle | 渲染目标句柄；PresentTarget、与 RHI/SwapChain/XR 对接 | 由 Pipeline 或调用方管理 |
| DrawCall 接口 | 批次、材质/网格/变换、实例化与合批；与 Material/Mesh/Shader 对接 | 单次 Pass 或帧 |
| VisibleSet / BatchList | 可见实体/组件、批次列表；CollectVisible、FrustumCull、SelectLOD、BuildBatches | 单帧 |

下游仅通过上述类型与句柄访问；命令缓冲与 RHI 的提交约定见 **pipeline-to-rci.md**。

## 能力列表（提供方保证）

1. **Culling**：CollectVisible、FrustumCull、OcclusionQuery（可选）、SelectLOD；与 Scene/Entity 对接。
2. **Batching**：BuildBatches、MaterialSlot、MeshSlot、Transform、Instancing、MergeBatch。
3. **PassExecution**：ExecutePass、GBuffer、Lighting、PostProcess；与 PipelineCore 图对接。
4. **Submit**：BuildCommandBuffer、SubmitToRHI、Present、XRSubmit（可选）；与 RHI/SwapChain/XR 对接。

## 调用顺序与约束

- 须在所有上游模块（Core、Scene、Entity、PipelineCore、RenderCore、Shader、Material、Mesh、Resource）初始化之后使用。
- 与 RHI 的提交格式与时机见 **pipeline-to-rci.md**；与 Editor/XR 的视口与交换链对接须明确。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：020-Pipeline 对应本契约；与 docs/module-specs/020-pipeline.md 一致 |
