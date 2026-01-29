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

（由 plan 产出后写回，或先手写最小声明如 TypeRegistry::RegisterType、序列化接口。）

## 调用顺序与约束

- 须在 Core 初始化之后使用；类型注册可在启动时或按模块加载时进行。
- 序列化/反序列化与资源引用（GUID）解析须与 013-Resource 等消费者约定引用格式。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 002-Object 模块规格与依赖表新增契约 |
