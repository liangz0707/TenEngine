# 012-Mesh 模块 ABI

- **契约**：[012-mesh-public-api.md](./012-mesh-public-api.md)（能力与类型描述）
- **本文件**：012-Mesh 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| 012-Mesh | te::mesh | — | 网格句柄 | te/mesh/Mesh.h | MeshHandle | 不透明句柄（detail::MeshData*）；CreateMesh 返回，ReleaseMesh 释放；仅持 CPU 顶点/索引及子网格、LOD、蒙皮元数据 |
| 012-Mesh | te::mesh | — | 网格描述（归属 012） | te/mesh/MeshAssetDesc.h | MeshAssetDesc | formatVersion, debugDescription, vertexLayout, vertexData（指针，序列化时不保存）, vertexDataSize, indexData（指针，序列化时不保存）, indexDataSize, indexFormat, submeshes, lodLevels, skinningData（可选）；IsValid() 方法；.mesh 为其序列化格式，.meshdata 为数据文件 |
| 012-Mesh | te::mesh | — | 子网格描述 | te/mesh/Mesh.h | SubmeshDesc | offset, count, materialSlotIndex；DrawCall 批次 |
| 012-Mesh | te::mesh | — | LOD 级别 | te/mesh/Mesh.h | LODLevel | distanceThreshold, screenSizeThreshold, submeshStartIndex, submeshCount |
| 012-Mesh | te::mesh | — | LOD 句柄 | te/mesh/Mesh.h | LODHandle | `using LODHandle = uint32_t;` LOD 级别索引 |
| 012-Mesh | te::mesh | — | 蒙皮数据 | te/mesh/Mesh.h | SkinningData | boneIndices, boneIndicesCount, boneWeights, boneWeightsCount, bindPoseMatrices, bindPoseMatrixCount, boneCount, vertexCount；与 015-Animation 对接 |
| 012-Mesh | te::mesh | — | 创建网格（仅内存） | te/mesh/MeshFactory.h | CreateMesh | `MeshHandle CreateMesh(MeshAssetDesc const* desc);` 入参由 013 传入；013 Loader 内调用；复制顶点/索引数据到内部存储 |
| 012-Mesh | te::mesh | — | 释放网格 | te/mesh/MeshFactory.h | ReleaseMesh | `void ReleaseMesh(MeshHandle h);` 释放顶点/索引内存数据与句柄；不释放 GPU 资源（由 MeshResource 管理） |
| 012-Mesh | te::mesh | — | 子网格数量 | te/mesh/Mesh.h | GetSubmeshCount | `uint32_t GetSubmeshCount(MeshHandle h);` |
| 012-Mesh | te::mesh | — | 取子网格 | te/mesh/Mesh.h | GetSubmesh | `SubmeshDesc const* GetSubmesh(MeshHandle h, uint32_t index);` 返回 nullptr 如果索引无效 |
| 012-Mesh | te::mesh | — | LOD 数量 | te/mesh/Mesh.h | GetLODCount | `uint32_t GetLODCount(MeshHandle h);` |
| 012-Mesh | te::mesh | — | 选择 LOD | te/mesh/Mesh.h | SelectLOD | `uint32_t SelectLOD(MeshHandle h, float distanceOrScreenSize);` 根据距离或屏幕尺寸选择 LOD 级别 |
| 012-Mesh | te::mesh | — | 取 LOD 级别描述 | te/mesh/Mesh.h | GetLODLevel | `bool GetLODLevel(MeshHandle h, uint32_t lodIndex, LODLevel* out);` 返回指定 LOD 的 submeshStartIndex/submeshCount 等；020 收集阶段仅对选中 LOD 的 submesh 生成 RenderItem |
| 012-Mesh | te::mesh | — | 流式请求 | te/mesh/Mesh.h | RequestStreaming | `void RequestStreaming(MeshHandle h, uint32_t lodLevel);` 与 013 RequestStreaming/StreamingHandle 对接（可选） |
| 012-Mesh | te::mesh | — | 蒙皮数据 | te/mesh/Mesh.h | GetSkinningData | `SkinningData const* GetSkinningData(MeshHandle h);` 无蒙皮返回 nullptr |
| 012-Mesh | te::mesh | — | Mesh 局部 AABB | te/mesh/Mesh.h | GetMeshAABB | `te::core::AABB GetMeshAABB(MeshHandle h);` 局部空间 AABB；020/029 在节点无 AABB 时可用 worldMatrix 变换后做视锥剔除 |
| 012-Mesh | te::mesh | — | Submesh 局部 AABB | te/mesh/Mesh.h | GetSubmeshAABB | `te::core::AABB GetSubmeshAABB(MeshHandle h, uint32_t submeshIndex);` 当前可返回 mesh AABB |
| 012-Mesh | te::mesh | — | 顶点步长 | te/mesh/Mesh.h | GetVertexStride | `uint32_t GetVertexStride(MeshHandle h);` 返回顶点步长（字节） |
| 012-Mesh | te::mesh | — | 索引格式 | te/mesh/Mesh.h | GetIndexFormat | `uint32_t GetIndexFormat(MeshHandle h);` 返回 0 表示 16 位，1 表示 32 位 |
| 012-Mesh | te::mesh | MeshResource | 网格资源类 | te/mesh/MeshResource.h | MeshResource | 实现 IMeshResource/IResource；管理网格数据生命周期：Load, LoadAsync, Save, Import, IsDeviceReady；GetMeshHandle, GetVertexData, GetVertexDataSize, GetIndexData, GetIndexDataSize, SetMeshHandle |
| 012-Mesh | te::mesh | MeshResource | 获取资源类型 | te/mesh/MeshResource.h | MeshResource::GetResourceType | `ResourceType GetResourceType() const override;` 返回 ResourceType::Mesh |
| 012-Mesh | te::mesh | MeshResource | 获取资源 ID | te/mesh/MeshResource.h | MeshResource::GetResourceId | `ResourceId GetResourceId() const override;` 返回资源 GUID |
| 012-Mesh | te::mesh | MeshResource | 释放资源 | te/mesh/MeshResource.h | MeshResource::Release | `void Release() override;` 递减引用计数，释放网格句柄 |
| 012-Mesh | te::mesh | MeshResource | 同步加载 | te/mesh/MeshResource.h | MeshResource::Load | `bool Load(char const* path, IResourceManager* manager) override;` 加载 .mesh 和 .meshdata 文件；调用 CreateMesh 创建句柄 |
| 012-Mesh | te::mesh | MeshResource | 异步加载 | te/mesh/MeshResource.h | MeshResource::LoadAsync | `bool LoadAsync(char const* path, IResourceManager* manager, LoadCompleteCallback on_done, void* user_data) override;` 使用基类默认实现（IThreadPool 后台线程执行 Load） |
| 012-Mesh | te::mesh | MeshResource | 保存资源 | te/mesh/MeshResource.h | MeshResource::Save | `bool Save(char const* path, IResourceManager* manager) override;` 保存 .mesh 和 .meshdata 文件；从 MeshHandle 提取数据 |
| 012-Mesh | te::mesh | MeshResource | 导入资源 | te/mesh/MeshResource.h | MeshResource::Import | `bool Import(char const* sourcePath, IResourceManager* manager) override;` 从外部格式（OBJ/glTF/FBX）导入并转换为引擎格式 |
| 012-Mesh | te::mesh | MeshResource | 设备就绪状态 | te/mesh/MeshResource.h | MeshResource::IsDeviceReady | `bool IsDeviceReady() const override;` 返回 mesh 是否已加载（CPU 数据就绪） |
| 012-Mesh | te::mesh | — | 全屏四边形 | te/mesh/BuiltinMeshes.h | GetFullscreenQuadMesh | `MeshResource const* GetFullscreenQuadMesh();` 缓存单例，勿 delete |
| 012-Mesh | te::mesh | — | 球体 | te/mesh/BuiltinMeshes.h | GetSphereMesh | `MeshResource const* GetSphereMesh(float radius, uint32_t segments);` |
| 012-Mesh | te::mesh | — | 半球 | te/mesh/BuiltinMeshes.h | GetHemisphereMesh | `MeshResource const* GetHemisphereMesh(float radius, uint32_t segments);` |
| 012-Mesh | te::mesh | — | 平面 | te/mesh/BuiltinMeshes.h | GetPlaneMesh | `MeshResource const* GetPlaneMesh(float width, float height);` |
| 012-Mesh | te::mesh | — | 矩形 | te/mesh/BuiltinMeshes.h | GetQuadMesh | `MeshResource const* GetQuadMesh(float width, float height);` 同 GetPlaneMesh |
| 012-Mesh | te::mesh | — | 三角形 | te/mesh/BuiltinMeshes.h | GetTriangleMesh | `MeshResource const* GetTriangleMesh();` |
| 012-Mesh | te::mesh | — | 立方体 | te/mesh/BuiltinMeshes.h | GetCubeMesh | `MeshResource const* GetCubeMesh(float size);` |
| 012-Mesh | te::mesh | — | 锥体 | te/mesh/BuiltinMeshes.h | GetConeMesh | `MeshResource const* GetConeMesh(float radius, float height, uint32_t segments);` |
| 012-Mesh | te::mesh | IResourceLoader | Mesh 类型 Loader | te/mesh/MeshLoader.h | MeshResourceLoader::CreateFromPayload | `IResource* CreateFromPayload(ResourceType type, void* payload, IResourceManager* manager);` type==Mesh 时将 payload 解释为 MeshAssetDesc*，CreateMesh 后包装为 MeshResource* 返回 |
| 012-Mesh | te::mesh | IDeserializer | Mesh 反序列化 | te/mesh/MeshDeserializer.h | MeshDeserializer::Deserialize | `void* Deserialize(void const* buffer, size_t size);` 产出 MeshAssetDesc*（payload），通过 002-Object 反序列化；013 不解析 |
| 012-Mesh | te::mesh | — | Save 时产出内存 | te/mesh/MeshSerialize.h | SerializeMeshToBuffer | `bool SerializeMeshToBuffer(MeshHandle h, void* buffer, size_t* size);` 序列化 MeshHandle 到缓冲区；013 Save 时按类型调用，012 产出可写盘内容 |
| 012-Mesh | te::mesh | — | OBJ 导入器 | te/mesh/MeshImporters.h | ImportMeshFromFastObj | `bool ImportMeshFromFastObj(char const* sourcePath, MeshAssetDesc* outDesc);` 使用 fast_obj 库导入 OBJ 文件；需要 TENENGINE_USE_FAST_OBJ |
| 012-Mesh | te::mesh | — | Assimp 导入器 | te/mesh/MeshImporters.h | ImportMeshFromAssimp | `bool ImportMeshFromAssimp(char const* sourcePath, MeshAssetDesc* outDesc);` 使用 Assimp 库导入多种格式（OBJ/FBX/glTF 等）；需要 TENENGINE_USE_ASSIMP |
| 012-Mesh | te::mesh | — | glTF 导入器 | te/mesh/MeshImporters.h | ImportMeshFromCgltf | `bool ImportMeshFromCgltf(char const* sourcePath, MeshAssetDesc* outDesc);` 使用 cgltf 库导入 glTF/glB 文件；需要 TENENGINE_USE_CGLTF |
| 012-Mesh | te::mesh | — | 模块初始化 | te/mesh/MeshModuleInit.h | InitializeMeshModule | `void InitializeMeshModule(IResourceManager* manager);` 注册资源工厂到 ResourceManager |

*VertexFormat、IndexFormat、BufferLayout 使用 009-RenderCore 契约类型；CreateBuffer/DestroyBuffer 通过 030-DeviceResourceManager 调用 008-RHI 契约。*

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| 2026-02-10 | ABI 同步：增加 GetLODLevel(MeshHandle, lodIndex, LODLevel*)、GetMeshAABB、GetSubmeshAABB |
| 2026-02-22 | 同步代码：移除 MeshRenderer 相关符号（待实现）；新增 GetVertexStride、GetIndexFormat；更新 MeshResource 方法列表（移除 EnsureDeviceResources 相关）；更新 MeshAssetDesc 添加 IsValid() 方法说明 |
