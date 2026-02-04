# 002-Object 模块 ABI

- **契约**：[002-object-public-api.md](./002-object-public-api.md)（能力与类型描述）
- **本文件**：002-Object 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 002-Object | TenEngine::object | — | 自由函数/模板 | 类型注册 | TenEngine/object/TypeRegistry.h | RegisterType | `template<typename T> void RegisterType(char const* name, ...);` 供 Entity 等模块注册组件类型 |
| 002-Object | TenEngine::object | — | 类型/接口 | 类型信息 | TenEngine/object/TypeRegistry.h | ITypeInfo, GetTypeInfo | `ITypeInfo const* GetTypeInfo(TypeId id);` 返回类型名、大小、序列化等信息；Entity 注册 Component 时可选使用 |

*来源：用户故事 US-entity-001（场景通过 ECS 组织，程序员可快速定义与扩展 Component 类型）。*

---

## 数据相关 TODO

（依据 [docs/assets/resource-serialization.md](../../docs/assets/resource-serialization.md)、各 data-model 与 013-resource-data-model。）

- [ ] **TypeRegistry**：支持 011-Material、013-Resource、004-Scene 在启动/模块加载时注册 **MaterialAssetDesc**、**ModelAssetDesc**、**TextureDescriptor**、**LevelAssetDesc**、**SceneNodeDesc**；须在 Core 初始化之后、任何对该类型的序列化/反序列化之前完成。
- [ ] **Serialize/Deserialize**：按 TypeId 序列化/反序列化上述描述类型；跨资源引用仅读写 **16 字节 GUID**，不存指针或路径。
- [ ] **IVersionMigration**：提供 `Migrate(buf, fromVersion, toVersion)`；反序列化前若读出版本低于当前支持版本，先 Migrate 再反序列化；各描述类型 **formatVersion** 约定由对应模块与本文约定。
- [ ] **GetTypeByName**：013 在 Load 时根据资源类型或扩展名得到描述类型名，通过 GetTypeByName 取 TypeId，再用于 Deserialize(buf, obj, typeId)；Save/Import 时用同一 TypeId 做 Serialize。
