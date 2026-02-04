# Implementation Plan: 002-Object Full Module Content

**Branch**: `002-object-fullversion-002` | **Date**: 2026-01-29 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/002-object-fullversion-002/spec.md`

**规约与契约**：完整模块规约见 `docs/module-specs/002-object.md`，对外 API 契约见 `specs/_contracts/002-object-public-api.md`。本计划实现完整模块内容，仅暴露契约已声明的类型与 API。**全量 ABI 内容**见 `contracts/002-object-ABI-full.md`，tasks 与 implement 阶段必须基于该全量内容实现。

## Summary

本 feature 实现 002-Object 的**完整模块内容**：(1) **反射** 类型注册、类型信息查询、属性/方法枚举、基类链、GetTypeByName/ById；(2) **序列化** 序列化器抽象、二进制与文本格式、版本迁移、对象引用与 GUID 解析；(3) **属性系统** PropertyDescriptor、PropertyBag、元数据、默认值、范围/枚举约束；(4) **类型注册** 注册表、类型工厂 CreateInstance、与 Core 模块加载协调的生命周期。技术栈 **C++17、CMake**；仅使用上游契约 `specs/_contracts/001-core-public-api.md` 已声明的类型与 API；**特别实现 `specs/_contracts/002-object-ABI.md` 数据相关 TODO 中的全部条目**。

## 实现范围（TenEngine：实现全量 ABI 内容）

> **ABI 生成与保存模式**：
> - **全量 ABI 内容**：已生成至 `contracts/002-object-ABI-full.md`，包含原始 ABI、`002-object-ABI.md` 数据相关 TODO 条目、以及 public-api 完整功能集。
> - **实现基于全量**：tasks 和 implement 阶段**必须基于** `contracts/002-object-ABI-full.md` 进行实现，不得仅实现「契约更新」小节的变化部分。
> - **TODO 必实现**：`specs/_contracts/002-object-ABI.md` 数据相关 TODO 中的 RegisterType、GetTypeInfo、GetTypeByName、Serialize、Deserialize、IVersionMigration::Migrate、formatVersion 约定、调用流程，**本次必须全部实现**。

**全量 ABI 符号**（详见 `contracts/002-object-ABI-full.md`）：

- **类型**：TypeId、TypeDescriptor、PropertyDescriptor、MethodDescriptor、SerializedBuffer、ObjectRef、GUID
- **TypeRegistry**：RegisterType、GetTypeByName、GetTypeById、CreateInstance
- **IVersionMigration**：Migrate
- **ISerializer**：Serialize、Deserialize、GetCurrentVersion、SetVersionMigration
- **PropertyBag**：GetProperty、SetProperty、FindProperty

## Technical Context

**Language/Version**: C++17  
**Build**: CMake（单一日志化构建；可复现）  
**Primary Dependencies**: 001-Core（仅使用 `specs/_contracts/001-core-public-api.md` 已声明的 Alloc/Free、容器、字符串、日志等）  
**Storage**: N/A（序列化缓冲由调用方管理；本模块不直接持久化到文件）  
**Testing**: 单元测试（反射、序列化往返、属性系统、类型工厂）；CTest  
**Target Platform**: Windows / Linux / macOS（与 Core 一致）  
**Project Type**: 静态库，由主工程或下游模块链接  
**Performance Goals**: 类型查询、序列化/反序列化、属性访问在合理规模下可接受  
**Constraints**: 仅暴露契约已声明的类型与 API；须在 Core 初始化之后使用；重复 TypeId 拒绝；跨资源引用仅读写 16 字节 GUID  
**Scale/Scope**: 完整能力列表：反射、序列化（含版本迁移与 GUID）、属性系统、类型注册

## 依赖引入方式（TenEngine 构建规约）

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-core | **源码** | 通过 TenEngineHelpers / tenengine_resolve_my_dependencies 引入上游源码构建（同级 worktree 或 TENENGINE_001_CORE_DIR）。 |

### 第三方依赖（本 feature 涉及模块所需）

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| 本 feature 无第三方依赖 | — | — | 002-object-public-api 未声明第三方库 |

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| **VI. Module Boundaries & Contract-First** | PASS | 仅暴露契约声明的类型与 API；全量 ABI 见 contracts/002-object-ABI-full.md |
| **Technology: C++17, CMake** | PASS | 语言 C++17，构建 CMake。 |
| **Implementation uses only upstream contracts** | PASS | 仅使用 001-core-public-api 已声明的 Alloc/Free 等。 |
| **Public API versioned; breaking changes MAJOR** | PASS | 对外 API 见契约；破坏性变更递增 MAJOR。 |
| **Testing** | PASS | 单元测试覆盖反射、序列化、属性系统、类型工厂。 |
| **Full ABI implementation** | PASS | 实现 002-object-ABI 全部符号及数据相关 TODO。 |

## Project Structure

### Documentation (this feature)

```text
specs/002-object-fullversion-002/
├── plan.md              # 本文件
├── research.md          # Phase 0
├── data-model.md        # Phase 1
├── quickstart.md        # Phase 1
├── contracts/           # Phase 1
│   └── 002-object-ABI-full.md   # 全量 ABI（实现参考）
└── tasks.md             # Phase 2（/speckit.tasks 产出）
```

### Source Code (repository root)

```text
modules/object/
├── CMakeLists.txt
├── include/te/object/
│   ├── TypeId.hpp
│   ├── TypeDescriptor.hpp
│   ├── TypeRegistry.hpp
│   ├── SerializedBuffer.hpp
│   ├── ObjectRef.hpp
│   ├── Guid.hpp
│   ├── Serializer.hpp
│   ├── VersionMigration.hpp
│   ├── PropertyDescriptor.hpp
│   ├── PropertyBag.hpp
│   └── detail/CoreMemory.hpp
├── src/
│   ├── CoreMemory.cpp
│   ├── TypeRegistry.cpp
│   ├── Serializer.cpp
│   ├── VersionMigration.cpp
│   └── PropertyBag.cpp
└── tests/unit/
    ├── TypeRegistry_test.cpp
    ├── Serializer_roundtrip_test.cpp
    ├── PropertyBag_test.cpp
    └── CreateInstance_test.cpp
```

**Structure Decision**: 单库、头文件与实现分离；对外仅通过 `include/te/object/` 暴露契约声明的类型与 API。依赖 001-Core 通过 CMake 链接并只引用 Core 契约中的头文件（Alloc/Free 等）。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

> **注意**：本小节**只保存相对于现有** `specs/_contracts/002-object-ABI.md` **的新增和修改**条目，用于写回契约。
>
> **实现时使用全量内容**：tasks 和 implement 阶段应基于 `contracts/002-object-ABI-full.md` 全量 ABI 进行实现。

| 操作 | 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|------|--------|----------|------|----------|--------|------|------|
| 新增 | 002-Object | te::object | TypeRegistry | 按名查询 TypeId | te/object/TypeRegistry.hpp | GetTypeByName | `static TypeDescriptor const* GetTypeByName(char const* name);` 未找到返回 nullptr |
| 新增 | 002-Object | te::object | TypeRegistry | 按 ID 查询 | te/object/TypeRegistry.hpp | GetTypeById | `static TypeDescriptor const* GetTypeById(TypeId id);` 等价 ITypeInfo |
| 新增 | 002-Object | te::object | TypeRegistry | 类型工厂 | te/object/TypeRegistry.hpp | CreateInstance | `static void* CreateInstance(TypeId id);` 使用 Core Alloc |
| 新增 | 002-Object | te::object | ISerializer | 序列化 | te/object/Serializer.hpp | Serialize | `virtual bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) = 0;` |
| 新增 | 002-Object | te::object | ISerializer | 反序列化 | te/object/Serializer.hpp | Deserialize | `virtual bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) = 0;` |
| 新增 | 002-Object | te::object | IVersionMigration | 版本迁移 | te/object/VersionMigration.hpp | Migrate | `virtual bool Migrate(SerializedBuffer& buf, uint32_t fromVersion, uint32_t toVersion) = 0;` |
| 新增 | 002-Object | te::object | — | 类型 | te/object/SerializedBuffer.hpp | SerializedBuffer | struct: void* data, size_t size, size_t capacity |
| 新增 | 002-Object | te::object | — | 类型 | te/object/ObjectRef.hpp | ObjectRef | struct: uint8_t guid[16]；跨资源引用仅 GUID |
| 新增 | 002-Object | te::object | — | 类型 | te/object/Guid.hpp | GUID | struct: uint8_t data[16] |
| 新增 | 002-Object | te::object | — | 类型 | te/object/TypeDescriptor.hpp | TypeDescriptor | struct: id, name, size, properties, baseTypeId 等；等价 ITypeInfo |
| 新增 | 002-Object | te::object | PropertyBag | 属性读写 | te/object/PropertyBag.hpp | GetProperty, SetProperty, FindProperty | 与反射和序列化联动 |

*数据约定*：formatVersion 由各描述类型所属模块与本契约约定；调用流程见 `docs/assets/resource-serialization.md` §4、§6。

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| （无） | — | — |
