# 012-Mesh 模块 ABI

- **契约**：[012-mesh-public-api.md](./012-mesh-public-api.md)（能力与类型描述）
- **本文件**：012-Mesh 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| 012-Mesh | te::mesh | — | 网格句柄 | te/mesh/Mesh.h | MeshHandle | 不透明句柄；CreateMesh 返回，ReleaseMesh 释放；内部持顶点/索引 DResource 槽位及子网格、LOD、蒙皮元数据 |
| 012-Mesh | te::mesh | — | 网格描述（归属 012） | te/mesh/MeshAssetDesc.h | MeshAssetDesc | formatVersion, debugDescription, vertexLayout, vertexData, indexData, indexFormat, submeshes, 可选 LOD/蒙皮；.mesh 为其序列化格式 |
| 012-Mesh | te::mesh | — | 子网格描述 | te/mesh/Mesh.h | SubmeshDesc | offset, count, materialSlotIndex；DrawCall 批次 |
| 012-Mesh | te::mesh | — | LOD 级别/句柄 | te/mesh/Mesh.h | LODLevel / LODHandle | LOD 级别、与 Resource 流式对接 |
| 012-Mesh | te::mesh | — | 蒙皮数据 | te/mesh/Mesh.h | SkinningData | BoneIndices, Weights, BindPose；与 015-Animation 对接 |
| 012-Mesh | te::mesh | — | 创建网格（仅内存） | te/mesh/MeshFactory.h | CreateMesh | `MeshHandle CreateMesh(MeshAssetDesc const* desc);` 入参由 013 传入；013 Loader 内调用 |
| 012-Mesh | te::mesh | — | 确保设备缓冲 | te/mesh/MeshDevice.h | EnsureDeviceResources | `bool EnsureDeviceResources(MeshHandle h, IDevice* device);` 对依赖链先 Ensure 再调 008 CreateBuffer |
| 012-Mesh | te::mesh | — | 释放网格 | te/mesh/MeshFactory.h | ReleaseMesh | `void ReleaseMesh(MeshHandle h);` 释放顶点/索引 DResource 与句柄 |
| 012-Mesh | te::mesh | — | 子网格数量 | te/mesh/Mesh.h | GetSubmeshCount | `uint32_t GetSubmeshCount(MeshHandle h);` |
| 012-Mesh | te::mesh | — | 取子网格 | te/mesh/Mesh.h | GetSubmesh | `SubmeshDesc const* GetSubmesh(MeshHandle h, uint32_t index);` |
| 012-Mesh | te::mesh | — | LOD 数量 | te/mesh/Mesh.h | GetLODCount | `uint32_t GetLODCount(MeshHandle h);` |
| 012-Mesh | te::mesh | — | 选择 LOD | te/mesh/Mesh.h | SelectLOD | `uint32_t SelectLOD(MeshHandle h, float distanceOrScreenSize);` 与契约策略一致 |
| 012-Mesh | te::mesh | — | 流式请求 | te/mesh/Mesh.h | RequestStreaming | 与 013 RequestStreaming/StreamingHandle 对接（可选） |
| 012-Mesh | te::mesh | — | 蒙皮数据 | te/mesh/Mesh.h | GetSkinningData | `SkinningData const* GetSkinningData(MeshHandle h);` 无蒙皮返回 nullptr |
| 012-Mesh | te::mesh | — | 顶点/索引缓冲句柄 | te/mesh/MeshDevice.h | GetVertexBufferHandle, GetIndexBufferHandle | EnsureDeviceResources 后可用；类型与 008 契约一致 |
| 012-Mesh | te::mesh | IResourceLoader | Mesh 类型 Loader | te/mesh/MeshLoader.h | MeshResourceLoader::CreateFromPayload | `IResource* CreateFromPayload(ResourceType type, void* payload, IResourceManager* manager);` type==Mesh 时将 payload 解释为 MeshAssetDesc*，CreateMesh 后包装为 IResource* 返回 |
| 012-Mesh | te::mesh | IDeserializer | Mesh 反序列化 | te/mesh/MeshDeserializer.h | MeshDeserializer::Deserialize | `void* Deserialize(void const* buffer, size_t size);` 产出 MeshAssetDesc*（payload），013 不解析 |
| 012-Mesh | te::mesh | — | Save 时产出内存 | te/mesh/MeshSerialize.h | SerializeMeshToBuffer | `bool SerializeMeshToBuffer(MeshHandle h, void* buffer, size_t* size);` 或等价；013 Save 时按类型调用，012 产出可写盘内容 |
| 012-Mesh | te::mesh | — | 与 002 注册描述类型 | te/mesh/MeshAssetDesc.h | — | MeshAssetDesc 与 .mesh 格式向 002 注册；一目录一资源（.mesh + .meshdata + 可选 .obj/.fbx） |

*VertexFormat、IndexFormat、BufferLayout 使用 009-RenderCore 契约类型；CreateBuffer/DestroyBuffer 使用 008-RHI 契约。*
