# Quickstart 验证步骤（T025）

按 `quickstart.md` 验证以下两条路径。

## 1. 通过 013 加载 Mesh

**前置**：001、008、009、013 已初始化；012 已向 013 注册 `RegisterResourceLoader(ResourceType::Mesh, MeshResourceLoader)` 与 `RegisterDeserializer(ResourceType::Mesh, MeshDeserializer)`。

1. 调用 `RequestLoadAsync(path, ResourceType::Mesh, callback)` 或 `LoadSync(path, ResourceType::Mesh)`。
2. 确认 013 读 .mesh → 调 012 Deserialize → 调 012 Loader CreateFromPayload → 返回 IResource*（IMeshResource*）。
3. 调用方取得 IMeshResource* 后可查询顶点/子网格等。

## 2. 直接使用 012 API

1. 准备 `MeshAssetDesc`，调用 `CreateMesh(&desc)` 得到 `MeshHandle`。
2. 需要绘制前调用 `EnsureDeviceResources(handle, device)`。
3. 使用 `GetSubmeshCount`、`GetSubmesh`、`GetLODCount`、`SelectLOD`、`GetSkinningData`、`GetVertexBufferHandle`/`GetIndexBufferHandle` 查询。
4. 使用完毕调用 `ReleaseMesh(handle)` 或（若经 013 加载）`IResource::Release()`。

## 3. 验收

- 路径 1：能通过 013 加载 .mesh 并得到可用的 IMeshResource*。
- 路径 2：能直接 CreateMesh → EnsureDeviceResources → 查询 → ReleaseMesh 无泄漏。

完成上述步骤后可在 tasks 中将 T025 勾选。
