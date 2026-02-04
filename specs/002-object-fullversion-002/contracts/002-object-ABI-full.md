# 002-Object 全量 ABI（实现参考）

> **用途**：tasks 与 implement 阶段**必须基于本全量 ABI 内容**进行实现。plan.md「契约更新」小节仅保存相对现有 `specs/_contracts/002-object-ABI.md` 的新增/修改部分。
>
> **来源**：原始 ABI + `specs/_contracts/002-object-ABI.md` 数据相关 TODO + `specs/_contracts/002-object-public-api.md` 完整功能集。

## 全量 ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 002-Object | te::object | — | 类型 | 类型标识 | te/object/TypeId.hpp | TypeId | `using TypeId = uint32_t;` 0 或 kInvalidTypeId 表示无效 |
| 002-Object | te::object | TypeDescriptor | 类型 | 类型描述 | te/object/TypeDescriptor.hpp | TypeDescriptor | struct: id, name, size, properties, propertyCount, methods, methodCount, baseTypeId |
| 002-Object | te::object | PropertyDescriptor | 类型 | 属性描述 | te/object/PropertyDescriptor.hpp | PropertyDescriptor | struct: name, valueTypeId, defaultValue |
| 002-Object | te::object | MethodDescriptor | 类型 | 方法描述（占位） | te/object/TypeDescriptor.hpp | MethodDescriptor | struct（本切片可为占位或最小集） |
| 002-Object | te::object | TypeRegistry | 类 | 类型注册 | te/object/TypeRegistry.hpp | RegisterType | `static bool RegisterType(TypeDescriptor const& desc);` 重复 TypeId 拒绝 |
| 002-Object | te::object | TypeRegistry | 类 | 类型查询 | te/object/TypeRegistry.hpp | GetTypeByName | `static TypeDescriptor const* GetTypeByName(char const* name);` 未找到返回 nullptr |
| 002-Object | te::object | TypeRegistry | 类 | 类型查询 | te/object/TypeRegistry.hpp | GetTypeById | `static TypeDescriptor const* GetTypeById(TypeId id);` 未找到返回 nullptr |
| 002-Object | te::object | TypeRegistry | 类 | 类型工厂 | te/object/TypeRegistry.hpp | CreateInstance | `static void* CreateInstance(TypeId id);` 使用 Core Alloc 分配；失败返回 nullptr |
| 002-Object | te::object | — | 类型 | 序列化缓冲 | te/object/SerializedBuffer.hpp | SerializedBuffer | struct: void* data, size_t size, size_t capacity；调用方管理 |
| 002-Object | te::object | — | 类型 | 对象引用 | te/object/ObjectRef.hpp | ObjectRef | struct: uint8_t guid[16]；跨资源引用仅读写 GUID |
| 002-Object | te::object | — | 类型 | 全局唯一标识 | te/object/Guid.hpp | GUID | struct: uint8_t data[16] |
| 002-Object | te::object | IVersionMigration | 接口 | 版本迁移 | te/object/VersionMigration.hpp | Migrate | `virtual bool Migrate(SerializedBuffer& buf, uint32_t fromVersion, uint32_t toVersion) = 0;` |
| 002-Object | te::object | ISerializer | 接口 | 序列化 | te/object/Serializer.hpp | Serialize | `virtual bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) = 0;` |
| 002-Object | te::object | ISerializer | 接口 | 反序列化 | te/object/Serializer.hpp | Deserialize | `virtual bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) = 0;` |
| 002-Object | te::object | ISerializer | 接口 | 当前版本 | te/object/Serializer.hpp | GetCurrentVersion | `virtual uint32_t GetCurrentVersion() const = 0;` |
| 002-Object | te::object | ISerializer | 接口 | 设置版本迁移 | te/object/Serializer.hpp | SetVersionMigration | `virtual void SetVersionMigration(IVersionMigration* migration) = 0;` |
| 002-Object | te::object | PropertyBag | 接口 | 属性读写 | te/object/PropertyBag.hpp | GetProperty | `virtual bool GetProperty(void* outValue, char const* name) const = 0;` |
| 002-Object | te::object | PropertyBag | 接口 | 属性读写 | te/object/PropertyBag.hpp | SetProperty | `virtual bool SetProperty(void const* value, char const* name) = 0;` |
| 002-Object | te::object | PropertyBag | 接口 | 查找属性 | te/object/PropertyBag.hpp | FindProperty | `virtual PropertyDescriptor const* FindProperty(char const* name) const = 0;` |

## 调用流程（resource-serialization）

1. 引擎启动 → 各模块调用 `RegisterType` 注册描述类型（MaterialAssetDesc、ModelAssetDesc、LevelAssetDesc 等）
2. Load 流程 → 调用方 `GetTypeByName(扩展名/类型名)` 取得 TypeDescriptor → `Deserialize(buf, obj, typeId)`
3. Save 流程 → 调用方 `GetTypeById(typeId)` 取得类型 → `Serialize(obj, buf, typeId)`
4. 版本迁移：反序列化前若读出版本低于当前支持版本，先 `IVersionMigration::Migrate` 再 `Deserialize`

## 数据约定

- **formatVersion**：由各描述类型所属模块与本契约约定；序列化时写出，反序列化前可 Migrate
- **跨资源引用**：仅读写 16 字节 GUID，不存指针或路径
- **上游调用**：类型元数据分配使用 001-Core `Alloc`/`Free`
