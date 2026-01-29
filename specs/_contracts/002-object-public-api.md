# 契约：002-Object 模块对外 API

## 适用模块

- **实现方**：**002-Object**（T0 对象模型与元数据）
- **对应规格**：`docs/module-specs/002-object.md`
- **依赖**：001-Core（001-core-public-api）

## 消费者（T0 下游）

- 004-Scene, 005-Entity, 007-Subsystems, 013-Resource, 015-Animation；020-Pipeline、024-Editor 等（通过 Scene/Entity/Resource 间接）。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| TypeId / TypeDescriptor | 类型标识与描述；属性/方法列表、基类链 | 注册后直至卸载 |
| SerializedBuffer / ObjectRef | 序列化字节流、对象引用 | 由调用方管理缓冲；ObjectRef 与资源 GUID 对应 |
| GUID | 全局唯一标识，用于资源引用与解析 | 与资源/对象绑定 |
| PropertyBag / PropertyDescriptor | 属性描述、元数据、默认值、范围/枚举 | 与类型或实例绑定 |
| 类型工厂 | CreateInstance、按 TypeId 创建 | 按调用约定 |

## 能力列表（提供方保证）

1. **反射**：类型注册、类型信息查询、属性/方法枚举、基类链、GetTypeByName/ById。
2. **序列化**：序列化器抽象、二进制/文本格式、版本迁移、对象引用与 GUID 解析；往返等价性可验证。
3. **属性系统**：属性描述、元数据、默认值、范围/枚举约束；与反射和序列化联动。
4. **类型注册**：注册表、按模块注册、类型工厂、生命周期；与 Core 模块加载协调。

## API 雏形（简化声明）

**本切片（002-object-minimal）暴露**：类型注册 + 简单序列化。仅使用 001-Core 契约已声明的类型（分配器、容器、字符串等）。

### 类型与句柄（本切片）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| TypeId | 类型唯一标识（如 uint32_t 或 opaque 句柄） | 注册后直至卸载 |
| TypeDescriptor | 类型描述：TypeId、名称、大小等（本切片不含属性/方法列表） | 与类型绑定 |
| SerializedBuffer | 序列化字节流缓冲；由调用方分配与释放 | 调用方管理 |

### 类型注册

```cpp
namespace te::object {

using TypeId = uint32_t;   // 或 opaque 句柄

struct TypeDescriptor {
    TypeId       id;
    char const*  name;     // 或使用 Core 契约中的 String 类型
    size_t       size;    // 实例大小（本切片最小所需）
    // 本切片不包含：属性列表、基类链、方法列表
};

class TypeRegistry {
public:
    // 注册类型；返回是否成功（如重复 id/name 可规定为覆盖或失败）
    static bool RegisterType(TypeDescriptor const& desc);

    // 按名/ID 查询；未找到返回 nullptr
    static TypeDescriptor const* GetTypeByName(char const* name);
    static TypeDescriptor const* GetTypeById(TypeId id);
};

}
```

### 简单序列化（本切片）

```cpp
namespace te::object {

// 调用方提供的缓冲；生命周期由调用方管理
struct SerializedBuffer {
    void*  data;
    size_t size;
    size_t capacity;   // 可写容量（用于序列化时扩展）
};

// 序列化器抽象：将已注册类型实例写入缓冲 / 从缓冲读回
class ISerializer {
public:
    virtual ~ISerializer() = default;
    // 将 obj（类型已注册）序列化到 out；返回是否成功
    virtual bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) = 0;
    // 从 buf 反序列化为 typeId 类型；obj 由调用方提供内存；返回是否成功
    virtual bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) = 0;
};

// 默认实现（如最小二进制格式）可由工厂或模块初始化返回
// std::unique_ptr<ISerializer> CreateDefaultSerializer();
}
```

### 调用顺序与约束（本切片）

- 须在 Core 初始化之后使用；类型注册可在启动时或按模块加载时进行。
- SerializedBuffer 的 data 可由调用方使用 Core 分配器（Alloc/Free）分配；本切片不规定缓冲格式的版本迁移与 GUID 解析。

## 调用顺序与约束

- 须在 Core 初始化之后使用；类型注册可在启动时或按模块加载时进行。
- 序列化/反序列化与资源引用（GUID）解析须与 013-Resource 等消费者约定引用格式。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 002-Object 模块规格与依赖表新增契约 |
| 2026-01-29 | API 雏形由 plan 002-object-minimal 同步（类型注册 + 简单序列化） |
