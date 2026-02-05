# Implementation Plan: 013-Resource 最小切片（ResourceId / LoadSync / Release）

**Branch**: `013-resource-fullversion-001` | **Date**: 2026-01-29 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/013-resource-fullversion-001/spec.md`

**规约与契约**：`docs/module-specs/013-resource.md`、`specs/_contracts/013-resource-public-api.md`。本 feature 仅实现本切片范围：ResourceId、LoadSync、Release；仅暴露契约已声明的类型与 API。

## Summary

实现 013-Resource 模块的最小切片：**ResourceId**（资源唯一标识，支持可寻址路径与 GUID）、**LoadSync**（同步加载，返回结果类型：成功带 LoadHandle、失败带错误码，不抛异常）、**Release**（释放 LoadHandle，幂等）。技术栈 C++17、CMake；依赖 001-Core、002-Object，仅使用两模块契约已声明的类型与 API。计划结束时产出一份「契约更新」：本 feature 对外暴露的函数签名与类型，格式可直接粘贴到契约的「API 雏形」小节。

## Technical Context

**Language/Version**: C++17  
**Primary Dependencies**: 001-Core（内存、文件、字符串等）、002-Object（GUID/引用解析、类型）；仅使用两模块契约已声明类型与 API。  
**Storage**: 无独立存储；资源元数据与句柄由本模块管理，底层 I/O 通过 Core 契约。  
**Testing**: 单元测试（LoadSync/Release 语义、幂等、结果类型）；契约/集成测试（与 Core、Object 对接）。  
**Target Platform**: 与 Core 一致（Windows/Linux/macOS 等）。

## 依赖引入方式（TenEngine 构建规约：必须澄清）

> **规约**：见 `docs/build-module-convention.md`。对本 feature 的**每个直接依赖**必须明确写出引入方式之一；**未写明时默认使用源码引入**。

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-core | **源码** | 通过 TenEngineHelpers 引入上游源码构建；TENENGINE_001_CORE_USE_SOURCE=ON、TENENGINE_001_CORE_DIR 或 TENENGINE_ROOT。 |
| 002-object | **源码** | 通过 TenEngineHelpers 引入上游源码构建；TENENGINE_002_OBJECT_USE_SOURCE=ON、TENENGINE_002_OBJECT_DIR 或 TENENGINE_ROOT。 |

**说明**：未写明时按**源码**处理。构建根目录为 TenEngine-013-resource；CMake 使用 `cmake/TenEngineHelpers.cmake`、`tenengine_resolve_my_dependencies("013-resource" ...)` 解析依赖。

**Project Type**: 单库（静态库 te_resource）；include + src + tests。  
**Performance Goals**: 本 spec 不量化「约定时间」；具体时限/性能在实现或后续迭代定义。  
**Constraints**: 仅暴露契约已声明的类型与 API；不实现 LoadAsync、Streaming、Addressing 全量、Import、GC、UnloadPolicy。  
**Scale/Scope**: 本切片：3 个对外能力（ResourceId、LoadSync、Release）；最小可验收集合。

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 原则 | 状态 | 说明 |
|------|------|------|
| I. Modular Renderer Architecture | PASS | 本模块边界清晰，仅实现 Resource 切片，独立可测。 |
| II. Modern Graphics API First | N/A | 本切片无 GPU 直接依赖。 |
| III. Data-Driven Pipeline | N/A | 本切片无管线配置。 |
| IV. Performance & Observability | PASS | 可加日志/指标；性能目标留 plan 或实现阶段。 |
| V. Versioning & ABI Stability | PASS | 公开 API 与契约一致，版本化遵循 Constitution。 |
| VI. Module Boundaries & Contract-First | PASS | 仅使用 001-Core、002-Object 契约已声明类型与 API；本模块仅暴露契约已声明的 ResourceId、LoadHandle、LoadSync、Release。 |
| Technology Stack (C++17, CMake) | PASS | C++17、CMake；单文档化构建系统。 |
| Code Quality & Testing | PASS | 单元/契约测试覆盖 LoadSync、Release、幂等、结果类型。 |

## Project Structure

### Documentation (this feature)

```text
specs/013-resource-fullversion-001/
├── plan.md              # 本文件
├── research.md           # Phase 0 产出
├── data-model.md         # Phase 1 产出
├── quickstart.md         # Phase 1 产出
├── contracts/            # 可选；API 雏形见 plan 末尾「契约更新」
└── tasks.md              # /speckit.tasks 产出（本命令不创建）
```

### Source Code (repository root: TenEngine-013-resource)

```text
include/
└── te_resource/          # 或 te/resource.hpp 等，与项目约定一致
    └── resource.hpp      # ResourceId, LoadHandle, LoadResult, LoadSync, Release 声明

src/
└── resource.cpp          # LoadSync / Release 实现；ResourceId 解析依赖 Object 契约

tests/
├── unit/
│   └── resource_test.cpp # LoadSync 成功/失败、Release 幂等、结果类型
└── contract/             # 可选：与 Core/Object 对接的契约测试

cmake/
├── TenEngineHelpers.cmake
└── TenEngineModuleDependencies.cmake
```

**Structure Decision**: 单库 + include/src/tests；公开 API 仅头文件声明与实现（或 PIMPL），与契约「API 雏形」一致。依赖通过 `tenengine_resolve_my_dependencies("013-resource" OUT_DEPS MY_DEPS)` 与 `target_link_libraries(te_resource PRIVATE ${MY_DEPS})` 引入。

## Complexity Tracking

> 无违规；不填表。

---

## Phase 0: Outline & Research

- **research.md** 已生成：技术选型（C++17、结果类型表示、ResourceId 双形式）、与 Core/Object 契约对接方式、错误码约定。
- 无 NEEDS CLARIFICATION；所有未知已在 spec 澄清阶段解决。

## Phase 1: Design & Contracts

- **data-model.md**：ResourceId（Path/GUID）、LoadHandle（不透明句柄）、LoadResult（success + handle + error_code）；生命周期与 Release 幂等。
- **quickstart.md**：如何包含头文件、调用 LoadSync/Release、处理失败。
- **契约更新**：见本 plan 末尾「契约更新」小节；可直接粘贴到 `specs/_contracts/013-resource-public-api.md` 的「API 雏形」小节。

---

## 契约更新（本 feature 对外暴露的函数签名与类型）

以下内容可直接粘贴到 `specs/_contracts/013-resource-public-api.md` 的「API 雏形」小节（若该小节已存在则替换或合并本切片部分）。

### API 雏形（本切片：013-resource-fullversion-001）

#### 类型与句柄（本切片暴露）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ResourceId | 资源唯一标识；本切片支持可寻址路径与 GUID 两种形式，与 Object 引用解析对接 | 调用方管理 |
| LoadHandle | 同步加载得到的句柄；不透明，由实现管理 | 请求发出至 Release 或进程结束 |
| LoadResult | LoadSync 返回类型：成功带 LoadHandle，失败带错误码；不抛异常 | 栈或调用方管理 |

#### 函数签名（C++17）

```cpp
namespace te::resource {

// ResourceId：支持可寻址路径或 GUID（与 Object 契约对接）
struct ResourceId {
    enum class Kind { Path, Guid };
    Kind kind;
    char const* value;   // path 或 guid 字符串；具体类型可与 Core/String 对齐
};

// LoadHandle：不透明句柄；每次 LoadSync 成功返回新句柄（显式引用）
using LoadHandle = void*;  // 或 struct LoadHandle { void* impl; }; 由实现定义

// LoadResult：成功时 success==true 且 handle 有效；失败时 success==false 且 error_code 有效
struct LoadResult {
    bool        success;    // true = 成功，false = 失败
    LoadHandle  handle;     // success 时有效；失败时为 nullptr 或未定义
    int         error_code; // 失败时错误码；0 表示无错误
};

// LoadSync：给定 ResourceId，同步加载；失败不抛异常，返回 LoadResult
LoadResult LoadSync(ResourceId const& id);

// Release：给定 LoadHandle，释放资源；对同一句柄多次调用为幂等（无操作/成功）
void Release(LoadHandle handle);

}
```

#### 调用顺序与约束（本切片）

- 须在 Core、Object 初始化之后调用 LoadSync/Release。
- ResourceId 的 value 格式（路径/GUID 字符串）须与 Object 序列化约定一致。
- 各模块资源句柄的释放顺序须与 Resource 卸载策略协调；对同一 LoadHandle 多次 Release 为幂等。
