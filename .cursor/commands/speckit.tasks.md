---
description: Generate an actionable, dependency-ordered tasks.md for the feature based on available design artifacts.
handoffs: 
  - label: Analyze For Consistency
    agent: speckit.analyze
    prompt: Run a project analysis for consistency
    send: true
  - label: Implement Project
    agent: speckit.implement
    prompt: Start the implementation in phases
    send: true
---

## User Input

```text
$ARGUMENTS
```

You **MUST** consider the user input before proceeding (if not empty).

## Outline

1. **Setup**: Run `.specify/scripts/powershell/check-prerequisites.ps1 -Json` from repo root and parse FEATURE_DIR and AVAILABLE_DOCS list. All paths must be absolute. For single quotes in args like "I'm Groot", use escape syntax: e.g 'I'\''m Groot' (or double-quote if possible: "I'm Groot").

2. **Load design documents**: Read from FEATURE_DIR:
   - **Required**: plan.md (tech stack, libraries, structure), spec.md (user stories with priorities)
   - **Optional**: data-model.md (entities), contracts/（API 或 ABI 设计产物；TenEngine 为接口/ABI 草图）, research.md (decisions), quickstart.md (test scenarios)
   - **TenEngine ABI 说明**：plan 生成时已产生**全量 ABI 内容**（包括原始 ABI、新增、修改），虽然 plan.md 中只保存变化部分，但 tasks 生成时必须基于**全量 ABI 内容**（从现有 `specs/_contracts/NNN-modulename-ABI.md` + plan 生成的新增/修改）进行任务分解，确保实现所有需要的接口。
   - Note: Not all projects have all documents. Generate tasks based on what's available.

3. **Execute task generation workflow**:
   - Load plan.md and extract tech stack, libraries, project structure
   - Load spec.md and extract user stories with their priorities (P1, P2, P3, etc.)
   - If data-model.md exists: Extract entities and map to user stories
   - If contracts/ exists: Map contracts/endpoints or ABI artifacts to user stories
   - If research.md exists: Extract decisions for setup tasks
   - Generate tasks organized by user story (see Task Generation Rules below)
   - Generate dependency graph showing user story completion order
   - Create parallel execution examples per user story
   - Validate task completeness (each user story has all needed tasks, independently testable)

4. **Generate tasks.md**: Use `.specify/templates/tasks-template.md` as structure, fill with:
   - Correct feature name from plan.md
   - **Build/CMake 任务（TenEngine）**：若任务包含「配置/构建工程」或执行 `cmake -B build`，该任务**必须**注明：执行前须已澄清 **根目录**（构建所在模块路径）；**各子模块均使用源码方式**引入依赖，未澄清根目录时**禁止**直接执行 cmake，须先向用户询问。**cmake 生成之后须检查**：引入的头文件/源文件是否完整、是否存在循环依赖或缺失依赖；发现问题须在任务中标注或先修复再继续。规约见 `docs/engine-build-module-convention.md` §3（构建方式澄清）。
   - **第三方库任务（TenEngine）**：若 plan 的「第三方依赖」小节列出第三方库，**必须**为每个第三方生成覆盖以下 7 步的任务（可合并为少量任务项）：① 版本适配/选择；② **自动下载（必须）**（FetchContent / git submodule / 脚本，禁止假设已存在）；③ 配置（CMake 选项）；④ 安装；⑤ 编译测试；⑥ 部署进工程；⑦ 配置实现（target_link_libraries、include、宏定义）。引入方式（header-only/source/sdk/system）与 CMake 写法从 `docs/third_party/<id>-<name>.md` 或 `docs/third_party-integration-workflow.md` 读取；详见该工作流文档 §四、§五。
   - Phase 1: Setup tasks (project initialization)
   - Phase 2: Foundational tasks (blocking prerequisites for all user stories)
   - Phase 3+: One phase per user story (in priority order from spec.md)
   - Each phase includes: story goal, independent test criteria, tests (if requested), implementation tasks
   - Final Phase: Polish & cross-cutting concerns
   - All tasks must follow the strict checklist format (see Task Generation Rules below)
   - Clear file paths for each task
   - Dependencies section showing story completion order
   - Parallel execution examples per story
   - Implementation strategy section (MVP first, incremental delivery)

5. **Report**: Output path to generated tasks.md and summary:
   - Total task count
   - Task count per user story
   - Parallel opportunities identified
   - Independent test criteria for each story
   - Suggested MVP scope (typically just User Story 1)
   - Format validation: Confirm ALL tasks follow the checklist format (checkbox, ID, labels, file paths)

Context for task generation: $ARGUMENTS

The tasks.md should be immediately executable - each task must be specific enough that an LLM can complete it without additional context.

**生成/构建工程**：若 tasks 中有配置或构建（如 cmake），执行该任务前**必须**已与用户确认 **根目录**；**各子模块均使用源码方式**（见 `docs/engine-build-module-convention.md` §3）。否则不得直接运行 cmake，应先询问用户。**cmake 生成之后须检查**：引入的头文件/源文件是否完整、是否存在循环依赖或缺失依赖，有问题须在任务中标注或先修复再继续。

## Task Generation Rules

**CRITICAL**: Tasks MUST be organized by user story to enable independent implementation and testing.

**Tests are OPTIONAL**: Only generate test tasks if explicitly requested in the feature specification or if user requests TDD approach.

**测试逻辑（TenEngine）**：生成测试任务时，测试逻辑**须能覆盖**：① **上游模块能力**：不得仅测本模块孤立逻辑；应通过调用本模块对外接口（其内部使用上游依赖）或显式调用上游模块 API，验证依赖链正确；② **第三方库调用能力**：若本模块依赖第三方库，测试须包含对第三方库的实际调用（如 spdlog、glm、stb、assimp 等 API），以验证第三方集成有效。测试可执行文件仍只 link 本模块 target（依赖经 target_link_libraries 传递）；测试代码应主动调用上游与第三方 API。

### Checklist Format (REQUIRED)

Every task MUST strictly follow this format:

```text
- [ ] [TaskID] [P?] [Story?] Description with file path
```

**Format Components**:

1. **Checkbox**: ALWAYS start with `- [ ]` (markdown checkbox)
2. **Task ID**: Sequential number (T001, T002, T003...) in execution order
3. **[P] marker**: Include ONLY if task is parallelizable (different files, no dependencies on incomplete tasks)
4. **[Story] label**: REQUIRED for user story phase tasks only
   - Format: [US1], [US2], [US3], etc. (maps to user stories from spec.md)
   - Setup phase: NO story label
   - Foundational phase: NO story label  
   - User Story phases: MUST have story label
   - Polish phase: NO story label
5. **Description**: Clear action with exact file path

**Examples**:

- ✅ CORRECT: `- [ ] T001 Create project structure per implementation plan`
- ✅ CORRECT: `- [ ] T005 [P] Implement authentication middleware in src/middleware/auth.py`
- ✅ CORRECT: `- [ ] T012 [P] [US1] Create User model in src/models/user.py`
- ✅ CORRECT: `- [ ] T014 [US1] Implement UserService in src/services/user_service.py`
- ❌ WRONG: `- [ ] Create User model` (missing ID and Story label)
- ❌ WRONG: `T001 [US1] Create model` (missing checkbox)
- ❌ WRONG: `- [ ] [US1] Create User model` (missing Task ID)
- ❌ WRONG: `- [ ] T001 [US1] Create model` (missing file path)

### Task Organization

1. **From User Stories (spec.md)** - PRIMARY ORGANIZATION:
   - Each user story (P1, P2, P3...) gets its own phase
   - Map all related components to their story:
     - Models needed for that story
     - Services needed for that story
     - Endpoints/UI needed for that story
     - If tests requested: Tests specific to that story
   - Mark story dependencies (most stories should be independent)

2. **From Contracts**（contracts/ 为 API 或 ABI 设计产物；TenEngine 为接口/ABI 草图）:
   - Map each contract/endpoint or ABI artifact → to the user story it serves
   - **TenEngine ABI**：基于**全量 ABI 内容**（现有 `specs/_contracts/NNN-modulename-ABI.md` 中的原始条目 + plan 生成的新增/修改）生成任务，确保实现所有需要的接口，不得仅实现变化部分。
   - If tests requested: Each contract → contract test task [P] before implementation in that story's phase

3. **From Data Model**:
   - Map each entity to the user story(ies) that need it
   - If entity serves multiple stories: Put in earliest story or Setup phase
   - Relationships → service layer tasks in appropriate story phase

4. **From Setup/Infrastructure**:
   - Shared infrastructure → Setup phase (Phase 1)
   - Foundational/blocking tasks → Foundational phase (Phase 2)
   - Story-specific setup → within that story's phase

5. **From Third-Party Dependencies (plan 的「第三方依赖」)**:
   - 对 plan 中列出的每个第三方，生成覆盖 7 步的任务：版本选择 → 自动下载（必须）→ 配置 → 安装 → 编译测试 → 部署进工程 → 配置实现。
   - 从 `docs/third_party/<id>-<name>.md` 读取引入方式（header-only/source/sdk/system）与 CMake 写法，生成对应任务描述。
   - 第三方任务通常放在 Setup 或 Foundational 阶段，优先于依赖该第三方的功能任务。

### Phase Structure

- **Phase 1**: Setup (project initialization)
- **Phase 2**: Foundational (blocking prerequisites - MUST complete before user stories)
- **Phase 3+**: User Stories in priority order (P1, P2, P3...)
  - Within each story: Tests (if requested) → Models → Services → Endpoints → Integration
  - Each phase should be a complete, independently testable increment
- **Final Phase**: Polish & Cross-Cutting Concerns
