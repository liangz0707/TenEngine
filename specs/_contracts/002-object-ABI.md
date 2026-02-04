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

（依据 [docs/assets/resource-serialization.md](../../docs/assets/resource-serialization.md)；本模块上游：001-Core。）

### 注册

| 需提供 | 需调用上游 |
|--------|------------|
| [ ] `RegisterType<T>(name)`：支持任意可序列化类型在模块初始化时注册 | 001：`Alloc`/`Free`（类型元数据分配） |
| [ ] `GetTypeInfo(typeId) → ITypeInfo const*`：按 TypeId 查询类型元数据 | — |
| [ ] `GetTypeByName(name) → TypeId`：按类型名查询 TypeId | — |

*调用时机：须在 001-Core 初始化之后、任何序列化/反序列化之前完成。*

### 序列化

| 需提供 | 需调用上游 |
|--------|------------|
| [ ] `Serialize(obj, buf, typeId)`：按 TypeId 将对象写入 buf | — |
| [ ] 跨资源引用约定：**仅读写 16 字节 GUID**，不存指针或路径 | — |

### 反序列化

| 需提供 | 需调用上游 |
|--------|------------|
| [ ] `Deserialize(buf, obj, typeId)`：按 TypeId 从 buf 还原对象 | — |
| [ ] `IVersionMigration::Migrate(buf, fromVersion, toVersion)`：版本迁移；反序列化前若读出版本低于当前支持版本，先 Migrate 再 Deserialize | — |

### 数据

- [ ] **ITypeInfo**：类型元数据（名称、大小、序列化信息）
- [ ] **formatVersion** 约定：由各描述类型所属模块与本契约约定

### 调用流程

1. 引擎启动 → 各模块调用 `RegisterType` 注册描述类型（MaterialAssetDesc、ModelAssetDesc、LevelAssetDesc 等）
2. Load 流程 → 调用方 `GetTypeByName(扩展名/类型名)` 取得 TypeId → `Deserialize(buf, obj, typeId)`
3. Save 流程 → 调用方 `GetTypeInfo(typeId)` 取得类型 → `Serialize(obj, buf, typeId)`
