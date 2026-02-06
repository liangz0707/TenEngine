# Implementation Plan: [FEATURE]

**Branch**: `[###-feature-name]` | **Date**: [DATE] | **Spec**: [link]
**Input**: Feature specification from `/specs/[###-feature-name]/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. 执行流程见 `.cursor/commands/speckit.plan.md`。

## Summary

[Extract from feature spec: primary requirement + technical approach from research]

## 实现范围（TenEngine：实现全量 ABI 内容）

> **ABI 生成与保存模式**：
> - **全量 ABI 内容写在 plan.md 内**：plan 执行时，参考 spec、Unity/Unreal 等文档，**生成本 feature 需要实现的全部 ABI 内容**（原始 + 新增 + 修改），**直接写在 plan.md 的本小节或下方「全量 ABI 内容（实现参考）」中**，不单独生成 `contracts/NNN-modulename-ABI-full.md` 等文件。
> - **契约更新只保存变化部分**：plan.md 的「契约更新」小节**只保存相对于现有 ABI 的新增和修改部分**，用于写回 `specs/_contracts/NNN-modulename-ABI.md`；若无新增/修改则产出空清单。
> - **实现基于全量内容**：tasks 和 implement 阶段**必须基于 plan.md 中的全量 ABI 内容**（原始 + 新增 + 修改）进行实现，不得仅实现变化部分。
>
> **规约**：本 feature **只实现 ABI 文件**中列出的符号与能力；不得设计或实现 ABI 未声明的对外接口。设计时可参考 **Unity、Unreal** 的模块与 API 构造。对外接口以 ABI 文件为准。见 `specs/_contracts/README.md`、`.specify/memory/constitution.md` §VI。

[若本 feature 对应某模块，在此列出本切片要实现的**全量 ABI 符号**（包括原始、新增、修改），或引用下方「全量 ABI 内容（实现参考）」表。]

### 全量 ABI 内容（实现参考）

[本 feature 对应的**全量 ABI 表**（原始 + 新增 + 修改），与 `specs/_contracts/NNN-modulename-ABI.md` 表头一致。tasks/implement 以此为准进行实现。]

## Technical Context

<!--
  ACTION REQUIRED: Replace the content in this section with the technical details
  for the project. The structure here is presented in advisory capacity to guide
  the iteration process.
-->

**Language/Version**: [e.g., Python 3.11, Swift 5.9, Rust 1.75 or NEEDS CLARIFICATION]  
**Primary Dependencies**: [e.g., FastAPI, UIKit, LLVM or NEEDS CLARIFICATION]  
**Storage**: [if applicable, e.g., PostgreSQL, CoreData, files or N/A]  
**Testing**: [e.g., pytest, XCTest, cargo test or NEEDS CLARIFICATION]  
**Target Platform**: [e.g., Linux server, iOS 15+, WASM or NEEDS CLARIFICATION]

## 依赖引入方式（TenEngine 构建规约）

> **规约**：见 `docs/engine-build-module-convention.md`。**当前所有子模块构建均使用源码方式**。对本 feature 的**每个直接依赖**列出模块 ID 即可；通过 TenEngineHelpers / `tenengine_resolve_my_dependencies` 以源码（add_subdirectory/FetchContent）引入。

| 依赖模块（如 001-core） | 引入方式 | 说明 |
|-------------------------|----------|------|
| [e.g. 001-core] | **源码** | 通过 TenEngineHelpers 引入上游源码构建（同级 worktree 或 TENENGINE_xxx_DIR）。 |
| … | 源码 | 当前统一使用源码，无需标注 DLL/不引入。 |

**说明**：当前仅支持源码引入；列出直接依赖后由 CMake 脚本统一解析。

### 第三方依赖（本 feature 涉及模块所需）

> **规约**：若本 feature 涉及的模块在 `specs/_contracts/NNN-modulename-public-api.md` 中声明了第三方库，**必须**在本节列出；每个第三方对应 `docs/third_party/<id>-<name>.md`，引入方式见该文档或 [third_party-integration-workflow.md](../../docs/third_party-integration-workflow.md)。Plan 指令从 public-api 读取并据此自动加入；Task 阶段将生成「版本选择、自动下载、配置、安装、编译测试、部署进工程、配置实现」等任务。

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| （无则填「本 feature 无第三方依赖」） | — | — | — |

## 构建规约与目录、CMake 检查（TenEngine，engine-build-module-convention）

> 若本 feature 涉及 CMake/构建，**必须**填写本小节。执行 cmake/构建任务前须满足以下约定；规约见 `docs/engine-build-module-convention.md` §2.5、§3。

### 构建根目录

| 模式 | 构建根目录 | CMakeLists.txt 位置 |
|------|------------|---------------------|
| **单仓** | **仓库根** | 根目录 `CMakeLists.txt` 仅 `project(...)`、`add_subdirectory(Engine/...)`；**不**在根定义库或可执行文件。 |
| 多 worktree | 当前模块 worktree 根 | worktree 根目录下 `CMakeLists.txt` 定义本模块。 |

**本 feature**：[单仓则填「构建根目录 = 仓库根；out-of-source 推荐 build/ 在仓库根下」；多 worktree 则填 worktree 路径。]

### 目录结构（§2.5）

| 约定 | 要求 |
|------|------|
| Engine 为代码根 | 所有可构建模块的 **src/、include/、tests/、cmake/** 均在 **`Engine/`** 下；**禁止**在仓库根直接放置 `src/`、`include/`、`tests/`。 |
| 模块目录命名 | **`Engine/TenEngine-NNN-modulename/`**（如 Engine/TenEngine-013-resource）。 |
| 模块内必选 | **`src/`**、**`include/`**、**`tests/`**、**`cmake/`**、**`CMakeLists.txt`**。 |

### CMake 检查清单

- [ ] 根 `CMakeLists.txt` 仅含 project、set(CMAKE_CXX_STANDARD)、add_subdirectory(Engine/...)；无 add_library/add_executable。
- [ ] 本模块 `Engine/TenEngine-NNN-xxx/CMakeLists.txt` 以**该模块目录**为当前作用域；相对路径 `src/`、`include/`、`tests/`；target 名与 ABI 一致（如 te_resource）；target_link_libraries 仅 link 上游 target。
- [ ] 本模块 tests 仅在本模块 `tests/` 下；**不** add_subdirectory(上游/tests)；测试只 link **本模块 target**。
- [ ] target_include_directories 与契约头文件路径一致（如 include/te/resource/ 对应 te/resource/*.h）。

**Technical Context（可选续）**：**Project Type**: [single/web/mobile - determines source structure]  
**Performance Goals**: [domain-specific, e.g., 1000 req/s, 10k lines/sec, 60 fps or NEEDS CLARIFICATION]  
**Constraints**: [domain-specific, e.g., <200ms p95, <100MB memory, offline-capable or NEEDS CLARIFICATION]  
**Scale/Scope**: [domain-specific, e.g., 10k users, 1M LOC, 50 screens or NEEDS CLARIFICATION]

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

[Gates determined based on constitution file]

## Project Structure

### Documentation (this feature)

```text
specs/[###-feature]/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code（仓库布局）

<!-- TenEngine 单仓：代码在 Engine/ 下，见上方「构建规约与目录、CMake 检查」；根目录无 src/include/tests。 -->

```text
# TenEngine 单仓（§2.5）：代码在 Engine/ 下；根目录仅 CMakeLists.txt、Engine/、docs/、specs/
CMakeLists.txt           # 仅 add_subdirectory(Engine/...)
Engine/
├── TenEngine-NNN-modulename/
│   ├── CMakeLists.txt
│   ├── include/         # 公开头文件（与契约路径一致，如 te/resource/）
│   ├── src/
│   ├── tests/           # contract/, unit/, integration/
│   └── cmake/

# 非 TenEngine 或多 worktree 时可用下方结构替代
# Option 1: Single project
# src/, tests/
# Option 2: Web (backend/, frontend/)
# Option 3: Mobile + API (api/, ios/ or android/)
```

**Structure Decision**: [TenEngine 单仓：本模块代码在 `Engine/TenEngine-NNN-modulename/`；根仅 add_subdirectory。或写明所选 Option。]

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

> **注意**：本小节**只保存相对于现有 ABI 的新增和修改**条目，用于查阅和写回契约；若无则本小节为空。写回时仅将下表增补或替换到 `specs/_contracts/NNN-modulename-ABI.md` 中。
>
> **实现时使用全量内容**：tasks 和 implement 阶段应基于**全量 ABI 内容**（包括原始 ABI、新增、修改）进行实现，详见上方「实现范围」小节。

| 操作 | 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|------|--------|----------|------|----------|--------|------|------|
| 新增/修改 | … | … | … | … | … | … | 完整函数签名 |

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |
