# Implementation Plan: [FEATURE]

**Branch**: `[###-feature-name]` | **Date**: [DATE] | **Spec**: [link]
**Input**: Feature specification from `/specs/[###-feature-name]/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. 执行流程见 `.cursor/commands/speckit.plan.md`。

## Summary

[Extract from feature spec: primary requirement + technical approach from research]

## 实现范围（TenEngine：实现全量 ABI 内容）

> **ABI 生成与保存模式**：
> - **生成全量 ABI 内容**：plan 执行时，参考 spec、Unity/Unreal 等文档，**生成本 feature 需要实现的全部 ABI 内容**，并可进行优化（补充、删除、合并等），包括：
>   - **原始 ABI**：现有 `specs/_contracts/NNN-modulename-ABI.md` 中已声明的所有条目（本 feature 需要实现的）
>   - **新增的 ABI**：本 feature 新增的接口条目
>   - **修改的 ABI**：对现有 ABI 条目的修改
> - **文档中只保存变化部分**：plan.md 文档的「契约更新」小节**只保存相对于现有 ABI 的新增和修改部分**，用于查阅和写回契约；不保存完整 ABI 表。
> - **实现基于全量内容**：tasks 和 implement 阶段**必须基于全量 ABI 内容**（原始 + 新增 + 修改）进行实现，不得仅实现变化部分。
>
> **规约**：本 feature **只实现 ABI 文件**中列出的符号与能力；不得设计或实现 ABI 未声明的对外接口。设计时可参考 **Unity、Unreal** 的模块与 API 构造。对外接口以 ABI 文件为准。见 `specs/_contracts/README.md`、`.specify/memory/constitution.md` §VI。

[若本 feature 对应某模块，在此列出本切片要实现的**全量 ABI 符号**（包括原始、新增、修改），或引用 ABI 文件中的表行。]

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

### Source Code (repository root)
<!--
  ACTION REQUIRED: Replace the placeholder tree below with the concrete layout
  for this feature. Delete unused options and expand the chosen structure with
  real paths (e.g., apps/admin, packages/something). The delivered plan must
  not include Option labels.
-->

```text
# [REMOVE IF UNUSED] Option 1: Single project (DEFAULT)
src/
├── models/
├── services/
├── cli/
└── lib/

tests/
├── contract/
├── integration/
└── unit/

# [REMOVE IF UNUSED] Option 2: Web application (when "frontend" + "backend" detected)
backend/
├── src/
│   ├── models/
│   ├── services/
│   └── api/
└── tests/

frontend/
├── src/
│   ├── components/
│   ├── pages/
│   └── services/
└── tests/

# [REMOVE IF UNUSED] Option 3: Mobile + API (when "iOS/Android" detected)
api/
└── [same as backend above]

ios/ or android/
└── [platform-specific structure: feature modules, UI flows, platform tests]
```

**Structure Decision**: [Document the selected structure and reference the real
directories captured above]

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
