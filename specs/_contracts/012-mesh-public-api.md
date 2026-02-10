# 契约：012-Mesh 模块对外 API

## 适用模块

- **实现方**：012-Mesh（L2；网格数据、LOD、蒙皮、顶点/索引；EnsureDeviceResources 时通过 030-DeviceResourceManager 调用 008-RHI 创建缓冲）
- **对应规格**：`docs/module-specs/012-mesh.md`
- **依赖**：001-Core、002-Object、008-RHI、009-RenderCore、013-Resource、030-DeviceResourceManager

## 消费者

- 013-Resource（Load(Mesh) 时调用 012 CreateMesh）、020-Pipeline、023-Terrain、015-Animation

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| MeshHandle | 网格句柄（detail::MeshData*）；CreateMesh(MeshAssetDesc) 仅接受内存；EnsureDeviceResources 时通过 030-DeviceResourceManager 调用 008-RHI 创建顶点/索引缓冲（DResource） | 创建后直至显式释放 |
| MeshAssetDesc | 网格资产描述；包含顶点/索引格式、数据指针、子网格、LOD、蒙皮等；序列化格式为 .mesh（元数据）+ .meshdata（二进制数据） | 由 013 解析后传入 CreateMesh |
| SubmeshDesc | 子网格划分、材质槽位、DrawCall 批次；offset, count, materialSlotIndex | 与 Mesh 绑定 |
| LODLevel | LOD 级别描述；distanceThreshold, screenSizeThreshold, submeshStartIndex, submeshCount | 与 Mesh 绑定 |
| LODHandle | LOD 句柄（uint32_t）；LOD 级别索引 | 与 Mesh 绑定 |
| SkinningData | 骨骼索引与权重、BindPose；boneIndices, boneWeights, bindPoseMatrices, boneCount, vertexCount；与 Animation 骨骼矩阵对接 | 与 Mesh 绑定（可选） |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | VertexIndex | VertexFormat、IndexFormat、BufferLayout；与 RenderCore 格式映射；支持同步/异步 GPU 缓冲创建 |
| 2 | Submesh | SubmeshCount、GetSubmesh、MaterialSlot、DrawCall 批次 |
| 3 | LOD | LODCount、SelectLOD、RequestStreaming；与 Resource 对接 |
| 4 | Skinning | BoneIndices、Weights、BindPose；与 Animation 骨骼矩阵对接 |
| 5 | Resource | MeshResource 实现 IResource/IMeshResource；支持 Load、LoadAsync、Save、Import、EnsureDeviceResources、EnsureDeviceResourcesAsync |
| 6 | Import | 支持从外部格式导入：OBJ（fast_obj）、glTF/glB（cgltf）、通用格式（Assimp）；通过 MeshResource::Import 调用 |
| 7 | Serialization | MeshAssetDesc 序列化/反序列化（002-Object）；.mesh 文件存储元数据，.meshdata 文件存储二进制数据 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Object、RHI、RenderCore、Resource、DeviceResourceManager 初始化之后使用。
- 013 解析得到内存数据（MeshAssetDesc）后调用 012 CreateMesh。
- DResource（GPU 缓冲）在下游触发 EnsureDeviceResources 时由 012 通过 030-DeviceResourceManager 调用 008-RHI 创建。
- 蒙皮数据与 Animation 骨骼名称/索引约定须一致。
- MeshAssetDesc 的 vertexData 和 indexData 指针在序列化时设置为 nullptr，实际数据存储在 .meshdata 文件中。
- 导入器需要相应的编译定义（TENENGINE_USE_FAST_OBJ、TENENGINE_USE_ASSIMP、TENENGINE_USE_CGLTF）才能使用。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 012-Mesh 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
| 2026-02-05 | ABI 写回（plan 012-mesh-full-module-001）：MeshHandle、MeshAssetDesc、SubmeshDesc、LODLevel、SkinningData、CreateMesh、ReleaseMesh、EnsureDeviceResources、GetSubmeshCount、GetSubmesh、GetLODCount、SelectLOD、GetSkinningData、GetVertexBufferHandle/GetIndexBufferHandle、MeshResourceLoader::CreateFromPayload、MeshDeserializer::Deserialize、SerializeMeshToBuffer、与 002 注册；全量 ABI 表写入 012-mesh-ABI.md |
| 2026-02-05 | 清除 TODO 列表：plan 012-mesh-full-module-001 已完成，描述归属、CreateMesh、MeshAssetDesc/.mesh 相关任务已从 public-api 移除 |
