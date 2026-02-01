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
   - **Build convention (TenEngine)**：若本 feature 涉及 CMake/构建，**必须**在 plan.md 中填写「依赖引入方式」小节（见 `.specify/templates/plan-template.md`）：各直接依赖均按**源码**方式引入（无外部库则可不列）；当前所有子模块构建均使用源码方式，规约见 `docs/engine-build-module-convention.md` §3（构建方式澄清）。
   - **第三方依赖 (TenEngine)**：若本 feature 涉及的模块（spec/contracts 或依赖清单）中声明了第三方库，**必须**在 plan 的「第三方依赖」小节中列出每个第三方：ID、引入方式（header-only/source/sdk/system，从 `docs/third_party/<id>-<name>.md` 读取）、文档链接；无第三方时填「本 feature 无第三方依赖」。工作流与 7 步任务见 `docs/third_party-integration-workflow.md`。
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

2. **Generate API/ABI design artifacts** from functional requirements:
   - For each user action or module boundary → interface/endpoint
   - **TenEngine**：
     - **生成全量 ABI 内容**：参考 spec、Unity/Unreal 等文档，生成本 feature 需要实现的**全部 ABI 内容**，包括：
       - **原始 ABI**：现有 `specs/_contracts/NNN-modulename-ABI.md` 中已声明的所有条目（本 feature 需要实现的）
       - **新增的 ABI**：本 feature 新增的接口条目
       - **修改的 ABI**：对现有 ABI 条目的修改
     - **文档中只保存变化部分**：plan.md 的「契约更新」小节**只保存相对于现有 ABI 的新增和修改部分**，用于查阅和写回契约；若无新增/修改则产出空清单。
     - **实现基于全量内容**：tasks 和 implement 阶段**必须基于全量 ABI 内容**（原始 + 新增 + 修改）进行实现，不得仅实现变化部分。
     - 产出至 `contracts/`（非 REST/GraphQL schema）；以 ABI 文件与契约为准。
   - 其他项目：可按 REST/GraphQL 产出 OpenAPI/GraphQL schema 至 `/contracts/`

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
- **生成/构建工程**：若用户或后续步骤会「生成工程」或执行 cmake，**不得**在未澄清前直接执行 cmake。须先向用户确认：**构建根目录**（在哪个模块目录执行构建）；各子模块均使用源码方式，无需再选 DLL/预编译。澄清规则见 `docs/engine-build-module-convention.md` §3（构建方式澄清）。plan.md 中须写明依赖引入方式，以便 tasks/implement 使用。