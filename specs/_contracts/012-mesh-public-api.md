# 契约：012-Mesh 模块对外 API

## 适用模块

- **实现方**：012-Mesh（L2；网格数据、LOD、蒙皮、顶点/索引；EnsureDeviceResources 时依赖 008-RHI 创建缓冲）
- **对应规格**：`docs/module-specs/012-mesh.md`
- **依赖**：001-Core、008-RHI、009-RenderCore、013-Resource

## 消费者

- 013-Resource（Load(Mesh) 时调用 012 CreateMesh）、020-Pipeline、023-Terrain、015-Animation

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| MeshHandle | 网格句柄；CreateMesh(vertexData, indexData, layout, submeshes) 仅接受内存；EnsureDeviceResources 时 012 调用 008-RHI 创建顶点/索引缓冲（DResource） | 创建后直至显式释放 |
| SubmeshDesc | 子网格划分、材质槽位、DrawCall 批次 | 与 Mesh 绑定 |
| LODHandle | LOD 级别、SelectLOD、StreamingRequest、与 Resource 对接 | 与 Mesh 绑定 |
| SkinningData | 骨骼索引与权重、BindPose、与 Animation 骨骼矩阵对接 | 与 Mesh 绑定 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | VertexIndex | VertexFormat、IndexFormat、BufferLayout；与 RenderCore 格式映射 |
| 2 | Submesh | SubmeshCount、GetSubmesh、MaterialSlot、DrawCall 批次 |
| 3 | LOD | LODCount、SelectLOD、StreamingRequest；与 Resource 对接 |
| 4 | Skinning | BoneIndices、Weights、BindPose；与 Animation 骨骼矩阵对接 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、RHI、RenderCore、Resource 初始化之后使用。013 解析得到内存数据后调用 012 CreateMesh；DResource 在下游触发 EnsureDeviceResources 时由 012 调用 008 创建。蒙皮数据与 Animation 骨骼名称/索引约定须一致。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 012-Mesh 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
| 2026-02-05 | ABI 写回（plan 012-mesh-full-module-001）：MeshHandle、MeshAssetDesc、SubmeshDesc、LODLevel、SkinningData、CreateMesh、ReleaseMesh、EnsureDeviceResources、GetSubmeshCount、GetSubmesh、GetLODCount、SelectLOD、GetSkinningData、GetVertexBufferHandle/GetIndexBufferHandle、MeshResourceLoader::CreateFromPayload、MeshDeserializer::Deserialize、SerializeMeshToBuffer、与 002 注册；全量 ABI 表写入 012-mesh-ABI.md |
| 2026-02-05 | 清除 TODO 列表：plan 012-mesh-full-module-001 已完成，描述归属、CreateMesh、MeshAssetDesc/.mesh 相关任务已从 public-api 移除 |
