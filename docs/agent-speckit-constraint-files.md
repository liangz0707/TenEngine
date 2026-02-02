# Spec Kit 指令引用的约束文件一览

本文档整理 **所有 Spec Kit（/speckit.*）指令** 所引用的 **约束/规约/契约/模板文件**，便于核对与维护。路径以仓库根为基准。

---

## 一、按指令汇总

| 指令 | 引用的约束/上下文文件 | 说明 |
|------|----------------------|------|
| **/speckit.specify** | `.specify/templates/spec-template.md`<br>`.specify/scripts/powershell/create-new-feature.ps1`<br>`.specify/scripts/powershell/check-prerequisites.ps1`<br>规格目录约定：`specs/[0-9]+-<short-name>` | 用 spec 模板生成规格；脚本产出 FEATURE_DIR、SPEC_FILE；不直接引用契约/规约，由工作流提示词在外部指定。 |
| **/speckit.clarify** | `.specify/scripts/powershell/check-prerequisites.ps1` → FEATURE_SPEC、FEATURE_DIR、IMPL_PLAN、TASKS | 以当前 feature 的 spec 为输入；不直接引用 constitution/契约，由工作流提示词在外部指定规约与契约路径。 |
| **/speckit.plan** | **FEATURE_SPEC**（由 setup-plan/check-prerequisites 产出）<br>**.specify/memory/constitution.md**<br>**IMPL_PLAN 模板**（setup-plan 已复制）<br>`.specify/templates/plan-template.md`<br>`specs/_contracts/NNN-modulename-ABI.md`<br>`specs/_contracts/README.md`<br>`docs/engine-build-module-convention.md`<br>`docs/agent-workflow-complete-guide.md`（2.0 写回契约）<br>`docs/third_party-integration-workflow.md`<br>`docs/third_party/<id>-<name>.md` | 实现范围以 ABI 与 constitution §VI 为准；构建规约见 engine-build-module-convention；写回契约见 agent-workflow-complete-guide；第三方依赖须填「第三方依赖」小节，引用 docs/third_party。 |
| **/speckit.tasks** | FEATURE_DIR 内 plan.md、spec 等（由 check-prerequisites 产出）<br>`docs/engine-build-module-convention.md` §3（构建方式澄清）<br>`docs/third_party-integration-workflow.md`<br>`docs/third_party/<id>-<name>.md` | 任务生成时若有构建/CMake，须已澄清根目录；各子模块均使用源码方式；第三方依赖须生成 7 步任务（版本选择、自动下载、配置、安装、编译测试、部署、配置实现），引入方式从 docs/third_party 读取。 |
| **/speckit.implement** | `.specify/scripts/powershell/check-prerequisites.ps1`（-RequireTasks -IncludeTasks）→ FEATURE_DIR、tasks.md、plan.md、data-model.md、contracts/、research.md、quickstart.md<br>FEATURE_DIR/checklists/*.md<br>`docs/engine-build-module-convention.md` §3（构建方式澄清）<br>plan.md「依赖引入方式」「第三方依赖」<br>`docs/third_party-integration-workflow.md`<br>`docs/third_party/<id>-<name>.md` | 执行前澄清构建根目录与平台；各子模块均使用源码方式；执行第三方任务时须按 workflow 的 7 步与引入方式完成，自动下载为必须，禁止假设已存在。 |
| **/speckit.analyze** | `.specify/scripts/powershell/check-prerequisites.ps1`（-RequireTasks -IncludeTasks）→ FEATURE_DIR、spec.md、plan.md、tasks.md<br>**.specify/memory/constitution.md** | 只读分析；Constitution 为最高权威，违反即 CRITICAL。 |
| **/speckit.constitution** | **.specify/memory/constitution.md**<br>`.specify/templates/plan-template.md`<br>`.specify/templates/spec-template.md`<br>`.specify/templates/tasks-template.md`<br>`.cursor/commands/speckit.*.md`（命令定义）<br>README.md、docs/quickstart.md 或 agent 指导文档 | 更新 constitution 后须与各模板及指导文档保持一致。 |
| **/speckit.checklist** | `.specify/scripts/powershell/check-prerequisites.ps1` → FEATURE_DIR、AVAILABLE_DOCS<br>FEATURE_DIR 内：spec.md、plan.md、tasks.md<br>`.specify/templates/checklist-template.md` | 根据 feature 上下文生成需求质量检查清单。 |
| **/speckit.taskstoissues** | `.specify/scripts/powershell/check-prerequisites.ps1`（-RequireTasks -IncludeTasks）→ FEATURE_DIR、tasks 路径 | 将 tasks 转为 GitHub issues，无额外约束文件。 |

---

## 二、约束文件清单（去重）

### 2.1 全局约束（Constitution / 契约 / 规约）

| 文件 | 用途 | 被引用指令 |
|------|------|------------|
| `.specify/memory/constitution.md` | 项目宪法：模块边界、契约策略、实现只使用契约声明类型与接口等 | plan, analyze, constitution |
| `specs/_contracts/README.md` | 契约目录说明、契约更新流程、ABI 与 TODO 约定 | plan（及工作流提示词） |
| `specs/_contracts/NNN-modulename-public-api.md` | 本模块契约（能力、类型、变更记录） | 工作流中 specify/clarify/plan/tasks/implement/analyze 的提示词 |
| `specs/_contracts/NNN-modulename-ABI.md` | 本模块 ABI（接口符号与签名权威来源） | plan, tasks, implement, 写回契约步骤 |
| `specs/_contracts/000-module-dependency-map.md` | T0 模块依赖表 | 工作流 0.2、依赖解析（见 agent-workflow-complete-guide、engine-build-module-convention） |
| `specs/_contracts/000-module-ABI.md` | ABI 总索引 | 工作流 0.0、契约说明 |
| `docs/module-specs/NNN-modulename.md` | 模块规约 | 工作流中 specify/clarify/plan 的提示词 |
| `docs/engine-build-module-convention.md` | CMake 构建规约、依赖方式、澄清要求（**构建方式澄清为 §3**；该文档无 §1.1） | plan, tasks, implement |
| `docs/agent-build-guide.md` | 子模块构建与测试、强制规则、提示词 | 构建相关（与 engine-build-module-convention 一致） |
| `docs/agent-workflow-complete-guide.md` | 完整工作流、写回契约（2.0）、全量生成增量保存模式（plan 生成全量 ABI 内容用于实现，但文档中只保存新增/修改，写回也仅写入该部分）、步骤提示词 | plan, 步骤 4 写回契约 |
| `docs/third_party-integration-workflow.md` | 第三方库集成工作流：引入方式、Plan/Task 自动纳入、7 步必须步骤、CMake 对应、自动识别 | plan, tasks, implement |
| `docs/agent-interface-sync.md` | 多 Agent 接口同步、T0 契约策略 | constitution 步骤 0.1 等 |

### 2.2 模板与脚本（.specify）

| 文件 | 用途 | 被引用指令 |
|------|------|------------|
| `.specify/templates/spec-template.md` | 规格文档结构 | specify, constitution |
| `.specify/templates/plan-template.md` | 计划文档结构、Constitution Check、依赖引入方式 | plan, constitution |
| `.specify/templates/tasks-template.md` | 任务列表结构 | constitution |
| `.specify/templates/checklist-template.md` | 检查清单结构 | checklist |
| `.specify/scripts/powershell/check-prerequisites.ps1` | 产出 FEATURE_DIR、FEATURE_SPEC、IMPL_PLAN、TASKS、AVAILABLE_DOCS | clarify, plan（setup-plan）, tasks, implement, analyze, checklist, taskstoissues |
| `.specify/scripts/powershell/setup-plan.ps1` | 产出 FEATURE_SPEC、IMPL_PLAN、SPECS_DIR、BRANCH | plan |
| `.specify/scripts/powershell/create-new-feature.ps1` | 创建 feature 分支与目录、初始化 spec | specify |

### 2.3 Feature 内产物（由脚本路径推导）

| 路径模式 | 用途 | 被引用指令 |
|----------|------|------------|
| `specs/NNN-modulename-[feature]/spec.md` | 本 feature 规格 | specify（写出）, clarify（读/写）, plan（读）, analyze（读）, checklist（读） |
| `specs/NNN-modulename-[feature]/plan.md` | 本 feature 计划 | plan（写）, tasks（读）, implement（读）, 写回契约（读）, analyze（读）, checklist（读） |
| `specs/NNN-modulename-[feature]/tasks.md` | 本 feature 任务 | tasks（写）, implement（读）, analyze（读）, checklist（读）, taskstoissues（读） |
| `specs/NNN-modulename-[feature]/checklists/*.md` | 需求质量检查清单 | specify（生成 requirements.md）, implement（检查通过再执行）, checklist（写出） |
| `specs/NNN-modulename-[feature]/data-model.md`、`contracts/`、`research.md`、`quickstart.md` | 计划阶段产物 | implement（可选读） |

---

## 三、工作流提示词中的约束（未写在命令内）

以下文件未在 `.cursor/commands/*.md` 内硬编码，而是在 **docs/agent-workflow-complete-guide.md** 的步骤提示词中要求 AI 读取，因此也视为 Spec Kit 工作流的约束来源：

- `docs/module-specs/NNN-modulename.md`（规约）
- `specs/_contracts/NNN-modulename-public-api.md`（契约）
- `specs/_contracts/NNN-modulename-ABI.md`（ABI）
- 上游模块的 `specs/_contracts/*-public-api.md` 与 `*-ABI.md`（0.2、步骤 1～3）
- `specs/_contracts/000-module-dependency-map.md`（依赖图）

---

## 四、维护建议

1. **修改 Constitution**：运行 `/speckit.constitution` 后，按 Sync Impact Report 检查并同步 `.specify/templates/` 下 plan、spec、tasks 模板及 `docs/agent-workflow-complete-guide.md`、`docs/agent-interface-sync.md` 中相关表述。  
2. **新增/重命名契约或规约**：在 `docs/agent-workflow-complete-guide.md` 与本文档中更新路径与说明。  
3. **新增 Spec Kit 指令**：在本文档「一、按指令汇总」与「二、约束文件清单」中补充该指令引用的所有约束文件。
