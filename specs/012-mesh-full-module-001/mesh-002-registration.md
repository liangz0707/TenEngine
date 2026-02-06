# MeshAssetDesc 与 002-Object 注册说明（T022）

**目的**：满足 data-model 与 asset 设计中对「与 002 注册描述类型」及「一目录一资源」的约定。

## 与 002-Object 注册

- **MeshAssetDesc** 归属 012-Mesh；**.mesh** 为其序列化/磁盘格式。
- **注册时机**：引擎或应用初始化时，由持有 002-Object 的层（如 013-Resource 或主程序）向 002 注册 **MeshAssetDesc** 类型及 **.mesh** 扩展名，以便：
  - 反序列化时 002 能按类型分发并产出 MeshAssetDesc（opaque payload）；
  - 引用解析与版本迁移与 002 约定一致。
- **实现方**：012 不直接依赖 002；注册调用由 013 或上层在启动时执行，012 仅提供 **MeshAssetDesc** 类型定义与 **MeshDeserializer**（013 将 payload 交 012 Loader）。

## 一目录一资源（.mesh + .meshdata）

- 约定见 `docs/asset/01-asset-data-structure.md` §1.5：**一个目录存储一个资源**。
- Mesh 资源目录内容：
  - **.mesh**：MeshAssetDesc 的序列化（描述与元数据）；
  - **.meshdata**（可选）：顶点/索引等大块二进制数据；
  - 可选：导入前源文件（.obj、.fbx 等）。
- 013 通过 GUID→路径解析读入 .mesh（及 .meshdata），反序列化后交 012 CreateMesh/CreateFromPayload。
