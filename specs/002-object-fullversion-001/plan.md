# Implementation Plan: 002-Object Full Feature Set

**Branch**: `002-object-fullversion-001` | **Date**: 2026-01-29 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/002-object-fullversion-001/spec.md`

**规约与契约**：完整模块规约见 `docs/module-specs/002-object.md`，对外 API 契约见 `specs/_contracts/002-object-public-api.md`。本计划实现本切片范围（完整功能集），仅暴露契约已声明的类型与 API。

## Summary

本 feature 实现 002-Object 的**完整功能集**：(1) **反射** 类型注册、类型信息查询、属性/方法枚举、基类链、GetTypeByName/ById；(2) **序列化** 序列化器抽象、二进制与文本格式、版本迁移、对象引用与 GUID 解析；(3) **属性系统** PropertyDescriptor、PropertyBag、元数据、默认值、范围/枚举约束；(4) **类型注册** 注册表、类型工厂 CreateInstance、与 Core 模块加载协调的生命周期。技术栈 C++17、CMake；仅使用上游契约 `specs/_contracts/001-core-public-api.md` 已声明的类型与 API；对外仅暴露 `specs/_contracts/002-object-public-api.md` 能力列表及类型与句柄对应的 API 雏形（见文末「契约更新」）。

## Technical Context

**Language/Version**: C++17  
**Build**: CMake（单一日志化构建；可复现）  
**Primary Dependencies**: 001-Core（仅使用 `specs/_contracts/001-core-public-api.md` 已声明的分配器、容器、字符串、日志等）  
**Storage**: N/A（序列化缓冲由调用方管理；本模块不直接持久化到文件，格式与 013-Resource 等约定在集成阶段）  
**Testing**: 单元测试（反射、序列化往返、属性系统、类型工厂）；可选用 CTest + 现有测试框架  
**Target Platform**: Windows / Linux / macOS（与 Core 一致）  
**Project Type**: 静态库或动态库，由主工程或下游模块链接  
**Performance Goals**: 类型查询、序列化/反序列化、属性访问在合理规模下可接受（具体指标由 spec 或后续任务定）  
**Constraints**: 仅暴露契约已声明的类型与 API；须在 Core 初始化之后使用；重复 TypeId 拒绝、重复类型名由实现约定；GUID 引用格式与 013-Resource 等约定留待集成  
**Scale/Scope**: 本切片实现完整能力列表：反射、序列化（含版本迁移与 GUID）、属性系统、类型注册（含 CreateInstance）

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|--------|
| **VI. Module Boundaries & Contract-First** | PASS | 仅暴露契约声明的类型与 API；本切片对应契约完整能力列表。契约更新见文末「契约更新」。 |
| **Technology: C++17, CMake** | PASS | 语言 C++17，构建 CMake。 |
| **Implementation uses only upstream contracts** | PASS | 仅使用 `001-core-public-api.md` 已声明的类型与接口。 |
| **Public API versioned; breaking changes MAJOR** | PASS | 对外 API 雏形见契约；破坏性变更递增 MAJOR。 |
| **Testing** | PASS | 单元测试覆盖反射、序列化、属性系统、类型工厂；公共 API 与契约一致。 |

## Project Structure

### Documentation (this feature)

```text
specs/002-object-fullversion-001/
├── plan.md              # 本文件
├── research.md          # Phase 0（可选：序列化格式、版本迁移策略、GUID 与 013-Resource 约定）
├── data-model.md        # Phase 1：TypeDescriptor、PropertyDescriptor、SerializedBuffer、ObjectRef、GUID 等
├── quickstart.md        # Phase 1：构建与最小调用示例
├── contracts/           # Phase 1：本 feature 对外 API 与契约对齐说明（可选）
└── tasks.md             # Phase 2（/speckit.tasks 产出）
```

### Source Code (repository root)

本模块以 002-Object 库形式存在，目录名按仓库约定（例如 `modules/object` 或 `te-object`）。

```text
modules/object/                    # 或 te-object/
├── CMakeLists.txt
├── include/
│   └── te/
│       └── object/                # 对外头文件（仅契约声明的 API）
│           ├── TypeId.hpp
│           ├── TypeDescriptor.hpp
│           ├── TypeRegistry.hpp
│           ├── SerializedBuffer.hpp
│           ├── ObjectRef.hpp
│           ├── Guid.hpp
│           ├── Serializer.hpp
│           ├── VersionMigration.hpp
│           ├── PropertyDescriptor.hpp
│           └── PropertyBag.hpp
├── src/
│   ├── TypeRegistry.cpp
│   ├── Serializer.cpp
│   ├── VersionMigration.cpp
│   └── ...
└── tests/
    ├── unit/
    │   ├── TypeRegistry_test.cpp
    │   ├── Serializer_roundtrip_test.cpp
    │   ├── PropertyBag_test.cpp
    │   └── CreateInstance_test.cpp
    └── CMakeLists.txt
```

**Structure Decision**: 单库、头文件与实现分离；对外仅通过 `include/te/object/` 暴露契约中声明的类型与 API。依赖 001-Core 通过 CMake 链接并只引用 Core 契约中的头文件。

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| （无） | — | — |

---

## 契约更新（API 雏形）

以下内容可直接粘贴到 `specs/_contracts/002-object-public-api.md` 的「**API 雏形**」小节（或与现有 minimal 切片并列，作为「完整功能集」小节）。本 feature（002-object-fullversion-001）对外暴露以下类型与函数签名；仅使用 001-Core 契约已声明的类型。

```markdown
### 本切片（002-object-fullversion-001）完整功能集

#### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| TypeId | 类型唯一标识（如 uint32_t 或 opaque 句柄） | 注册后直至卸载 |
| TypeDescriptor | 类型描述：TypeId、名称、大小、属性/方法列表、基类链 | 与类型绑定 |
| SerializedBuffer | 序列化字节流缓冲；由调用方分配与释放 | 调用方管理 |
| ObjectRef | 对象引用；与资源 GUID 对应，序列化/反序列化中解析 | 由调用方或实现管理 |
| GUID | 全局唯一标识，用于资源引用与解析 | 与资源/对象绑定 |
| PropertyDescriptor | 属性描述：名称、类型、元数据、默认值、范围/枚举 | 与类型或实例绑定 |
| PropertyBag | 属性集合；可序列化字段、元数据、约束 | 与类型或实例绑定 |

#### 反射与类型注册

```cpp
namespace te::object {

using TypeId = uint32_t;   // 或 opaque 句柄

struct PropertyDescriptor;  // 属性描述：name, type, metadata, default, range/enum
struct MethodDescriptor;    // 方法描述（本切片内可为占位或最小集）

struct TypeDescriptor {
    TypeId    id;
    char const* name;
    size_t    size;
    // 完整功能集：属性/方法列表、基类链
    PropertyDescriptor const* properties;
    size_t    propertyCount;
    MethodDescriptor const* methods;
    size_t    methodCount;
    TypeId    baseTypeId;   // 基类 TypeId，无则为 0 或无效值
};

class TypeRegistry {
public:
    // 注册类型；重复 TypeId 拒绝，重复 name 由实现约定；返回是否成功
    static bool RegisterType(TypeDescriptor const& desc);

    static TypeDescriptor const* GetTypeByName(char const* name);
    static TypeDescriptor const* GetTypeById(TypeId id);

    // 按 TypeId 创建实例；分配语义按调用约定（调用方或实现方分配）
    static void* CreateInstance(TypeId id);
};

}
```

#### 序列化（含版本迁移与 ObjectRef/GUID）

```cpp
namespace te::object {

struct SerializedBuffer {
    void*  data;
    size_t size;
    size_t capacity;
};

struct ObjectRef {
    uint8_t guid[16];   // 或 GUID 类型；与资源/对象绑定
    // 解析语义与 013-Resource 等约定在集成阶段
};

struct GUID {
    uint8_t data[16];
};

// 版本迁移：旧格式升级到当前格式；本切片必选
class IVersionMigration {
public:
    virtual ~IVersionMigration() = default;
    virtual bool Migrate(SerializedBuffer& buf, uint32_t fromVersion, uint32_t toVersion) = 0;
};

class ISerializer {
public:
    virtual ~ISerializer() = default;
    // 序列化；支持 ObjectRef/GUID 解析；二进制或文本由实现/工厂选择
    virtual bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) = 0;
    virtual bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) = 0;
    // 版本迁移接口（必选）
    virtual uint32_t GetCurrentVersion() const = 0;
    virtual void SetVersionMigration(IVersionMigration* migration) = 0;
};

}
```

#### 属性系统

```cpp
namespace te::object {

struct PropertyDescriptor {
    char const* name;
    TypeId      valueTypeId;
    void const* defaultValue;   // 可选
    // 元数据、范围/枚举约束（可由扩展结构或后续接口提供）
};

class PropertyBag {
public:
    // 读写属性；与反射和序列化联动
    virtual bool GetProperty(void* outValue, char const* name) const = 0;
    virtual bool SetProperty(void const* value, char const* name) = 0;
    virtual PropertyDescriptor const* FindProperty(char const* name) const = 0;
    virtual ~PropertyBag() = default;
};

}
```

#### 调用顺序与约束（本切片）

- 须在 Core 初始化之后使用；类型注册可在启动时或按模块加载时进行。
- 重复 TypeId 拒绝；重复类型名由实现约定（覆盖或拒绝），须在实现/契约中明确。
- SerializedBuffer 的 data 可由调用方使用 Core 分配器（Alloc/Free）分配。
- 版本迁移为本切片必选能力；序列化支持二进制与文本两种格式（或通过抽象可扩展）。
- GUID 引用格式与 013-Resource 等消费者的约定留待集成阶段；本 feature 保证 GUID 可解析、可扩展。
```
