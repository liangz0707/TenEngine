# Implementation Plan: 001-Core 最小切片（Alloc/Free、Log）

**Branch**: `001-core-minimal` | **Date**: 2026-01-29 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/001-core-minimal/spec.md`  
**规约**: [docs/module-specs/001-core.md](../../docs/module-specs/001-core.md) | **契约**: [specs/_contracts/001-core-public-api.md](../_contracts/001-core-public-api.md)

## Summary

本 feature 实现 001-Core 的**最小可用子集**：**(1) 内存 Alloc/Free**（指定大小与对齐的堆分配、显式释放；Alloc(0 或非法 alignment) 返回 nullptr，Free(nullptr) 与 double-free 为 no-op）；**(2) 分级 Log**（至少 Debug/Info/Warn/Error 四级，输出仅 stdout/stderr，可配置级别→通道及过滤，写入线程安全，不包含 Assert/CrashHandler）。技术栈 C++17、CMake；仅暴露契约已声明的类型与 API；对外接口以本 plan 末尾「契约更新」为准，写回契约「API 雏形」后实施。

## Technical Context

**Language/Version**: C++17  
**Build**: CMake 3.16+  
**Primary Dependencies**: 无（根模块）；可选 std 库：&lt;cstdlib&gt;, &lt;cstddef&gt;, &lt;mutex&gt;, &lt;atomic&gt;, &lt;cstdio&gt; 等  
**Storage**: N/A（本切片无持久化）  
**Testing**: 单元测试（Alloc/Free 成功与边界、Log 级别与过滤）；可选用 Google Test 或手写 main  
**Target Platform**: Windows / Linux / macOS（本切片仅用标准 C++ 与 stdio，无平台 API）  
**Project Type**: 静态库或动态库，供主工程或下游模块链接  
**Performance Goals**: Alloc/Free 为热点路径，避免不必要的锁；Log 写入线程安全、单条消息原子  
**Constraints**: 仅暴露契约中声明的类型与 API；不实现 Allocator 抽象、池化、泄漏追踪、Assert、CrashHandler  
**Scale/Scope**: 本切片仅 Alloc/Free、Log；Thread、Platform、Math、Containers、ModuleLoad 后续切片

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| VI. Module Boundaries & Contract-First | PASS | 仅暴露契约类型/API；契约来源 T0-contracts，本 plan 产出「契约更新」写回 API 雏形 |
| V. Versioning & ABI | PASS | 公开 API 按 MAJOR.MINOR.PATCH；本切片为初始 API 雏形 |
| Technology: C++17, CMake | PASS | 符合 Constitution 技术栈 |
| Code Quality & Testing | PASS | 单元测试覆盖 Alloc/Free 与 Log 契约行为；无违规豁免 |

## Project Structure

### Documentation (this feature)

```text
specs/001-core-minimal/
├── plan.md              # This file
├── research.md          # Phase 0
├── data-model.md        # Phase 1
├── quickstart.md        # Phase 1
├── contracts/           # Phase 1 (API sketch for this slice)
├── checklists/
│   └── requirements.md
└── tasks.md             # (/speckit.tasks - not created by plan)
```

### Source Code (repository root)

```text
include/te/core/           # 公共 API 头文件（仅本切片暴露）
├── alloc.h                # Alloc / Free 声明
└── log.h                  # LogLevel, Log 声明与配置

src/                       # 实现（可拆 te/core/ 子目录）
├── alloc.cpp
└── log.cpp

tests/
├── unit/
│   ├── test_alloc.cpp
│   └── test_log.cpp
└── CMakeLists.txt

CMakeLists.txt             # 根：add_subdirectory(src), add_subdirectory(tests)
```

**Structure Decision**: 单库布局；`include/te/core/` 对应 001-Core 公共 API，本切片仅 `alloc.h`、`log.h`。实现与测试分离，便于契约测试与后续扩展。

## Phase 0: Research Summary

见 [research.md](./research.md)。结论：C++17 使用 `std::aligned_alloc` / `_aligned_malloc` 抽象为统一 Alloc/Free；Log 使用 `std::mutex` 保护写入、单条消息原子输出到 stdout/stderr；级别与通道配置采用最小结构体 + 全局/线程局部配置。

## Phase 1: Data Model & Contracts

- **Data model**: 见 [data-model.md](./data-model.md)。实体：内存块句柄（`void*`）、LogLevel（枚举）、Log 配置（级别阈值、stderr 阈值）。
- **Contracts**: 本切片 API 见下方「契约更新」；详细占位见 [contracts/001-core-minimal-api.md](./contracts/001-core-minimal-api.md)。

## Quickstart

见 [quickstart.md](./quickstart.md)。要点：CMake 配置、构建、运行单元测试、示例调用 Alloc/Free 与 Log。

## Complexity Tracking

> 无 Constitution 违规需豁免。

---

## 契约更新（API 雏形）

以下内容可直接粘贴到 `specs/_contracts/001-core-public-api.md` 的「**API 雏形（简化声明）**」小节，作为本 feature 对外暴露的类型与函数声明。本切片仅实现这些；其余能力（线程、平台、数学、容器、模块加载等）由后续切片补充。

```markdown
### 本切片（001-core-minimal）产出

#### 内存

- `void* Alloc(size_t size, size_t alignment);`
  - 分配至少 `size` 字节、按 `alignment` 对齐的块；失败返回 `nullptr`。
  - `size == 0` 或 `alignment` 不合法（如非 2 的幂）时返回 `nullptr`。
- `void Free(void* ptr);`
  - 释放由 `Alloc` 返回的块；`ptr == nullptr` 或 double-free 为 no-op（安全忽略）。

#### 日志

- 枚举 `LogLevel`: `Debug`, `Info`, `Warn`, `Error`。
- `void Log(LogLevel level, char const* message);`
  - 按级别写入一条消息；输出仅 stdout/stderr，可配置「某级别及以上→stderr、其余→stdout」及级别过滤。
  - 写入为线程安全；单条消息原子输出。
- 配置（示例，具体名由实现定）：设置级别过滤阈值、设置 stderr 阈值（≥ 该级别输出到 stderr），以便可配置通道与过滤。
```

**说明**：初始化/关闭顺序、配置 API 的具名与签名由实现与主工程约定，并在契约「调用顺序与约束」中补充；本雏形仅列出 Alloc/Free、LogLevel、Log 及配置意图。
