# 012-Mesh 契约与 ABI（本 feature）

本目录为 012-Mesh 与 013-Resource 对接的接口摘要；**全量 ABI 内容**见 [../plan.md](../plan.md) 中「全量 ABI 内容（实现参考）」表。

## 与 013 的对接接口

- **IResourceLoader**: 012 实现 `MeshResourceLoader::CreateFromPayload(ResourceType type, void* payload, IResourceManager* manager)`；type == Mesh 时，payload 为 `MeshAssetDesc*`，内部调用 `CreateMesh(desc)`，返回实现 IResource/IMeshResource 的对象。
- **IDeserializer**: 012 实现 `MeshDeserializer::Deserialize(void const* buffer, size_t size)`，产出 `MeshAssetDesc*`（opaque payload）；013 读 .mesh 后调用，得到 payload 再调 Loader。
- **Save**: 012 提供 `SerializeMeshToBuffer(MeshHandle h, void* buffer, size_t* size)`（或等价）；013 Save(IResource*, path) 时按 ResourceType::Mesh 调用，012 产出可写盘内存，013 负责写文件。

## 权威来源

- 契约：`specs/_contracts/012-mesh-public-api.md`
- ABI 表：`specs/_contracts/012-mesh-ABI.md`（写回后）；实现时以 plan 全量 ABI 表为准。
