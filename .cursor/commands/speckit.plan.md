---
description: Execute the implementation planning workflow using the plan template to generate design artifacts.
handoffs: 
  - label: Create Tasks
    agent: speckit.tasks
    prompt: Break the plan into tasks
    send: true
  - label: Create Checklist
    agent: speckit.checklist
    prompt: Create a checklist for the following domain...
---

## User Input

```text
$ARGUMENTS
```

You **MUST** consider the user input before proceeding (if not empty).

## Outline

1. **Setup**: Run `.specify/scripts/powershell/setup-plan.ps1 -Json` from repo root and parse JSON for FEATURE_SPEC, IMPL_PLAN, SPECS_DIR, BRANCH. For single quotes in args like "I'm Groot", use escape syntax: e.g 'I'\''m Groot' (or double-quote if possible: "I'm Groot").

2. **Load context**: Read FEATURE_SPEC and `.specify/memory/constitution.md`. Load IMPL_PLAN template (already copied).

3. **Execute plan workflow**: Follow the structure in IMPL_PLAN template to:
   - Fill Technical Context (mark unknowns as "NEEDS CLARIFICATION")
   - Fill Constitution Check section from constitution
   - **实现范围 (TenEngine)**：本 feature **只实现 ABI 文件**（`specs/_contracts/NNN-modulename-ABI.md`）中列出的符号与能力；不得设计或实现 ABI 未声明的对外接口。设计时可参考 **Unity、Unreal** 的模块与 API 构造。对外接口以 ABI 文件为准；契约（public-api）描述能力与类型，接口符号与签名以 ABI 为准。**若本 feature 对现存 ABI 有新增或变更**，则计划结束时产出一份「契约更新」：列出新增/变更的命名空间、头文件、符号与完整签名，可直接用于更新 ABI 与 public-api。见 `specs/_contracts/README.md`、`.specify/memory/constitution.md` §VI。
   - **Build convention (TenEngine)**：若本 feature 涉及 CMake/构建，**必须**在 plan.md 中填写「依赖引入方式」小节（见 `.specify/templates/plan-template.md`）：每个直接依赖为 **源码** / **DLL** / **不引入外部库**，默认源码。规约见 `docs/engine-build-module-convention.md`。
   - Evaluate gates (ERROR if violations unjustified)
   - Phase 0: Generate research.md (resolve all NEEDS CLARIFICATION)
   - Phase 1: Generate data-model.md, contracts/, quickstart.md
   - Phase 1: Update agent context by running the agent script
   - Re-evaluate Constitution Check post-design

4. **Stop and report**: Command ends after Phase 2 planning. Report branch, IMPL_PLAN path, and generated artifacts.

## Phases

### Phase 0: Outline & Research

1. **Extract unknowns from Technical Context** above:
   - For each NEEDS CLARIFICATION → research task
   - For each dependency → best practices task
   - For each integration → patterns task

2. **Generate and dispatch research agents**:

   ```text
   For each unknown in Technical Context:
     Task: "Research {unknown} for {feature context}"
   For each technology choice:
     Task: "Find best practices for {tech} in {domain}"
   ```

3. **Consolidate findings** in `research.md` using format:
   - Decision: [what was chosen]
   - Rationale: [why chosen]
   - Alternatives considered: [what else evaluated]

**Output**: research.md with all NEEDS CLARIFICATION resolved

### Phase 1: Design & Contracts

**Prerequisites:** `research.md` complete

1. **Extract entities from feature spec** → `data-model.md`:
   - Entity name, fields, relationships
   - Validation rules from requirements
   - State transitions if applicable

2. **Generate API contracts** from functional requirements:
   - For each user action → endpoint
   - Use standard REST/GraphQL patterns
   - Output OpenAPI/GraphQL schema to `/contracts/`

3. **Agent context update**:
   - Run `.specify/scripts/powershell/update-agent-context.ps1 -AgentType cursor-agent`
   - These scripts detect which AI agent is in use
   - Update the appropriate agent-specific context file
   - Add only new technology from current plan
   - Preserve manual additions between markers

**Output**: data-model.md, /contracts/*, quickstart.md, agent-specific file

## Key rules

- Use absolute paths
- ERROR on gate failures or unresolved clarifications
- **实现范围**：仅实现 **ABI 文件**中列出的符号与能力；不实现 ABI 未声明的对外接口。设计时可参考 **Unity、Unreal** 的模块与 API 构造。**若对现存 ABI 有新增或变更**，则计划结束时产出一份「契约更新」，并同步到 ABI 文件（完整 ABI 条目）与 public-api，见 `docs/agent-workflow-complete-guide.md`「2.0 写回契约」。
- **生成/构建工程**：若用户或后续步骤会「生成工程」或执行 cmake，**不得**在未澄清前直接执行 cmake。须先向用户确认：**构建方式**（各依赖 源码/DLL/不引入）、**根目录**（在哪个模块目录执行构建）。澄清规则见 `docs/engine-build-module-convention.md` §1.1。plan.md 中须写明依赖引入方式，以便 tasks/implement 使用。