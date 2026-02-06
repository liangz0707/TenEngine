# Implementation Plan: 002-Object 完整模块实现

**Branch**: `002-object-fullmodule-001` | **Date**: 2026-02-05 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `/specs/002-object-fullmodule-001/spec.md`

**Note**: 执行流程见 `.cursor/commands/speckit.plan.md`。

## Summary

实现 002-Object 模块的完整 ABI：反射（TypeDescriptor、TypeRegistry）、类型工厂（CreateInstance）、序列化（ISerializer、IVersionMigration、SerializedBuffer）、属性系统（PropertyBag、PropertyDescriptor）及 ObjectRef/GUID。仅依赖 001-Core 契约声明的 Alloc/Free、容器、字符串、日志等；构建通过源码引入 001-core，实现 `specs/_contracts/002-object-ABI.md` 中全部符号。

## 实现范围（TenEngine：实现全量 ABI 内容）

本 feature 实现 `specs/_contracts/002-object-ABI.md` 中**全部**已声明符号与能力；无新增或修改 ABI 条目，故「契约更新」小节为空。tasks/implement 以下方全量 ABI 表为准。

### 全量 ABI 内容（实现参考）

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 002-Object | te::object | — | 类型 | 类型标识 | te/object/TypeId.hpp | TypeId | `using TypeId = uint32_t;` 0 或 kInvalidTypeId 表示无效 |
| 002-Object | te::object | TypeDescriptor | 类型 | 类型描述 | te/object/TypeDescriptor.hpp | TypeDescriptor | struct: id, name, size, properties, propertyCount, methods, methodCount, baseTypeId |
| 002-Object | te::object | PropertyDescriptor | 类型 | 属性描述 | te/object/PropertyDescriptor.hpp | PropertyDescriptor | struct: name, valueTypeId, defaultValue |
| 002-Object | te::object | MethodDescriptor | 类型 | 方法描述（占位） | te/object/TypeId.hpp | MethodDescriptor | struct（本切片可为占位或最小集） |
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

## Technical Context

**Language/Version**: C++17  
**Primary Dependencies**: 001-Core（仅使用 `specs/_contracts/001-core-public-api.md` 声明的 Alloc/Free、Allocator、容器、字符串、日志等）  
**Storage**: N/A（序列化缓冲由调用方管理；类型元数据在进程内存中，无持久化到文件）  
**Testing**: CTest + 单元测试（TypeRegistry、Serializer 往返、PropertyBag）；与 001-Core 契约行为一致  
**Target Platform**: Windows / Linux / macOS（与 Core 一致）

**Project Type**: 引擎子模块（反射/序列化/属性）；源码位于 `Engine/TenEngine-002-object/`。  
**Performance Goals**: 类型查询与 CreateInstance 为热路径，需 O(1) 或 O(log n) 查找；序列化往返可接受线性时间。  
**Constraints**: 仅使用 001-Core 契约 API；类型元数据分配使用 Core Alloc/Free。  
**Scale/Scope**: 支持数百类型注册；单次序列化缓冲由调用方提供，无引擎侧上限约定。

## 依赖引入方式（TenEngine 构建规约）

规约见 `docs/engine-build-module-convention.md`。当前所有子模块构建均使用**源码**方式。

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-core | **源码** | 通过 TenEngineHelpers / `tenengine_resolve_my_dependencies("002-object")` 解析；同级 `Engine/TenEngine-001-core` 或 TENENGINE_001_CORE_DIR。 |

**说明**：构建根目录为仓库根；002-object 代码在 `Engine/TenEngine-002-object/`。依赖由 CMake 脚本统一解析，不引入上游 tests。

### 第三方依赖（本 feature 涉及模块所需）

002-Object 的 `specs/_contracts/002-object-public-api.md` 未声明任何第三方库（仅依赖 001-Core）。

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| 本 feature 无第三方依赖 | — | — | — |

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 原则 | 状态 | 说明 |
|------|------|------|
| **I. Modular** | 通过 | 002-Object 为独立模块，边界清晰（反射/序列化/属性/类型注册）。 |
| **II. Modern Graphics** | 不适用 | 本模块无图形 API。 |
| **III. Data-Driven** | 通过 | 类型与属性为数据驱动；序列化格式可版本化与迁移。 |
| **IV. Performance & Observability** | 通过 | 可依赖 Core 日志；热路径为类型查找与 CreateInstance。 |
| **V. Versioning & ABI** | 通过 | 公开 API 以 ABI 文件为准；版本化遵循 Constitution。 |
| **VI. Contract-First / Full ABI / No stubs** | 通过 | 实现全部 ABI 符号；依赖 001-Core 以源码引入；仅使用上游契约声明 API。 |

**Post–Phase 1**: 设计未引入未声明接口；数据模型与 contracts 与 ABI 一致。无违规。

## Project Structure

### Documentation (this feature)

```text
specs/002-object-fullmodule-001/
├── plan.md              # 本文件
├── research.md          # Phase 0
├── data-model.md        # Phase 1
├── quickstart.md        # Phase 1
├── contracts/           # Phase 1（引用 plan 与 _contracts）
├── checklists/
│   └── requirements.md
└── tasks.md             # Phase 2（/speckit.tasks 产出）
```

### Source Code (repository root)

本仓库为**单仓**布局（§2.5）；构建根目录为**仓库根**。002-Object 源码位于：

```text
Engine/TenEngine-002-object/
├── CMakeLists.txt
├── cmake/
│   ├── TenEngineHelpers.cmake
│   └── TenEngineModuleDependencies.cmake
├── include/
│   └── te/object/          # 与 ABI 头文件路径一致
│       ├── TypeId.hpp
│       ├── TypeDescriptor.hpp
│       ├── PropertyDescriptor.hpp
│       ├── SerializedBuffer.hpp
│       ├── ObjectRef.hpp
│       ├── Guid.hpp
│       ├── TypeRegistry.hpp
│       ├── VersionMigration.hpp
│       ├── Serializer.hpp
│       ├── PropertyBag.hpp
│       └── detail/         # 内部实现细节
├── src/
│   ├── TypeRegistry.cpp
│   ├── Serializer.cpp
│   ├── VersionMigration.cpp
│   ├── PropertyBag.cpp
│   └── CoreMemory.cpp      # 与 Core Alloc 对接等
└── tests/
    ├── unit/
    │   ├── TypeRegistry_test.cpp
    │   ├── Serializer_roundtrip_test.cpp
    │   ├── PropertyBag_test.cpp
    │   └── CreateInstance_test.cpp
    └── (optional integration)
```

**Structure Decision**: 采用单仓下 `Engine/TenEngine-002-object/` 的既有布局；公开头文件与 ABI 表路径一致（te/object/*.hpp），实现与测试仅 link te_object 与上游 te_core。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

本 feature **无**相对于现有 `specs/_contracts/002-object-ABI.md` 的新增或修改条目；实现以现有 ABI 全量为准。写回时本小节为空。

| 操作 | 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|------|--------|----------|------|----------|--------|------|------|
| （无） | — | — | — | — | — | — | — |

## Complexity Tracking

无 Constitution 违规需豁免；本表留空。
