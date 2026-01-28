# 契约：012-Mesh 模块对外 API

## 适用模块

- **实现方**：**012-Mesh**（网格数据与几何）
- **对应规格**：`docs/module-specs/012-mesh.md`
- **依赖**：001-Core（001-core-public-api）、009-RenderCore（009-rendercore-public-api）

## 消费者（T0 下游）

- 020-Pipeline（顶点/索引、DrawCall、批次）
- 023-Terrain（地形网格格式、块网格）
- 015-Animation（蒙皮数据、骨骼索引与权重对接）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| MeshHandle | 网格句柄；顶点/索引缓冲、子网格、与 RenderCore 顶点格式对接 | 创建后直至显式释放 |
| SubmeshDesc | 子网格划分、材质槽位、DrawCall 批次 | 与 Mesh 绑定 |
| LODHandle | LOD 级别、SelectLOD、StreamingRequest、与 Resource 对接 | 与 Mesh 绑定 |
| SkinningData | 骨骼索引与权重、BindPose、与 Animation 骨骼矩阵对接 | 与 Mesh 绑定 |

下游仅通过上述类型与句柄访问；与 RHI Buffer 的创建/绑定通过 Pipeline 或 RenderCore 桥接。

## 能力列表（提供方保证）

1. **VertexIndex**：VertexFormat、IndexFormat、BufferLayout；与 RenderCore 格式映射。
2. **Submesh**：SubmeshCount、GetSubmesh、MaterialSlot、DrawCall 批次。
3. **LOD**：LODCount、SelectLOD、StreamingRequest；与 Resource 对接。
4. **Skinning**：BoneIndices、Weights、BindPose；与 Animation 骨骼矩阵对接。

## 调用顺序与约束

- 须在 Core、RenderCore 初始化之后使用；顶点格式与 RenderCore/RHI 一致。
- 蒙皮数据与 Animation 模块的骨骼名称/索引约定须一致。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：012-Mesh 对应本契约；与 docs/module-specs/012-mesh.md 一致 |
