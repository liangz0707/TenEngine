# Implementation Plan: 002-Object Minimal Slice

**Branch**: `002-object-minimal` | **Date**: 2026-01-29 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/002-object-minimal/spec.md`

**规约与契约**：完整模块规约见 `docs/module-specs/002-object.md`，对外 API 契约见 `specs/_contracts/002-object-public-api.md`。本计划仅实现本切片范围，仅暴露契约已声明的类型与 API。

## Summary

本 feature 实现 002-Object 的最小子集：(1) **类型注册** TypeRegistry::RegisterType 与按名/ID 查询；(2) **简单序列化** Serializer 抽象与最小二进制写入/读取。技术栈 C++17、CMake；仅使用上游契约 `specs/_contracts/001-core-public-api.md` 已声明的类型与 API；对外仅暴露 `specs/_contracts/002-object-public-api.md` 中与本切片对应的 API 雏形（见文末「契约更新」）。

## Technical Context

**Language/Version**: C++17  
**Build**: CMake（单一日志化构建；可复现）  
**Primary Dependencies**: 001-Core（仅使用 `specs/_contracts/001-core-public-api.md` 已声明的分配器、容器、字符串、日志等）  
**Storage**: N/A（序列化缓冲由调用方管理；本切片不持久化到文件）  
**Testing**: 单元测试（类型注册、序列化往返）；可选用 CTest + 现有测试框架  
**Target Platform**: Windows / Linux / macOS（与 Core 一致）  
**Project Type**: 静态库或动态库，由主工程或下游模块链接  
**Performance Goals**: 类型查询与简单序列化在合理规模下可接受（具体指标由 spec 或后续任务定）  
**Constraints**: 仅暴露契约已声明的类型与 API；无版本迁移、无 ObjectRef/GUID 解析；须在 Core 初始化之后使用  
**Scale/Scope**: 本切片仅支持已注册类型的简单序列化（标量、简单聚合）；无属性系统、无完整反射枚举

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|--------|
| **VI. Module Boundaries & Contract-First** | PASS | 仅暴露契约声明的类型与 API；本切片对应契约能力：类型注册、简单序列化。契约更新见文末「契约更新」。 |
| **Technology: C++17, CMake** | PASS | 语言 C++17，构建 CMake。 |
| **Implementation uses only upstream contracts** | PASS | 仅使用 `001-core-public-api.md` 已声明的类型与接口。 |
| **Public API versioned; breaking changes MAJOR** | PASS | 对外 API 雏形见契约；破坏性变更递增 MAJOR。 |
| **Testing** | PASS | 单元测试覆盖类型注册与序列化往返；公共 API 与契约一致。 |

## Project Structure

### Documentation (this feature)

```text
specs/002-object-minimal/
├── plan.md              # 本文件
├── research.md          # Phase 0（可选：序列化格式、类型 ID 方案）
├── data-model.md        # Phase 1：TypeDescriptor、SerializedBuffer 等数据结构
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
│           └── Serializer.hpp
├── src/
│   ├── TypeRegistry.cpp
│   ├── Serializer.cpp
│   └── ...
└── tests/
    ├── unit/
    │   ├── TypeRegistry_test.cpp
    │   └── Serializer_roundtrip_test.cpp
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

以下内容可直接粘贴到 `specs/_contracts/002-object-public-api.md` 的「**API 雏形**」小节。本 feature 对外仅暴露以下类型与函数签名；不包含属性系统、GUID、ObjectRef、类型工厂 CreateInstance 等本切片未实现的能力。

```markdown
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
    TypeId    id;
    char const* name;     // 或使用 Core 契约中的 String 类型
    size_t    size;       // 实例大小（本切片最小所需）
    // 本切片不包含：属性列表、基类链、方法列表
};

class TypeRegistry {
public:
    // 注册类型；返回是否成功（如重复 id/name 可规定为覆盖或失败）
    static bool RegisterType(TypeDescriptor const& desc);

    // 按名查询；未找到返回 nullptr 或无效 TypeId
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

// 提供默认实现（如最小二进制格式）；具体实现可由工厂或模块初始化返回
// std::unique_ptr<ISerializer> CreateDefaultSerializer();
}
```

### 调用顺序与约束（本切片）

- 须在 Core 初始化之后使用；类型注册可在启动时或按模块加载时进行。
- SerializedBuffer 的 data 可由调用方使用 Core 分配器分配；本切片不规定缓冲格式的版本迁移与 GUID 解析。
```
