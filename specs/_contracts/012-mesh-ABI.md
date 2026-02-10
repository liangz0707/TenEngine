# 012-Mesh 模块 ABI

- **契约**：[012-mesh-public-api.md](./012-mesh-public-api.md)（能力与类型描述）
- **本文件**：012-Mesh 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| 012-Mesh | te::mesh | — | 网格句柄 | te/mesh/Mesh.h | MeshHandle | 不透明句柄（detail::MeshData*）；CreateMesh 返回，ReleaseMesh 释放；内部持顶点/索引 DResource 槽位及子网格、LOD、蒙皮元数据 |
| 012-Mesh | te::mesh | — | 网格描述（归属 012） | te/mesh/MeshAssetDesc.h | MeshAssetDesc | formatVersion, debugDescription, vertexLayout, vertexData（指针，序列化时不保存）, vertexDataSize, indexData（指针，序列化时不保存）, indexDataSize, indexFormat, submeshes, lodLevels, skinningData（可选）；.mesh 为其序列化格式，.meshdata 为数据文件 |
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
| 012-Mesh | te::mesh | — | 流式请求 | te/mesh/Mesh.h | RequestStreaming | `void RequestStreaming(MeshHandle h, uint32_t lodLevel);` 与 013 RequestStreaming/StreamingHandle 对接（可选） |
| 012-Mesh | te::mesh | — | 蒙皮数据 | te/mesh/Mesh.h | GetSkinningData | `SkinningData const* GetSkinningData(MeshHandle h);` 无蒙皮返回 nullptr |
| 012-Mesh | te::mesh | — | 确保设备缓冲（同步） | te/mesh/MeshDevice.h | EnsureDeviceResources | `bool EnsureDeviceResources(MeshHandle h, rhi::IDevice* device);` 同步创建 GPU 顶点/索引缓冲；通过 030-DeviceResourceManager::CreateDeviceBuffer 创建 |
| 012-Mesh | te::mesh | — | 确保设备缓冲（异步） | te/mesh/MeshDevice.h | EnsureDeviceResourcesAsync | `void EnsureDeviceResourcesAsync(MeshHandle h, rhi::IDevice* device, void (*on_done)(void*), void* user_data);` 异步创建 GPU 顶点/索引缓冲；通过 030-DeviceResourceManager::CreateDeviceBufferAsync 创建；回调链：顶点缓冲完成→索引缓冲完成→用户回调 |
| 012-Mesh | te::mesh | — | 顶点缓冲句柄 | te/mesh/MeshDevice.h | GetVertexBufferHandle | `rhi::IBuffer* GetVertexBufferHandle(MeshHandle h);` EnsureDeviceResources 后可用；返回 nullptr 如果未创建 |
| 012-Mesh | te::mesh | — | 索引缓冲句柄 | te/mesh/MeshDevice.h | GetIndexBufferHandle | `rhi::IBuffer* GetIndexBufferHandle(MeshHandle h);` EnsureDeviceResources 后可用；返回 nullptr 如果未创建 |
| 012-Mesh | te::mesh | MeshResource | 网格资源类 | te/mesh/MeshResource.h | MeshResource | 实现 IMeshResource/IResource；管理网格数据生命周期：Load, LoadAsync, Save, Import, EnsureDeviceResources, EnsureDeviceResourcesAsync |
| 012-Mesh | te::mesh | MeshResource | 获取资源类型 | te/mesh/MeshResource.h | MeshResource::GetResourceType | `ResourceType GetResourceType() const override;` 返回 ResourceType::Mesh |
| 012-Mesh | te::mesh | MeshResource | 获取资源 ID | te/mesh/MeshResource.h | MeshResource::GetResourceId | `ResourceId GetResourceId() const override;` 返回资源 GUID |
| 012-Mesh | te::mesh | MeshResource | 释放资源 | te/mesh/MeshResource.h | MeshResource::Release | `void Release() override;` 递减引用计数，释放网格句柄和 GPU 资源 |
| 012-Mesh | te::mesh | MeshResource | 同步加载 | te/mesh/MeshResource.h | MeshResource::Load | `bool Load(char const* path, IResourceManager* manager) override;` 加载 .mesh 和 .meshdata 文件；调用 CreateMesh 创建句柄 |
| 012-Mesh | te::mesh | MeshResource | 异步加载 | te/mesh/MeshResource.h | MeshResource::LoadAsync | `bool LoadAsync(char const* path, IResourceManager* manager, LoadCompleteCallback on_done, void* user_data) override;` 使用基类默认实现（IThreadPool 后台线程执行 Load） |
| 012-Mesh | te::mesh | MeshResource | 保存资源 | te/mesh/MeshResource.h | MeshResource::Save | `bool Save(char const* path, IResourceManager* manager) override;` 保存 .mesh 和 .meshdata 文件；从 MeshHandle 提取数据 |
| 012-Mesh | te::mesh | MeshResource | 导入资源 | te/mesh/MeshResource.h | MeshResource::Import | `bool Import(char const* sourcePath, IResourceManager* manager) override;` 从外部格式（OBJ/glTF/FBX）导入并转换为引擎格式 |
| 012-Mesh | te::mesh | MeshResource | 确保设备资源（同步） | te/mesh/MeshResource.h | MeshResource::EnsureDeviceResources | `void EnsureDeviceResources() override;` 调用全局 EnsureDeviceResources，更新内部 GPU 缓冲句柄 |
| 012-Mesh | te::mesh | MeshResource | 确保设备资源（异步） | te/mesh/MeshResource.h | MeshResource::EnsureDeviceResourcesAsync | `void EnsureDeviceResourcesAsync(void (*on_done)(void*), void* user_data) override;` 调用全局 EnsureDeviceResourcesAsync，在回调中更新内部 GPU 缓冲句柄 |
| 012-Mesh | te::mesh | MeshResource | 获取网格句柄 | te/mesh/MeshResource.h | MeshResource::GetMeshHandle | `MeshHandle GetMeshHandle() const;` 返回内部网格句柄 |
| 012-Mesh | te::mesh | MeshResource | 获取顶点数据 | te/mesh/MeshResource.h | MeshResource::GetVertexData | `void const* GetVertexData() const;` 返回顶点数据指针 |
| 012-Mesh | te::mesh | MeshResource | 获取顶点数据大小 | te/mesh/MeshResource.h | MeshResource::GetVertexDataSize | `size_t GetVertexDataSize() const;` 返回顶点数据大小（字节） |
| 012-Mesh | te::mesh | MeshResource | 获取索引数据 | te/mesh/MeshResource.h | MeshResource::GetIndexData | `void const* GetIndexData() const;` 返回索引数据指针 |
| 012-Mesh | te::mesh | MeshResource | 获取索引数据大小 | te/mesh/MeshResource.h | MeshResource::GetIndexDataSize | `size_t GetIndexDataSize() const;` 返回索引数据大小（字节） |
| 012-Mesh | te::mesh | MeshResource | 设置设备 | te/mesh/MeshResource.h | MeshResource::SetDevice | `void SetDevice(rhi::IDevice* device);` 设置 RHI 设备用于 GPU 资源创建 |
| 012-Mesh | te::mesh | MeshResource | 获取设备顶点缓冲 | te/mesh/MeshResource.h | MeshResource::GetDeviceVertexBuffer | `rhi::IBuffer* GetDeviceVertexBuffer() const;` 返回 GPU 顶点缓冲句柄 |
| 012-Mesh | te::mesh | MeshResource | 获取设备索引缓冲 | te/mesh/MeshResource.h | MeshResource::GetDeviceIndexBuffer | `rhi::IBuffer* GetDeviceIndexBuffer() const;` 返回 GPU 索引缓冲句柄 |
| 012-Mesh | te::mesh | MeshResource | 设置网格句柄 | te/mesh/MeshResource.h | MeshResource::SetMeshHandle | `void SetMeshHandle(MeshHandle handle);` 由 MeshLoader::CreateFromPayload 调用 |
| 012-Mesh | te::mesh | IResourceLoader | Mesh 类型 Loader | te/mesh/MeshLoader.h | MeshResourceLoader::CreateFromPayload | `IResource* CreateFromPayload(ResourceType type, void* payload, IResourceManager* manager);` type==Mesh 时将 payload 解释为 MeshAssetDesc*，CreateMesh 后包装为 MeshResource* 返回 |
| 012-Mesh | te::mesh | IDeserializer | Mesh 反序列化 | te/mesh/MeshDeserializer.h | MeshDeserializer::Deserialize | `void* Deserialize(void const* buffer, size_t size);` 产出 MeshAssetDesc*（payload），通过 002-Object 反序列化；013 不解析 |
| 012-Mesh | te::mesh | — | Save 时产出内存 | te/mesh/MeshSerialize.h | SerializeMeshToBuffer | `bool SerializeMeshToBuffer(MeshHandle h, void* buffer, size_t* size);` 序列化 MeshHandle 到缓冲区；013 Save 时按类型调用，012 产出可写盘内容 |
| 012-Mesh | te::mesh | — | OBJ 导入器 | te/mesh/MeshImporters.h | ImportMeshFromFastObj | `bool ImportMeshFromFastObj(char const* sourcePath, MeshAssetDesc* outDesc);` 使用 fast_obj 库导入 OBJ 文件；需要 TENENGINE_USE_FAST_OBJ |
| 012-Mesh | te::mesh | — | Assimp 导入器 | te/mesh/MeshImporters.h | ImportMeshFromAssimp | `bool ImportMeshFromAssimp(char const* sourcePath, MeshAssetDesc* outDesc);` 使用 Assimp 库导入多种格式（OBJ/FBX/glTF 等）；需要 TENENGINE_USE_ASSIMP |
| 012-Mesh | te::mesh | — | glTF 导入器 | te/mesh/MeshImporters.h | ImportMeshFromCgltf | `bool ImportMeshFromCgltf(char const* sourcePath, MeshAssetDesc* outDesc);` 使用 cgltf 库导入 glTF/glB 文件；需要 TENENGINE_USE_CGLTF |
| 012-Mesh | te::mesh | — | 模块初始化 | te/mesh/MeshModuleInit.h | InitializeMeshModule | `void InitializeMeshModule(IResourceManager* manager);` 注册资源工厂和 MeshAssetDesc 类型到 002-Object TypeRegistry；TypeId: 0x01200001 |

*VertexFormat、IndexFormat、BufferLayout 使用 009-RenderCore 契约类型；CreateBuffer/DestroyBuffer 通过 030-DeviceResourceManager 调用 008-RHI 契约。*
