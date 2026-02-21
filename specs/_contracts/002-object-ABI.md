# 002-Object 模块 ABI

- **契约**：[002-object-public-api.md](./002-object-public-api.md)（能力与类型描述）
- **本文件**：002-Object 对外 ABI 显式表。
- **更新来源**：002-object-fullversion-002 全量 ABI 写回（2026-01-29）。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 002-Object | te::object | — | 类型 | 类型标识 | te/object/TypeId.h | TypeId | `using TypeId = uint32_t;` 0 或 kInvalidTypeId 表示无效 |
| 002-Object | te::object | TypeDescriptor | 类型 | 类型描述 | te/object/TypeId.h | TypeDescriptor | struct: id, name, size, properties, propertyCount, baseTypeId, createInstance |
| 002-Object | te::object | PropertyDescriptor | 类型 | 属性描述 | te/object/TypeId.h | PropertyDescriptor | struct: name, valueTypeId, offset, size, defaultValue |
| 002-Object | te::object | TypeRegistry | 类 | 类型注册 | te/object/TypeRegistry.h | RegisterType | `static bool RegisterType(TypeDescriptor const& desc);` 重复 TypeId 拒绝；线程安全 |
| 002-Object | te::object | TypeRegistry | 类 | 类型查询 | te/object/TypeRegistry.h | GetTypeByName | `static TypeDescriptor const* GetTypeByName(char const* name);` 未找到返回 nullptr；线程安全 |
| 002-Object | te::object | TypeRegistry | 类 | 类型查询 | te/object/TypeRegistry.h | GetTypeById | `static TypeDescriptor const* GetTypeById(TypeId id);` 未找到返回 nullptr；线程安全 |
| 002-Object | te::object | TypeRegistry | 类 | 类型工厂 | te/object/TypeRegistry.h | CreateInstance | `static void* CreateInstance(TypeId id);` `static void* CreateInstance(char const* typeName);` 使用 Core Alloc 分配；失败返回 nullptr |
| 002-Object | te::object | TypeRegistry | 类 | 类型检查 | te/object/TypeRegistry.h | IsTypeRegistered | `static bool IsTypeRegistered(TypeId id);` `static bool IsTypeRegistered(char const* name);` |
| 002-Object | te::object | TypeRegistry | 类 | 类型枚举 | te/object/TypeRegistry.h | EnumerateTypes | `static void EnumerateTypes(void (*callback)(TypeDescriptor const*, void*), void* userData);` |
| 002-Object | te::object | — | 类型 | 序列化缓冲 | te/object/Serializer.h | SerializedBuffer | struct: void* data, size_t size, size_t capacity；调用方管理；IsValid(), Clear() |
| 002-Object | te::object | — | 枚举 | 序列化格式 | te/object/Serializer.h | SerializationFormat | `enum class SerializationFormat { Binary, JSON, XML };` |
| 002-Object | te::object | — | 类型 | 对象引用 | te/object/Guid.h | ObjectRef | struct: GUID guid；跨资源引用仅读写 GUID；IsNull() |
| 002-Object | te::object | — | 类型 | 全局唯一标识 | te/object/Guid.h | GUID | struct: uint8_t data[16]；Generate(), FromString(), ToString(), operator==/!=/<, IsNull() |
| 002-Object | te::object | IVersionMigration | 接口 | 版本迁移 | te/object/Serializer.h | Migrate | `virtual bool Migrate(SerializedBuffer& buf, uint32_t fromVersion, uint32_t toVersion) = 0;` |
| 002-Object | te::object | ISerializer | 接口 | 序列化 | te/object/Serializer.h | Serialize | `virtual bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) = 0;` `virtual bool Serialize(SerializedBuffer& out, void const* obj, char const* typeName) = 0;` |
| 002-Object | te::object | ISerializer | 接口 | 反序列化 | te/object/Serializer.h | Deserialize | `virtual bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) = 0;` `virtual bool Deserialize(SerializedBuffer const& buf, void* obj, char const* typeName) = 0;` |
| 002-Object | te::object | ISerializer | 接口 | 当前版本 | te/object/Serializer.h | GetCurrentVersion | `virtual uint32_t GetCurrentVersion() const = 0;` |
| 002-Object | te::object | ISerializer | 接口 | 设置版本迁移 | te/object/Serializer.h | SetVersionMigration | `virtual void SetVersionMigration(IVersionMigration* migration) = 0;` |
| 002-Object | te::object | ISerializer | 接口 | 获取格式 | te/object/Serializer.h | GetFormat | `virtual SerializationFormat GetFormat() const = 0;` |
| 002-Object | te::object | — | 函数 | 创建二进制序列化器 | te/object/Serializer.h | CreateBinarySerializer | `ISerializer* CreateBinarySerializer();` |
| 002-Object | te::object | — | 函数 | 创建 JSON 序列化器 | te/object/Serializer.h | CreateJSONSerializer | `ISerializer* CreateJSONSerializer();` |
| 002-Object | te::object | — | 函数 | 创建 XML 序列化器 | te/object/Serializer.h | CreateXMLSerializer | `ISerializer* CreateXMLSerializer();` |
| 002-Object | te::object | — | 函数 | 序列化到文件 | te/object/Serializer.h | SerializeToFile | `bool SerializeToFile(char const* path, void const* obj, TypeId typeId, SerializationFormat format = Binary);` 使用 Core 文件 I/O |
| 002-Object | te::object | — | 函数 | 从文件反序列化 | te/object/Serializer.h | DeserializeFromFile | `bool DeserializeFromFile(char const* path, void* obj, TypeId typeId, SerializationFormat format = Binary);` `bool DeserializeFromFile(char const* path, void* obj, char const* typeName, SerializationFormat format = Binary);` 使用 Core 文件 I/O |
| 002-Object | te::object | IPropertyBag | 接口 | 属性容器接口 | te/object/PropertyBag.h | GetProperty | `virtual bool GetProperty(void* outValue, char const* name) const = 0;` `virtual bool GetProperty(void* outValue, char const* name, TypeId typeId) const = 0;` |
| 002-Object | te::object | IPropertyBag | 接口 | 属性设置 | te/object/PropertyBag.h | SetProperty | `virtual bool SetProperty(void const* value, char const* name) = 0;` `virtual bool SetProperty(void const* value, char const* name, TypeId typeId) = 0;` |
| 002-Object | te::object | IPropertyBag | 接口 | 查找属性 | te/object/PropertyBag.h | FindProperty | `virtual PropertyDescriptor const* FindProperty(char const* name) const = 0;` |
| 002-Object | te::object | IPropertyBag | 接口 | 属性数量 | te/object/PropertyBag.h | GetPropertyCount | `virtual size_t GetPropertyCount() const = 0;` |
| 002-Object | te::object | IPropertyBag | 接口 | 按索引获取属性 | te/object/PropertyBag.h | GetProperty | `virtual PropertyDescriptor const* GetProperty(size_t index) const = 0;` |
| 002-Object | te::object | PropertyBag | 类 | 属性容器实现 | te/object/PropertyBag.h | PropertyBag | `PropertyBag(void* instance, TypeDescriptor const* typeDesc);` 基于 TypeDescriptor 的实现 |

## 调用流程（resource-serialization）

1. 引擎启动 → 各模块调用 `RegisterType` 注册描述类型（MaterialAssetDesc、LevelAssetDesc 等；ModelAssetDesc 由 029 注册，TextureAssetDesc 由 028 注册）
2. Load 流程 → 调用方 `GetTypeByName(扩展名/类型名)` 取得 TypeDescriptor → `Deserialize(buf, obj, typeId)`
3. Save 流程 → 调用方 `GetTypeById(typeId)` 取得类型 → `Serialize(obj, buf, typeId)`
4. 版本迁移：反序列化前若读出版本低于当前支持版本，先 `IVersionMigration::Migrate` 再 `Deserialize`

## 数据约定

- **formatVersion**：由各描述类型所属模块与本契约约定；序列化时写出，反序列化前可 Migrate
- **跨资源引用**：仅读写 16 字节 GUID，不存指针或路径
- **上游调用**：类型元数据分配使用 001-Core `Alloc`/`Free`

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 002-Object 契约；按模块规格与 resource-serialization 约定 ABI 与 TODO |
| 2026-01-29 | 002-object-fullversion-002 全量 ABI 写回：TypeDescriptor、TypeRegistry、ISerializer、IVersionMigration、PropertyBag 等；数据相关 TODO 已实现 |
| 2026-02-06 | 完全重新设计：新增 JSON 和 XML 序列化器；增强 TypeRegistry（IsTypeRegistered、EnumerateTypes）；增强 GUID（Generate、FromString、ToString）；新增文件序列化便捷函数（SerializeToFile、DeserializeFromFile）；PropertyBag 增强（类型检查、按索引访问）；头文件扩展名从 .hpp 改为 .h |
| 2026-02-22 | Verified alignment with code: TypeId = std::uint32_t; kInvalidTypeId = 0; TypeDescriptor/PropertyDescriptor structures match; TypeRegistry static methods match; GUID methods match; ISerializer includes GetFormat(); IVersionMigration in Serializer.h; PropertyBag constructor and methods match; all serializer factory functions match |
