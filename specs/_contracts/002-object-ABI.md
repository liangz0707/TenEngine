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
