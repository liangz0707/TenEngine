# Data Model: 012-Mesh 完整模块

**Branch**: `012-mesh-full-module-001`

## 1. 实体与关系

### 1.1 MeshHandle

- **语义**: 网格句柄；创建后直至显式 ReleaseMesh；内部持有顶点/索引 DResource 槽位（EnsureDeviceResources 后填充）、子网格、LOD、蒙皮元数据。
- **生命周期**: CreateMesh 返回 → EnsureDeviceResources 可选 → 查询/绘制 → ReleaseMesh 释放。
- **关系**: 与 MeshAssetDesc 对应（CreateMesh 入参）；与 008 IBuffer 关联（顶点/索引缓冲）。

### 1.2 MeshAssetDesc

- **归属**: 012-Mesh（`docs/asset/`、013-resource 契约）。
- **字段**:
  - formatVersion (uint32)
  - debugDescription (string, UTF-8)
  - vertexLayout（与 009 VertexFormat 一致）
  - vertexData（原始数据指针或 span）
  - indexFormat（与 009 IndexFormat 一致）
  - indexData
  - submeshes（SubmeshDesc 数组）
  - 可选：LOD 信息、SkinningData（BoneIndices、Weights、BindPose）
- **序列化**: .mesh 为其序列化/磁盘格式；与 002 注册；一目录一资源（.mesh + .meshdata + 可选 .obj/.fbx）。

### 1.3 SubmeshDesc

- **语义**: 子网格划分、材质槽位、DrawCall 批次范围。
- **字段**: offset、count、materialSlotIndex（与 ModelAssetDesc 的 materialGuids 下标对应）。
- **关系**: 与 MeshHandle 绑定；GetSubmesh(handle, index) 返回。

### 1.4 LODLevel / LODHandle

- **语义**: LOD 级别；SelectLOD(handle, distanceOrScreenSize) 返回当前应使用的 LOD 索引；与 013 RequestStreaming 对接流式加载。
- **关系**: 与 MeshHandle 绑定；多 LOD 时 LODCount > 1。

### 1.5 SkinningData

- **语义**: 骨骼索引与权重、BindPose；与 015-Animation 骨骼矩阵对接。
- **关系**: 与 MeshHandle 绑定；无蒙皮时 GetSkinningData 返回 nullptr。

## 2. 状态与校验

- **MeshHandle 有效性**: 非空句柄、未 Release 前可查询与 EnsureDeviceResources；Release 后不可使用。
- **CreateMesh 入参**: MeshAssetDesc 非空、vertexData/indexData 与 layout/indexFormat 一致、submeshes 索引范围合法；否则返回空句柄或错误。
- **EnsureDeviceResources**: 依赖链先 Ensure；008 CreateBuffer 成功后再标记设备就绪；失败可查询 IsDeviceReady 为 false。

## 3. 与 013 的边界

- **Loader**: CreateFromPayload(ResourceType::Mesh, payload, manager) → payload 解释为 MeshAssetDesc* → CreateMesh(desc) → 包装为 IResource*（实现 IResource/IMeshResource）返回。
- **Deserializer**: Deserialize(buffer, size) → 产出 MeshAssetDesc*（opaque payload）；013 不解析。
- **Save**: 013 按类型调用 012 产出内存内容；012 SerializeMeshToBuffer(handle, buffer, size) 产出 .mesh 布局，013 写盘。
