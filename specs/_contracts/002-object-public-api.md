# 契约：002-Object 模块对外 API

## 适用模块

- **实现方**：002-Object（L0；反射、序列化、属性系统、类型注册、GUID）
- **对应规格**：`docs/module-specs/002-object.md`
- **依赖**：001-Core

## 消费者

- 004-Scene、005-Entity、007-Subsystems、013-Resource、015-Animation

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| TypeId | 类型唯一标识（uint32_t 或 opaque 句柄） | 注册后直至卸载 |
| TypeDescriptor | 类型描述：TypeId、名称、大小、属性/方法列表、基类链 | 与类型绑定 |
| SerializedBuffer | 序列化字节流缓冲；data、size、capacity；由调用方分配与释放 | 调用方管理 |
| ObjectRef | 对象引用；与资源 GUID 对应 | 与对象/资源绑定 |
| GUID | 全局唯一标识，用于资源引用与解析 | 与资源/对象绑定 |
| PropertyBag / PropertyDescriptor | 属性描述、元数据、默认值、范围/枚举 | 与类型或实例绑定 |
| 类型工厂 | CreateInstance、按 TypeId 创建 | 按调用约定 |
| AssetDesc 等 | MaterialAssetDesc、LevelAssetDesc、SceneNodeDesc、MeshAssetDesc 等由对应模块拥有并向 002 注册；ModelAssetDesc、IModelResource→029-World，TextureAssetDesc→028-Texture；013/029/028/011/012 反序列化时通过 002 得到描述 | 注册后直至卸载 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 反射 | 类型注册、类型信息查询、属性/方法枚举、基类链；TypeRegistry::RegisterType、GetTypeByName、GetTypeById |
| 2 | 序列化 | 序列化器抽象、二进制/文本格式、版本迁移、对象引用与 GUID 解析；ISerializer::Serialize、Deserialize；往返等价可验证 |
| 3 | 属性系统 | 属性描述、元数据、默认值、范围/枚举约束；与反射和序列化联动 |
| 4 | 类型注册 | 注册表、按模块注册、类型工厂、生命周期；与 Core 模块加载协调 |

命名空间 `te::object`。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core 初始化之后使用。资源描述类型（AssetDesc）由本模块定义或注册，013/004/011/012 反序列化时通过 002 取得描述。

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计。）

- [ ] **AssetDesc 注册**：提供 *AssetDesc 类型注册（ShaderAssetDesc、MaterialAssetDesc、MeshAssetDesc、LevelAssetDesc、SceneNodeDesc、AudioAssetDesc 等）；ModelAssetDesc 由 029 注册，TextureAssetDesc 由 028 注册；各资源模块向 002 注册。
- [ ] **GUID 引用解析**：描述文件中 GUID 引用由 002 在反序列化时按类型与引用解析约定处理；与 ObjectRef、VersionMigration 一致。
- [ ] **反序列化**：资源描述与原始数据由 013 读盘后交 002 统一反序列化得到 *AssetDesc；与 013、各模块 Create*/Loader 对接。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 002-Object 契约；按模块规格与 resource-serialization 约定 ABI 与 TODO |
| 2026-01-29 | 002-object-fullversion-002 全量 ABI 写回：TypeDescriptor、TypeRegistry、ISerializer、IVersionMigration、PropertyBag 等；数据相关 TODO 已实现 |
| 2026-02-05 | 统一目录；能力列表用表格；去除代码示例与 ABI 引用 |
