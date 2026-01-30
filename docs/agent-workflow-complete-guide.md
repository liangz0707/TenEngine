# AI 完整执行指南

本文档提供**规约预置要求**与**两套工作流**：**通用模块流程**（模块首次落地 / 首个 feature）、**功能迭代流程**（同模块后续 feature 迭代）。每步可复制粘贴给 AI。  
执行前请确认主仓库为 `G:\AIHUMAN\WorkSpaceSDD\TenEngine`，worktree 为 `TenEngine-NNN-modulename`（如 `TenEngine-001-core`）。

> **文档资产与复现代码**：若要将规约/契约等文档作为资产、随时从文档生成或复现代码，见 **[agent-docs-as-assets-codegen.md](agent-docs-as-assets-codegen.md)**。

---

## 〇、模块自识别约定

AI 或用户只需在**当前 worktree**（如 `TenEngine-001-core`）下操作，即可**自动推断**规约、契约、分支与 feature，无需在提示词里写死 `001`、`002` 等。

| 来源 | 推断结果 | 用途 |
|------|----------|------|
| **当前 worktree 目录名** | `NNN-modulename` | 如 `TenEngine-001-core` → `001-core`；`TenEngine-002-object` → `002-object`。 |
| **模块规约** | `docs/module-specs/NNN-modulename.md` | 规约路径。 |
| **本模块契约** | `specs/_contracts/NNN-modulename-public-api.md` | 契约路径。 |
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

**契约与 ABI**：契约描述能力与类型；**接口须在 ABI 文件中以完整条目更新**。接口符号与签名写入对应 `NNN-modulename-ABI.md`（见 `specs/_contracts/000-module-ABI.md` 总索引）；若下游需要某接口而上游尚未实现，须在**上游 ABI 文件**中增加 **TODO** 条目。plan 产出后**先在 ABI 中更新完整表行**再同步契约，在主仓库 **T0-contracts** 上提交并推送。

---

## 二、工作流与提示词

**执行位置**：步骤 0.0、0.1 在主仓库 `G:\AIHUMAN\WorkSpaceSDD\TenEngine`；0.2 及步骤 1～8 在**当前 worktree**（`TenEngine-NNN-modulename`）。步骤 4 写回契约时需切回主仓库 T0-contracts。

**占位符**：**`<本切片范围>`** 替换为本 feature 要实现的具项枚举；**`[feature]`** 替换为实际 feature 名（如 minimal、fullversion-001）。规约、契约、分支等由 **worktree 目录名**推断。

**何时用哪种流程**：模块**首次**落地或**首个** feature → **通用模块流程**；同模块**后续** feature 迭代（契约与实现已有基础）→ **功能迭代流程**。

### 2.0 写回契约（步骤 4 必做）

**目的**：plan 生成的对外接口**必须**进入契约与 **ABI 文件**（完整 ABI 条目），下游与 tasks/implement 都以契约为准；接口符号与签名的权威来源是 ABI 文件。

**何时**：**/speckit.plan 完成后、/speckit.tasks 之前**。

**步骤**：

1. **从 plan 产物提取对外 API**  
   - 打开 `specs/<feature>/plan.md`（及同目录 plan 生成的 `contracts/`、data-model.md 等）。  
   - 提取本模块**对外**暴露的接口：函数签名、类型名、枚举等；若 plan 已产出「契约更新」清单，直接使用。

2. **在主仓库 T0-contracts 上更新契约与 ABI（ABI 须完整条目）**  
   - 主仓库 `git checkout T0-contracts`，`git pull origin T0-contracts`。  
   - **先在 ABI 文件中更新完整 ABI 条目**：编辑 `specs/_contracts/NNN-modulename-ABI.md`，按 plan 产出增加或修改**完整 ABI 表行**（模块名、命名空间、类名、接口说明、头文件、符号、说明/完整函数签名）；不得仅写占位或只改 public-api。  
   - 再编辑 `specs/_contracts/NNN-modulename-public-api.md`：按 plan 产出更新能力/类型与变更记录。  
   - `git add`、`git commit -m "contract(NNN-modulename): sync API sketch from plan NNN-modulename-[feature]"`、`git push origin T0-contracts`。

3. **在 worktree 拉取更新后的契约**  
   - 在当前 worktree（当前 feature 分支）：`git fetch origin T0-contracts`，`git merge origin/T0-contracts`。确认契约与 ABI 已更新。

4. **再跑 /speckit.tasks**  
   - 此后 tasks 与 implement 以**已更新契约与 ABI**为准。

**下游所需接口**：若下游模块需要某接口而本模块尚未实现，须在本模块的 **ABI 文件**中增加该接口的 **TODO** 条目（说明列标明「TODO：下游 NNN-xxx 需要」及拟议签名），待实现时转为正式 ABI 行。见 `specs/_contracts/README.md`「契约更新流程」。

**Prompt 约定**：调用 /speckit.plan 时在 prompt 末尾加：**「设计时可参考 Unity、Unreal 的模块与 API 构造。若本 feature 对现存 ABI（specs/_contracts/NNN-modulename-ABI.md）有新增或变更，则计划结束时产出一份「契约更新」：列出新增/变更的命名空间、头文件、符号与完整签名，可直接用于更新 ABI 与 public-api。」**

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

**位置**：当前或将使用的 `TenEngine-NNN-modulename` worktree（可对多个 worktree 依次执行）。

**提示词**：

```
请对当前工作目录对应的 worktree 执行：git fetch origin T0-contracts，然后 git merge origin/T0-contracts。

若当前即在 G:\AIHUMAN\WorkSpaceSDD\TenEngine-NNN-modulename 下，直接在该目录执行即可；若要对多个 worktree 拉取契约，请依次 cd 到每个 TenEngine-NNN-modulename 并执行上述命令。

若有冲突，对 specs/_contracts/ 和 docs 下的文件采纳 origin/T0-contracts 的版本。执行后简要确认对应 specs/_contracts/NNN-modulename-public-api.md 已存在（NNN-modulename 从 worktree 名推断）。

若本模块有上游依赖：阅读 specs/_contracts/000-module-dependency-map.md 与 docs/module-specs/NNN-modulename.md 的 Dependencies，确定上游模块；阅读各上游的 specs/_contracts/*-public-api.md 与对应 *-ABI.md（能力列表、类型与句柄、命名空间与符号），确认本模块实现只使用其中已声明的类型与接口。
```

#### 步骤 1：创建 feature 与 spec

**方式 A：/speckit.specify**

**提示词**：

```
请从当前工作目录推断模块标识 NNN-modulename（如 TenEngine-001-core → 001-core）。规约见 docs/module-specs/NNN-modulename.md，契约见 specs/_contracts/NNN-modulename-public-api.md。

执行 /speckit.specify，描述：本 feature 的完整模块规约见上述规约，契约见 specs/_contracts/NNN-modulename-public-api.md；**本 feature 实现完整模块内容**。spec.md 中引用规约与契约，描述本模块范围。创建的分支名须为 NNN- 开头的 feature 分支（如 NNN-modulename-[feature]）。

若本模块有上游依赖：在 spec 中明确依赖的上游 API 见各上游契约；实现时只使用上游契约已声明的类型与 API。
```

**方式 B：手动创建（可选）**  
不运行 /speckit.specify 时：创建并切换到 feature 分支 `NNN-modulename-[feature]`，创建目录 `specs/NNN-modulename-[feature]/`，在 `spec.md` 中写明**本 feature 实现完整模块内容**；完整规约见 `docs/module-specs/NNN-modulename.md`，契约见 `specs/_contracts/NNN-modulename-public-api.md`，实现时只暴露契约已声明的类型与接口。

#### 步骤 2：澄清规格（可选）

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。执行 /speckit.clarify。以 docs/module-specs/NNN-modulename.md 与 specs/_contracts/NNN-modulename-public-api.md 为准（若有上游，亦考虑上游契约）；仅针对本 feature 实现完整模块内容中不明确处澄清。
```

#### 步骤 3：生成计划（须产出契约更新）

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。执行 /speckit.plan。要求：技术栈 C++17、CMake；规约与契约为 docs/module-specs/NNN-modulename.md、specs/_contracts/NNN-modulename-public-api.md（若有上游，加上游契约）；设计时可参考 **Unity、Unreal** 的模块与 API 构造；仅暴露/使用契约已声明的类型与 API；**本 feature 实现完整模块内容**。**若本 feature 对现存 ABI**（specs/_contracts/NNN-modulename-ABI.md）**有新增或变更**，则计划结束时产出一份「契约更新」：列出新增/变更的命名空间、头文件、符号与完整签名，可直接用于更新 ABI 与 public-api；写在 plan.md 末尾或单独一节。
```

#### 步骤 4：写回契约（必须）

**提示词**：

```
请从当前 worktree 推断 NNN-modulename 及 feature 名（如 NNN-modulename-[feature]）。按本文档「2.0 写回契约」执行：1) 从 specs/NNN-modulename-[feature]/plan.md（及同目录 plan 产物）提取对外接口；若有「契约更新」清单则直接用。2) 在主仓库 G:\AIHUMAN\WorkSpaceSDD\TenEngine 的 T0-contracts 分支上**先在** specs/_contracts/NNN-modulename-ABI.md **中更新完整 ABI 表行**，再编辑 NNN-modulename-public-api.md 更新能力/类型与变更记录，提交并推送，提交信息如 contract(NNN-modulename): sync from plan NNN-modulename-[feature]。3) 回到当前 worktree，git fetch origin T0-contracts，git merge origin/T0-contracts。4) 确认契约已更新后再继续 /speckit.tasks。
```

#### 步骤 5：生成任务列表

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。在当前 feature 分支下执行 /speckit.tasks。以 plan.md 与 specs/_contracts/NNN-modulename-public-api.md 为准，任务只暴露契约已声明的 API。
```

#### 步骤 6：一致性分析（可选）

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。执行 /speckit.analyze，检查 specs/NNN-modulename-[feature]/ 与 docs/module-specs/NNN-modulename.md、specs/_contracts/NNN-modulename-public-api.md 的一致性。
```

#### 步骤 7：执行实现

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。执行 /speckit.implement，按 specs/NNN-modulename-[feature]/tasks.md 实现；只暴露契约中已声明的类型与接口。提交信息使用英文。
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

**位置**：当前 `TenEngine-NNN-modulename` worktree。

**提示词**：

```
请对当前 worktree 执行：git fetch origin T0-contracts，然后 git merge origin/T0-contracts。若有冲突，对 specs/_contracts/ 和 docs 下的文件采纳 origin/T0-contracts 的版本。确认 specs/_contracts/NNN-modulename-public-api.md 已更新（NNN-modulename 从 worktree 名推断）。

若本模块有上游依赖：阅读 specs/_contracts/000-module-dependency-map.md 与 docs/module-specs/NNN-modulename.md 的 Dependencies，确认上游契约变更；本迭代只使用上游契约已声明的类型与接口。
```

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

#### 步骤 3：生成计划（须产出契约更新）

**提示词**：

```
请从当前 worktree 推断 NNN-modulename。执行 /speckit.plan。要求：技术栈 C++17、CMake；规约与契约为 docs/module-specs/NNN-modulename.md、specs/_contracts/NNN-modulename-public-api.md（若有上游，加上游契约）；设计时可参考 **Unity、Unreal** 的模块与 API 构造；本迭代实现 **<本切片范围>**，在既有 API 基础上**扩展**；仅暴露/使用契约已声明或本次拟新增的类型与 API。**若本迭代对现存 ABI**（specs/_contracts/NNN-modulename-ABI.md）**有新增或变更**，则计划结束时产出一份「契约更新」：列出新增/变更的命名空间、头文件、符号与完整签名，可直接用于更新 ABI 与 public-api；写在 plan.md 末尾或单独一节。
```

#### 步骤 4：写回契约（必须）

**提示词**：

```
请从当前 worktree 推断 NNN-modulename 及 feature 名（如 NNN-modulename-[feature]）。按本文档「2.0 写回契约」：1) 从 specs/NNN-modulename-[feature]/plan.md 及 plan 产物提取本迭代**新增/变更**的对外接口；若有「契约更新」清单则直接用。2) 在主仓库 G:\AIHUMAN\WorkSpaceSDD\TenEngine 的 T0-contracts 分支上**先在** specs/_contracts/NNN-modulename-ABI.md **中增补/更新完整 ABI 表行**，再编辑 NNN-modulename-public-api.md **增补**能力/类型与变更记录，提交并推送，提交信息如 contract(NNN-modulename): sync from plan NNN-modulename-[feature]。3) 回到当前 worktree，git fetch origin T0-contracts，git merge origin/T0-contracts。4) 确认契约已更新后再继续 /speckit.tasks。
```

#### 步骤 5～8

与 **通用模块流程** 步骤 5～8 相同；使用相同的提示词（生成任务列表、可选分析、执行实现、合并回模块分支并推送）。

---

### 示例切片（替换 `<本切片范围>` 时参考）

| 模块 | 示例 `<本切片范围>` |
|------|---------------------|
| **001-core** | 内存 Alloc/Free、分级 Log |
| **002-object** | 类型注册 TypeRegistry::RegisterType、简单序列化接口 |
| **008-rhi** | 按规约能力列表选 1～2 项，如 Device 创建、CommandBuffer 分配；须显式枚举。 |

---

## 三、规约补充检查清单（执行前自检）

- [ ] `.specify/memory/constitution.md` 存在，且含「契约以 T0-contracts 为准」「实现只使用契约中声明的类型与接口」等表述（或执行步骤 0.1 补充）。
- [ ] `docs/module-specs/NNN-modulename.md`、`specs/_contracts/NNN-modulename-public-api.md` 存在且完整（**NNN-modulename** 由当前 worktree 名 `TenEngine-NNN-modulename` 推断，如 001-core、002-object）。
- [ ] 契约至少包含：适用模块、消费者、版本/ABI、能力列表、类型与句柄、调用顺序与约束、变更记录；对应 ABI 文件存在且契约引用该 ABI（可先占位，或由步骤 0.0 补充）。
- [ ] `specs/_contracts/000-module-dependency-map.md` 存在且为 T0 依赖表。
- [ ] 主仓库已存在 **T0-contracts** 分支且已推送；当前使用的 **TenEngine-NNN-modulename** worktree 已创建并对应 **T0-NNN-modulename** 分支。
- [ ] 当前 worktree 已执行 `git fetch origin T0-contracts` + `git merge origin/T0-contracts`；若有上游依赖，上游契约与 ABI 已含所需类型与符号。
- [ ] Spec Kit 在 **feature 分支**（`NNN-modulename-[feature]`）上运行，plan/tasks/implement 使用 `specs/NNN-modulename-[feature]/`；plan 的 prompt 中写明**仅使用契约中已声明的类型与接口**。
- [ ] 实现完成后将 feature 分支 **合并回 T0-NNN-modulename** 并推送；**修改对外 API 时**：先在 T0-contracts 更新契约并推送，再改代码或通知下游。

以上就绪后，按 **二、工作流** 选择 **通用模块流程**（首次 feature）或 **功能迭代流程**（后续迭代），依步骤 0.0～0.2、1～8 依次执行；**`<本切片范围>`** 替换为「示例切片」中的示例或按规约自填。步骤 1 可选手动创建 spec，见 **二、2.1 步骤 1 方式 B**。

---

## 四、阶段视角与契约变更

**何时何处调用**（阶段视角）：

| 阶段 | 何处 | 动作 |
|------|------|------|
| 0 | 主仓库 | /speckit.constitution（一次性）；worktree 拉取 T0-contracts |
| 规约来源 | — | **直接使用** `docs/module-specs/NNN-*.md` 与契约；spec.md 只写本切片并引用 |
| 1 契约与 ABI | 主仓库 T0-contracts | 编辑 NNN-modulename-public-api 与对应 ABI 文件（可选），推送 |
| 1 Spec Kit | TenEngine-NNN-modulename | feature + spec → clarify → plan → **写回契约（2.0）** → tasks → [analyze] → implement；合并到 T0-NNN-modulename，推送 |
| 2+ | 按需 | 契约变更时在 T0-contracts 更新，下游拉取并适配 |

**契约变更时的同步**：若某模块**修改了对外 API**，在 **T0-contracts** 上**须先在** `NNN-modulename-ABI.md` **中更新完整 ABI 条目**（表行含符号与完整函数签名），再更新 `NNN-modulename-public-api.md`（能力列表 / 类型与句柄 / 变更记录）并推送；若下游需要某接口而上游尚未实现，须在**上游模块的 ABI 文件**中增加该接口的 **TODO** 条目。在 `000-module-dependency-map.md` 确认下游。下游通过拉取 T0-contracts 与契约/ABI 变更记录获知变化（**Follow-up 与 Issue 的具体操作已废弃**，见 `docs/agent-interface-sync.md`）。

---

## 五、可选：一次执行总提示

若希望 AI **按一整段总提示**从头执行（需支持长上下文与多步骤），可复制：

```
请按 docs/agent-workflow-complete-guide.md「二、工作流」顺序执行：阶段 0（0.0～0.2）→ 通用模块流程或功能迭代流程（步骤 1～8）。主仓库为 G:\AIHUMAN\WorkSpaceSDD\TenEngine，worktree 为 TenEngine-NNN-modulename；规约与契约见该文档「一、三」。<本切片范围> 替换为示例切片或规约自填。每步若需切换目录（如主仓库 T0-contracts）请说明并执行。提交信息一律使用英文。
```
