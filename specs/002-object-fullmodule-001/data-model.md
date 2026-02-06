# Data Model: 002-Object

**Feature**: 002-object-fullmodule-001 | **Phase 1**

## 实体与类型（与 ABI 一致）

### TypeId

- **语义**: 类型唯一标识；`uint32_t`；0 或 `kInvalidTypeId` 表示无效。
- **生命周期**: 注册后直至进程/模块卸载。
- **验证**: 注册时不允许重复 TypeId；CreateInstance 仅接受已注册 id。

### TypeDescriptor

- **字段**: id (TypeId), name (char const* 或等价), size (size_t), properties (PropertyDescriptor const*), propertyCount, methods (MethodDescriptor const*), methodCount, baseTypeId (TypeId)。
- **关系**: 描述一个类型；properties/methods 为只读数组，由注册方或模块管理；baseTypeId 用于基类链（可选，0 表示无基类）。
- **生命周期**: 与类型注册绑定；不得在未注销时释放其引用的属性/方法数组。

### PropertyDescriptor

- **字段**: name (char const*), valueTypeId (TypeId), defaultValue (可选或 opaque)。
- **关系**: 属于某 TypeDescriptor；valueTypeId 指向另一已注册类型（用于嵌套或引用类型）。
- **验证**: name 非空；valueTypeId 可为 0 表示“无默认”或基础类型由实现约定。

### MethodDescriptor（占位）

- **字段**: 最小集（如 name；或空 struct）以满足 ABI。
- **关系**: 属于某 TypeDescriptor；本 feature 不实现运行时方法调用。

### SerializedBuffer

- **字段**: void* data, size_t size, size_t capacity。
- **语义**: 调用方分配与释放；序列化/反序列化仅读写 data[0..size)，必要时扩展由实现与调用方约定（如 Grow 由调用方或接口扩展）。
- **生命周期**: 调用方管理；实现不拥有 buffer。

### ObjectRef

- **字段**: uint8_t guid[16]。
- **语义**: 跨资源引用；仅读写 16 字节 GUID；解析由 013/004 等负责。
- **验证**: 无；为不透明句柄。

### GUID

- **字段**: uint8_t data[16]（或等价）。
- **语义**: 全局唯一标识；与 ObjectRef 一致表示时可为同一布局。
- **生成**: 非 002 职责；可由 Core 或 013 提供。

## 接口（行为约束）

### TypeRegistry

- **RegisterType(desc)**: 注册 TypeDescriptor；重复 id 返回 false；成功返回 true。
- **GetTypeByName(name)**: 返回非空 TypeDescriptor const* 或 nullptr。
- **GetTypeById(id)**: 同上。
- **CreateInstance(id)**: 使用 Core Alloc 分配 TypeDescriptor::size 字节；失败返回 nullptr；不保证调用 C++ 构造函数（由实现或调用方约定）。

### ISerializer

- **Serialize(out, obj, typeId)**: 将 obj 按 typeId 对应类型序列化到 out；返回 true 表示成功。
- **Deserialize(buf, obj, typeId)**: 从 buf 反序列化到 obj；返回 true 表示成功。
- **GetCurrentVersion()**: 返回当前格式版本号（用于 Migrate 判断）。
- **SetVersionMigration(migration)**: 设置 IVersionMigration*；反序列化前若版本低于当前则先 Migrate。

### IVersionMigration

- **Migrate(buf, fromVersion, toVersion)**: 原地将 buf 从 fromVersion 迁移到 toVersion；返回 true 表示成功。

### PropertyBag

- **GetProperty(outValue, name)**: 按 name 写入选定属性到 outValue；返回 true 表示成功。
- **SetProperty(value, name)**: 按 name 设置属性；返回 true 表示成功。
- **FindProperty(name)**: 返回 PropertyDescriptor const* 或 nullptr。

## 状态与生命周期

- **类型注册表**: 进程内单例或按模块作用域；Init 后可用，Shutdown 前有效。
- **实例**: 由 CreateInstance 或调用方分配；释放由调用方或统一用 Core Free。
- **SerializedBuffer**: 始终调用方管理；实现不持有 data 所有权。
- **版本迁移**: 反序列化前若读出版本号 < GetCurrentVersion()，先调用 Migrate 再 Deserialize；Migrate 失败则 Deserialize 不调用或返回 false。

## 与 001-Core 的边界

- 分配/释放：仅使用 001-Core 契约中的 Alloc/Free 或 GetDefaultAllocator()->Alloc/Free。
- 容器/字符串：若实现需要动态容器或字符串，使用 001-core-public-api 声明的类型（如 Array、Map、String）。
- 日志：可选使用 Core Log/CheckError 等，用于错误与断言。
