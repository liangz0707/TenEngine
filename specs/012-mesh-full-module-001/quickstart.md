# Quickstart: 012-Mesh 完整模块

**Branch**: `012-mesh-full-module-001`

## 1. 前置条件

- 001-Core、008-RHI、009-RenderCore、013-Resource 已初始化。
- 012 已向 013 注册 `RegisterResourceLoader(ResourceType::Mesh, MeshResourceLoader)` 与 `RegisterDeserializer(ResourceType::Mesh, MeshDeserializer)`。

## 2. 通过 013 加载 Mesh（推荐）

1. 调用方通过 013 统一入口加载：`RequestLoadAsync(path, ResourceType::Mesh, callback)` 或 `LoadSync(path, ResourceType::Mesh)`。
2. 013 读 .mesh（及 .meshdata 若存在）→ 调 012 的 Deserialize 得 payload → 调 012 的 Loader CreateFromPayload(Mesh, payload, manager) → 返回 IResource*（即 IMeshResource*）。
3. 调用方在回调或同步返回后取得 `IMeshResource*`，可向下转型为 `IMeshResource*` 使用。

## 3. 直接使用 012 API（如测试或 013 内部）

1. **创建**: 准备 `MeshAssetDesc`（formatVersion、vertexLayout、vertexData、indexData、indexFormat、submeshes 等），调用 `CreateMesh(&desc)` 得到 `MeshHandle`。
2. **设备资源**: 在需要绘制前调用 `EnsureDeviceResources(handle, device)`；012 内部调 008 CreateBuffer 创建顶点/索引缓冲。
3. **查询**: `GetSubmeshCount(handle)`、`GetSubmesh(handle, i)`、`GetLODCount(handle)`、`SelectLOD(handle, distanceOrScreenSize)`、`GetSkinningData(handle)`；`GetVertexBufferHandle(handle)` / `GetIndexBufferHandle(handle)` 在 Ensure 后可用。
4. **释放**: `ReleaseMesh(handle)`；若为 013 加载得到的 IResource*，则调用 `IResource::Release()`，013 再与 012 协调释放。

## 4. Save（写盘）

013 调用 `Save(IResource*, path)` 时，对 Mesh 类型会向 012 请求内存内容；012 提供 `SerializeMeshToBuffer(handle, buffer, size)` 产出 .mesh 布局，013 将 buffer 写入 path。

## 5. 参考

- 规约：`docs/module-specs/012-mesh.md`
- 契约：`specs/_contracts/012-mesh-public-api.md`
- 全量 ABI：`specs/012-mesh-full-module-001/plan.md` §全量 ABI 内容
- Asset 加载流程：`docs/asset/02-asset-loading-flow.md`
