# AI 完整执行指南

本文档提供**规约预置要求**与**两套工作流**：**通用模块流程**（模块首次落地 / 首个 feature）、**功能迭代流程**（同模块后续 feature 迭代）。执行前请确认主仓库为 `G:\AIHUMAN\WorkSpaceSDD\TenEngine`，worktree 为 `TenEngine-NNN-modulename`。

> **文档资产与复现代码**：见 **[agent-docs-as-assets-codegen.md](agent-docs-as-assets-codegen.md)**。

---

## 完整指令流程（一览）

| 顺序 | 指令 / 动作 | 说明 |
|------|-------------|------|
| 0 | **engine.contracts-pull** | 拉取 T0-contracts 到当前分支（specify / 每 task 前建议执行）。 |
| 1 | **speckit.specify** | 创建 feature 与 spec（第一步会拉取契约）。 |
| 2 | **speckit.clarify**（可选） | 澄清规格。 |
| 3 | **speckit.plan** | 生成 plan；产出「契约更新」清单（仅新增/修改）。 |
| 4 | **engine.contracts-writeback** | 将 plan 的 ABI/API 修改写回 T0-contracts（含清除已实现 TODO），提交并推送；再拉取到 worktree。 |
| 5 | **engine.contracts-downstream-todo**（可选） | 若写回时增加了 ## 变更记录，为下游 public-api 添加 TODO 兼容记录。 |
| 6 | **speckit.tasks** | 生成任务列表。 |
| 7 | **speckit.analyze**（可选） | 一致性分析。 |
| 8 | **speckit.implement** | 按 tasks 实现。 |
| 9 | 合并到 T0-NNN-modulename 并推送 | feature 分支合并回模块分支。 |
| — | **engine.contracts-push** | 将**当前分支**的契约相关变更（specs/_contracts、docs、cmake、.cursor、.specify）同步到 T0-contracts 并推送；非步骤 4 写回时也可用。 |

**契约相关命令**：拉取 **engine.contracts-pull**；写回（plan 后）**engine.contracts-writeback**；当前分支契约推送到 T0-contracts **engine.contracts-push**；下游 TODO **engine.contracts-downstream-todo**。约束见 `.cursor/commands/` 对应文件。

---

## 〇、模块自识别约定

AI 或用户只需在**当前 worktree**（如 `TenEngine-001-core`）下操作，即可**自动推断**规约、契约、分支与 feature，无需在提示词里写死 `001`、`002` 等。

| 来源 | 推断结果 | 用途 |
|------|----------|------|
| **当前 worktree 目录名** | `NNN-modulename` | 如 `TenEngine-001-core` → `001-core`；`TenEngine-002-object` → `002-object`。 |
| **模块规约** | `docs/module-specs/NNN-modulename.md` | 规约路径。 |
| **本模块契约** | `specs/_contracts/NNN-modulename-public-api.md` | 契约路径。 |
| **本模块 ABI** | `specs/_contracts/NNN-modulename-ABI.md` | 接口符号与签名的权威来源；与契约配套。 |
| **T0 模块分支** | `T0-NNN-modulename` | 如 `T0-001-core`、`T0-002-object`。 |
| **Feature 分支/目录** | `NNN-modulename-[feature]`、`specs/NNN-modulename-[feature]/` | 将 `[feature]` 替换为实际 feature 名（如 minimal、fullversion-001）。 |

**下游模块**（有上游依赖时）：上游契约从 `specs/_contracts/000-module-dependency-map.md` 与 `docs/module-specs/NNN-modulename.md` 的 **Dependencies** 解析；实现时**只使用**这些上游契约中已声明的类型与接口。

**`<本切片范围>`**：替换为本 feature 要实现的**具项枚举**（如 `内存 Alloc/Free、分级 Log`）。见下方 **「示例切片」**。

**规约来源**：规约直接使用 `docs/module-specs/NNN-*.md` 与 `specs/_contracts/NNN-*-public-api.md`，不再在 specify 中重写。

---

## 一、规约中需先补充的内容

以下**必须已就绪**，AI 才能按工作流正确执行。若某项缺失，可用本段给出的提示词先补全。

| 项目 | 位置 | 要求 | 若缺失 |
|------|------|------|--------|
| **Constitution** | `.specify/memory/constitution.md` | 含：模块边界以 specs/_contracts 契约为准、多 Agent 从 T0-contracts 拉取契约、实现只使用契约中声明的类型与接口 | 见步骤 0.1 |
| **模块规约** | `docs/module-specs/NNN-modulename.md` | 完整模块说明、功能、子模块、上下游；NNN-modulename 由 worktree 名推断 | 按 `docs/module-specs/README.md` 编写 |
| **契约结构** | `specs/_contracts/NNN-modulename-public-api.md` | 至少含：适用模块、消费者、版本/ABI、能力列表、类型与句柄、调用顺序与约束、变更记录；接口符号以对应 ABI 文件为准 | 见步骤 0.0 |
| **依赖图** | `specs/_contracts/000-module-dependency-map.md` | T0 模块依赖表 | 当前仓库已有 |
| **分支与 worktree** | 主仓库 + worktree | 主仓库有 T0-contracts 并已推送；**TenEngine-NNN-modulename** worktree 已创建并对应 **T0-NNN-modulename** | 需人工或脚本创建 |

**契约与 ABI**：接口符号与签名以 `NNN-modulename-ABI.md` 为准；写回见 **2.0** 与 **engine.contracts-writeback**；下游所需而上游未实现的接口在上游 ABI 中加 TODO。见 `specs/_contracts/README.md`。

---

## 二、工作流与提示词

**执行位置**：步骤 0.0、0.1 在主仓库 `G:\AIHUMAN\WorkSpaceSDD\TenEngine`；0.2 及步骤 1～8 在**当前 worktree**（`TenEngine-NNN-modulename`）。步骤 4 写回契约时需切回主仓库 T0-contracts。

**占位符**：**`<本切片范围>`** 替换为本 feature 要实现的具项枚举；**`[feature]`** 替换为实际 feature 名（如 minimal、fullversion-001）。规约、契约、分支等由 **worktree 目录名**推断。

**何时用哪种流程**：模块**首次**落地或**首个** feature → **通用模块流程**；同模块**后续** feature 迭代（契约与实现已有基础）→ **功能迭代流程**。

### 2.0 写回契约（步骤 4 必做）

**含义**：将 plan 产出的 **ABI 与 API 修改**写回 `specs/_contracts/` 下的 ABI 与 public-api 文件；若 plan 实现的是 ABI 中某条 **TODO**，写回时须**清除该 TODO**（改为正式 ABI 行）。完成后提交并推送到 T0-contracts，再在 worktree 拉取。

**何时**：plan 完成后、tasks 之前。**推荐直接运行 engine.contracts-writeback**；也可按下列步骤手动执行。

**步骤**：1) 从 `specs/<feature>/plan.md` 提取「契约更新」清单（仅新增/修改）。2) 在主仓库 T0-contracts 上：先更新 `NNN-modulename-public-api.md`（能力/类型与变更记录），再在 `NNN-modulename-ABI.md` 中增补或替换对应条目、清除已实现的 TODO 行；只提交契约相关路径后 `git push origin T0-contracts`。3) 在 worktree 执行 **engine.contracts-pull**（或 `git fetch` + `git merge origin/T0-contracts`）。4) 继续 **speckit.tasks**。5) 若增加了 ## 变更记录，可主动运行 **engine.contracts-downstream-todo**。

**ABI 约定**：plan 生成**全量** ABI 内容供实现参考，但 plan.md 只保存**新增/修改**部分用于写回；tasks/implement 基于全量内容。详见 `.cursor/commands/speckit.plan.md` 与 **2.0.1**。

**下游所需接口**：下游需要而上游未实现的接口，须在**上游** ABI 中增加 **TODO** 行（说明列写「TODO：下游 NNN-xxx 需要」及拟议签名），待实现后写回时清除。见 `specs/_contracts/README.md`。

#### 2.0.1 Plan 的 ABI 产出（全量生成、增量保存）

- **全量生成**：plan 执行时生成本 feature 需要的**全部 ABI 内容**（现有 ABI 条目 + 新增 + 修改），供 tasks/implement 使用。
- **增量保存**：plan.md 的「契约更新」小节**只保存新增/修改**的 ABI 条目，用于写回；不保存完整 ABI 表。
- **实现**：tasks 与 implement 基于**全量** ABI 内容，不得仅实现变化部分。约束见 `.cursor/commands/speckit.plan.md`。

#### 2.0.2 下游所需接口：在上游 ABI 中登记 TODO

**何时**：下游在 plan/spec/实现中发现需要**上游**尚未声明的接口。

**步骤**：在主仓库 T0-contracts 上，打开**上游**的 `specs/_contracts/<上游>-ABI.md`，在表中新增一行，说明列写「**TODO：下游 NNN-xxx 需要**」及拟议签名；提交并推送。下游拉取后可引用该 TODO；上游实现后写回时清除该 TODO。依赖关系见 `000-module-dependency-map.md`。

---

### 2.1 通用模块流程

**适用**：模块首次建立 feature；规约、契约已就绪（或经步骤 0.0 补全）。若有上游依赖，0.2 与步骤 1 中会确认上游契约并约束实现只使用其已声明类型与接口。

#### 步骤 0.0：规约与契约预补（可选，建议先做）

**位置**：主仓库，分支 **T0-contracts**（若在 master，先 `git checkout T0-contracts` 并拉取）。

**提示词**：

```
当前在主仓库 G:\AIHUMAN\WorkSpaceSDD\TenEngine，请切换到 T0-contracts 分支并拉取最新。

检查 specs/_contracts/ 下即将使用的模块对应契约（文件名形如 NNN-modulename-public-api.md；模块由 worktree 名 TenEngine-NNN-modulename 推断）：
- 若某契约尚无对应 ABI 文件或 ABI 表为空，则按 `specs/_contracts/000-module-ABI.md` 格式在 `NNN-modulename-ABI.md` 中补全表头与完整 ABI 表行（或占位行）；契约引用该 ABI 文件。若下游模块需要某接口而上游尚未实现，在上游模块的 ABI 文件中增加该接口的 TODO 行。保存后提交并推送到 origin T0-contracts，提交信息英文，例如：contract(NNN-modulename): add ABI placeholder。
```

#### 步骤 0.1：Constitution（若尚未含 T0 契约策略）

**位置**：主仓库，任意分支。

**提示词**：

```
请执行 /speckit.constitution，创建或更新本引擎项目的原则。要求包含：代码质量与测试约定、ABI 与模块边界以 specs/_contracts 契约为准、多 Agent 协作时契约必须从 T0-contracts 拉取、实现时只使用契约中已声明的类型与接口。完成后确认 .specify/memory/constitution.md 已存在并符合 docs/agent-interface-sync.md 中的 T0 契约策略。
```

#### 步骤 0.2：worktree 拉取契约（若有上游则确认上游契约）

**位置**：当前 worktree（`TenEngine-NNN-modulename`）。

**动作**：运行 **engine.contracts-pull**（或 `git fetch origin T0-contracts` + `git merge origin/T0-contracts`）。若有上游依赖，阅读 `000-module-dependency-map.md` 与 `docs/module-specs/NNN-modulename.md` 的 Dependencies，确认各上游的 public-api 与 ABI 已拉取；实现只使用其中已声明的类型与接口。

#### 步骤 1：创建 feature 与 spec

**方式 A：/speckit.specify**

**提示词**：

```
请从当前工作目录推断模块标识 NNN-modulename（如 TenEngine-001-core → 001-core）。规约见 docs/module-specs/NNN-modulename.md，契约见 specs/_contracts/NNN-modulename-public-api.md。

执行 /speckit.specify，描述：本 feature 的完整模块规约见上述规约，契约见 specs/_contracts/NNN-modulename-public-api.md；**本 feature 实现完整模块内容**。spec.md 中引用规约与契约，描述本模块范围。创建的分支名须为 NNN- 开头的 feature 分支（如 NNN-modulename-[feature]-XXX）[不使用已有分支]。

若本模块有上游依赖：在 spec 中明确依赖的上游 API 见各上游契约；实现时只使用上游契约已声明的类型与 API。
```

**方式 B：手动创建（可选）**  
不运行 /speckit.specify 时：创建并切换到 feature 分支 `NNN-modulename-[feature]`，创建目录 `specs/NNN-modulename-[feature]/`，在 `spec.md` 中写明**本 feature 实现完整模块内容**；完整规约见 `docs/module-specs/NNN-modulename.md`，契约见 `specs/_contracts/NNN-modulename-public-api.md`，实现时只暴露契约已声明的类型与接口。

#### 步骤 2：澄清规格（可选）

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。执行 /speckit.clarify。以 docs/module-specs/NNN-modulename.md 与 specs/_contracts/NNN-modulename-public-api.md 为准（若有上游，亦考虑上游契约）；仅针对本 feature 实现完整模块内容中不明确处澄清。
```

#### 步骤 3：生成计划

**动作**：执行 **speckit.plan**；规约与契约为 `docs/module-specs/NNN-modulename.md`、`specs/_contracts/NNN-modulename-public-api.md`（若有上游，加上游契约）。约束与 ABI 全量/增量约定见 `.cursor/commands/speckit.plan.md` 与本文档 **2.0.1**。

#### 步骤 4：写回契约（必须）

**动作**：运行 **engine.contracts-writeback**，或按本文档「2.0 写回契约」手动执行。完成后继续 **speckit.tasks**。

#### 步骤 5：生成任务列表

**动作**：执行 **speckit.tasks**；以 plan.md 及本模块 public-api、ABI 为输入。约束见 `.cursor/commands/speckit.tasks.md`。

#### 步骤 6：一致性分析（可选）

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。执行 /speckit.analyze，检查 specs/NNN-modulename-[feature]/ 与 docs/module-specs/NNN-modulename.md、specs/_contracts/NNN-modulename-public-api.md、specs/_contracts/NNN-modulename-ABI.md 的一致性。
```

#### 步骤 7：执行实现

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。执行 /speckit.implement，按 specs/NNN-modulename-[feature]/tasks.md 实现；只暴露契约与 ABI（specs/_contracts/NNN-modulename-ABI.md）中已声明的类型与接口，符号与签名以 ABI 为准。提交信息使用英文。
```

#### 步骤 8：合并回模块分支并推送

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。切换到 T0-NNN-modulename，合并 feature 分支 NNN-modulename-[feature]，提交信息英文（如 Merge feature NNN-modulename-[feature] (Spec Kit)），推送到 origin T0-NNN-modulename。
```

---

### 2.2 功能迭代流程

**适用**：同模块**后续** feature 迭代；本模块契约与 ABI 及已有实现已存在，在既有基础上**新增** `<本切片范围>`。0.0、0.1 可省略；0.2 必须（拉取契约，含上游/契约变更）；步骤 1～8 同通用模块流程，但提示词强调「功能迭代」「在既有契约与实现基础上扩展」。

#### 步骤 0.2：worktree 拉取契约（必须）

**动作**：同通用流程 0.2，运行 **engine.contracts-pull**；若有上游依赖，确认上游契约与 ABI 已更新，本迭代只使用其已声明的类型与接口。

#### 步骤 1：创建 feature 与 spec（功能迭代）

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。本流程为**功能迭代**：在本模块既有契约与实现基础上，新增 **<本切片范围>**。

执行 /speckit.specify，描述：规约见 docs/module-specs/NNN-modulename.md，契约见 specs/_contracts/NNN-modulename-public-api.md；**本 feature 实现完整模块内容**。spec.md 引用规约与契约，描述本模块范围。创建的分支名须为 NNN- 开头的 feature 分支（如 NNN-modulename-[feature]）。若有上游依赖，实现只使用上游契约已声明的类型与 API。
```

**可选：手动创建** — 不运行 specify 时，创建分支 `NNN-modulename-[feature]`、`specs/NNN-modulename-[feature]/spec.md`，写明本迭代 **<本切片范围>** 并引用规约与契约。

#### 步骤 2：澄清规格（可选）

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。执行 /speckit.clarify。以 docs/module-specs/NNN-modulename.md、specs/_contracts/NNN-modulename-public-api.md（及上游契约）为准；仅针对本迭代 **<本切片范围>** 中不明确处澄清。
```

#### 步骤 3：生成计划

**动作**：执行 **speckit.plan**，本迭代实现 **<本切片范围>**；规约与契约同上，约束见 `.cursor/commands/speckit.plan.md` 与 **2.0.1**。

#### 步骤 4：写回契约（必须）

**动作**：同通用流程步骤 4，运行 **engine.contracts-writeback** 或按「2.0 写回契约」执行。

#### 步骤 5～8

与 **通用模块流程** 步骤 5～8 相同；使用相同提示词。

---

### 示例切片（替换 `<本切片范围>` 时参考）

| 模块 | 示例 `<本切片范围>` |
|------|---------------------|
| **001-core** | 内存 Alloc/Free、分级 Log |
| **002-object** | 类型注册 TypeRegistry::RegisterType、简单序列化接口 |
| **008-rhi** | 按规约能力列表选 1～2 项，如 Device 创建、CommandBuffer 分配；须显式枚举。 |

---

## 三、执行前检查清单

- [ ] **一、规约中需先补充的内容** 中各项已就绪（Constitution、模块规约、契约结构、依赖图、分支与 worktree）。
- [ ] 当前 worktree 已拉取契约（**engine.contracts-pull**）；若有上游依赖，上游契约与 ABI 已含所需类型与符号。
- [ ] Spec Kit 在 **feature 分支** `NNN-modulename-[feature]` 上运行；plan/tasks/implement 只使用契约与 ABI 已声明的类型与接口；实现完成后合并回 **T0-NNN-modulename** 并推送。

就绪后按 **二、工作流** 选 **通用模块流程** 或 **功能迭代流程**，依步骤执行；**`<本切片范围>`** 见「示例切片」或规约自填。

---

## 四、阶段视角与契约变更

| 阶段 | 何处 | 动作 |
|------|------|------|
| 0 | 主仓库 / worktree | constitution（一次性）；**engine.contracts-pull** |
| 1 | TenEngine-NNN-modulename | specify → clarify(可选) → plan → **engine.contracts-writeback** → downstream-todo(可选) → tasks → analyze(可选) → implement → 合并到 T0-NNN-modulename 推送 |
| 契约变更 | T0-contracts | 修改对外 API 时在 T0-contracts 更新 ABI 与 public-api 并推送；或从当前分支用 **engine.contracts-push** 同步契约路径到 T0-contracts。下游通过拉取 T0-contracts 获知变化。见 `docs/agent-interface-sync.md`。 |

---

## 五、可选：一次执行总提示

若希望 AI 按一整段总提示从头执行，可复制：

```
请按 docs/agent-workflow-complete-guide.md「完整指令流程」与「二、工作流」顺序执行：engine.contracts-pull → specify → clarify(可选) → plan → engine.contracts-writeback → tasks → implement → 合并到 T0-NNN-modulename 推送。主仓库 G:\AIHUMAN\WorkSpaceSDD\TenEngine，worktree TenEngine-NNN-modulename。规约与契约见该文档「一、三」。<本切片范围> 替换为示例切片或规约自填。提交信息使用英文。
```
