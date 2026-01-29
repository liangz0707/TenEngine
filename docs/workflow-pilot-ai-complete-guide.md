# 双模块试点：AI 完整执行指南

本文档提供两件事：**规约中需先补充的内容**（含由 AI 执行补充的提示词）、**完整提示词流程**（从规约预补到阶段 0/1/2，每步可复制粘贴给 AI）。  
执行前请确认主仓库为 `G:\AIHUMAN\WorkSpaceSDD\TenEngine`，worktree 为 `TenEngine-NNN-modulename`（如 `TenEngine-001-core`、`TenEngine-002-object`）。

> **文档资产与复现代码**：若要将规约/契约等文档作为资产、随时从文档生成或复现代码，见 **[docs/docs-as-assets-codegen.md](docs-as-assets-codegen.md)**。

---

## 〇、模块自识别约定（通用提示词依据）

以下约定用于**统一提示词**：AI 或用户只需在**当前 worktree**（如 `TenEngine-001-core`）下操作，即可**自动推断**对应的规约、契约、分支与 feature，无需在提示词里写死 `001`、`002` 等。

| 来源 | 推断结果 | 用途 |
|------|----------|------|
| **当前 worktree 目录名** | `NNN-modulename` | 如 `TenEngine-001-core` → `001-core`；`TenEngine-002-object` → `002-object`。 |
| **模块规约** | `docs/module-specs/NNN-modulename.md` | 规约路径。 |
| **本模块契约** | `specs/_contracts/NNN-modulename-public-api.md` | 契约路径。 |
| **T0 模块分支** | `T0-NNN-modulename` | 如 `T0-001-core`、`T0-002-object`。 |
| **Feature 分支/目录** | `NNN-modulename-<feature>`、`specs/NNN-modulename-<feature>/` | `<feature>` 取值：`abstract/fullversion/minimal/[featurename]`（默认 minimal）。 |

**下游模块**（有上游依赖时）：上游契约从 `specs/_contracts/000-module-dependency-map.md` 与 `docs/module-specs/NNN-modulename.md` 的 **Dependencies** 解析；实现时**只使用**这些上游契约中已声明的类型与接口。

**本切片范围**：规约、契约可统一推断；**本 feature 实现哪几项能力**（如 Alloc/Free、Log）需按模块指定。提示词中用占位符 **`<本切片范围>`** 表示，使用时替换为具体枚举（如 `内存 Alloc/Free、分级 Log`）。参见下方 **「阶段 1 / 2 示例切片」**。

---

## 一、规约中需先补充的内容

以下内容**必须已就绪**，AI 才能按流程正确执行。若某项缺失，可使用本段给出的「由 AI 补充」提示词先补全。

| 项目 | 位置 | 要求 | 若缺失：给 AI 的提示词 |
|------|------|------|------------------------|
| **Constitution** | `.specify/memory/constitution.md` | 含：模块边界以 specs/_contracts 契约为准、多 Agent 从 T0-contracts 拉取契约、实现只使用契约中声明的类型与接口 | 见下方「步骤 0.1」 |
| **模块规约** | `docs/module-specs/NNN-modulename.md` | 已有完整模块说明、功能、子模块、上下游；**NNN-modulename** 由 worktree 名推断 | 当前仓库已有；若缺失需先按 `docs/module-specs/README.md` 编写 |
| **契约结构** | `specs/_contracts/NNN-modulename-public-api.md` | 至少含：适用模块、消费者、版本/ABI、**能力列表**、**类型与句柄**、调用顺序与约束、变更记录；**「API 雏形」小节**可先为空 | 见下方「步骤 0.0」 |
| **依赖图** | `specs/_contracts/000-module-dependency-map.md` | 已有 T0 模块依赖表 | 当前仓库已有 |
| **工作流说明** | `docs/workflow-two-modules-pilot.md` | 已有，供 AI 查阅 §4.2.5 写回契约等 | 当前仓库已有 |
| **分支与 worktree** | 主仓库 + worktree | 主仓库存在 T0-contracts 并已推送；**TenEngine-NNN-modulename** worktree 已创建并对应 **T0-NNN-modulename** | 需人工或脚本创建 worktree |

**API 雏形小节模板**（若契约中尚无，可由 AI 或你手动画入）：

```markdown
## API 雏形（简化声明）

（由 plan 产出后写回，或先手写最小声明如 Alloc/Free、Log。）
```

保存后在主仓库 **T0-contracts** 上提交并推送，再执行阶段 0～2。

---

## 二、完整提示词流程（按顺序复制粘贴）

### 步骤 0.0：规约与契约预补（可选，建议先做）

**执行位置**：主仓库 `G:\AIHUMAN\WorkSpaceSDD\TenEngine`，分支 **T0-contracts**（若当前在 master，先 `git checkout T0-contracts` 并拉取）。

**给 AI 的提示词（通用）**：

```
当前在主仓库 G:\AIHUMAN\WorkSpaceSDD\TenEngine，请切换到 T0-contracts 分支并拉取最新。

检查 specs/_contracts/ 下你即将使用的模块对应契约（文件名形如 NNN-modulename-public-api.md；模块由 worktree 名 TenEngine-NNN-modulename 推断）：
- 若某契约没有「API 雏形」或「API 雏形（简化声明）」小节，则在「能力列表」与「调用顺序与约束」之间插入：

  ## API 雏形（简化声明）

  （由 plan 产出后写回，或先手写最小声明如 Alloc/Free、Log。）

保存后提交并推送到 origin T0-contracts，提交信息英文，例如：contract(NNN-modulename): add empty API sketch section for pilot（可列出多个模块）。
```

---

### 步骤 0.1：主仓库 Constitution（若尚未含 T0 契约策略）

**执行位置**：主仓库 `G:\AIHUMAN\WorkSpaceSDD\TenEngine`（任意分支）。

**给 AI 的提示词**：

```
请执行 /speckit.constitution，创建或更新本引擎项目的原则。要求包含：代码质量与测试约定、ABI 与模块边界以 specs/_contracts 契约为准、多 Agent 协作时契约必须从 T0-contracts 拉取、实现时只使用契约中已声明的类型与接口。完成后确认 .specify/memory/constitution.md 已存在并符合 docs/multi-agent-interface-sync.md 中的 T0 契约策略。
```

---

### 步骤 0.2：worktree 拉取契约（通用）

**执行位置**：在**当前**或**将要使用的** `TenEngine-NNN-modulename` worktree 下执行（可对多个 worktree 依次执行）。

**给 AI 的提示词**：

```
请对当前工作目录对应的 worktree 执行：git fetch origin T0-contracts，然后 git merge origin/T0-contracts。

若当前即在 G:\AIHUMAN\WorkSpaceSDD\TenEngine-NNN-modulename 下，直接在该目录执行即可。若要对多个 worktree 拉取契约，请依次 cd 到每个 TenEngine-NNN-modulename 并执行上述命令。

若有冲突，对 specs/_contracts/ 和 docs 下的文件采纳 origin/T0-contracts 的版本。执行后简要确认对应 specs/_contracts/NNN-modulename-public-api.md 已存在（NNN-modulename 从 worktree 名推断，如 TenEngine-001-core → 001-core）。
```

---

### 通用提示词使用说明

- **在任意 `TenEngine-NNN-modulename` 下**打开 Cursor，复制**阶段 1**（无上游依赖）或**阶段 2**（有上游依赖）的提示词即可；**无需**在提示词里写 `001`、`002` 等。
- **唯一需替换**：**`<本切片范围>`** → 本 feature 要实现的**具项枚举**（如 `内存 Alloc/Free、分级 Log`）。见下方「阶段 1 / 2 示例切片」或按规约自填。
- **规约、契约、T0 分支**由 **当前 worktree 目录名**推断（`TenEngine-NNN-modulename` → `NNN-modulename`）；feature 分支名为 `NNN-modulename-<feature>`（`abstract/fullversion/minimal/[featurename]`，默认 minimal）；上游契约由依赖图与模块规约的 Dependencies 解析。

---

### 阶段 1：通用模块流程（在 TenEngine-NNN-modulename 下执行）

以下 1.1～1.8 均在**当前 worktree**（如 `TenEngine-001-core`、`TenEngine-008-rhi` 等）下执行；步骤 1.4 写回契约时需切换到主仓库。**模块标识** `NNN-modulename` 由**当前 worktree 目录名**推断（`TenEngine-NNN-modulename` → `NNN-modulename`）；规约 `docs/module-specs/NNN-modulename.md`，契约 `specs/_contracts/NNN-modulename-public-api.md`，T0 分支 `T0-NNN-modulename`，feature 分支 `NNN-modulename-<feature>`（`abstract/fullversion/minimal/[featurename]`，默认 minimal）。

**`<本切片范围>`**：替换为**本 feature 要实现的具项枚举**（如 `内存 Alloc/Free、分级 Log`），否则易被模型过度收缩。参见下方 **「阶段 1 / 2 示例切片」**。

| 步骤 | 给 AI 的提示词（通用） |
|------|------------------------|
| **1.1 创建 feature 与 spec** | 请从当前工作目录推断模块标识 NNN-modulename（如 TenEngine-001-core → 001-core）。规约见 docs/module-specs/NNN-modulename.md，契约见 specs/_contracts/NNN-modulename-public-api.md。执行 /speckit.specify，描述：本 feature 的完整模块规约见上述规约，对外 API 契约见上述契约；本 feature 仅实现其中最小子集：**<本切片范围>**（必须显式枚举，如 Alloc/Free、Log）。spec.md 中引用规约与契约，只描述本切片范围。创建的分支名须为 NNN- 开头，后缀使用 `<feature>`（abstract/fullversion/minimal/[featurename]），如 NNN-modulename-minimal。 |
| **1.2 澄清规格（可选）** | 请从当前 worktree 推断 NNN-modulename。执行 /speckit.clarify。以 docs/module-specs/NNN-modulename.md 与 specs/_contracts/NNN-modulename-public-api.md 为准；仅针对本 feature 切片 **<本切片范围>** 中不明确处澄清。 |
| **1.3 生成计划（须产出契约更新）** | 请从当前 worktree 推断 NNN-modulename。执行 /speckit.plan。要求：技术栈 C++17、CMake；规约与契约为 docs/module-specs/NNN-modulename.md、specs/_contracts/NNN-modulename-public-api.md；仅暴露契约已声明的类型与 API；本切片实现 **<本切片范围>**。计划结束时产出一份「契约更新」：本 feature 对外暴露的函数签名与类型，格式可直接粘贴到上述契约的「API 雏形」小节；写在 plan.md 末尾或单独一节。 |
| **1.4 写回契约（必须）** | 请从当前 worktree 推断 NNN-modulename 及 feature 名（如 NNN-modulename-<feature>）。按 docs/workflow-two-modules-pilot.md §4.2.5 执行：1) 从当前 worktree 的 specs/NNN-modulename-<feature>/plan.md（及同目录 plan 产物）提取对外接口；若有「契约更新」清单则直接用。2) 在主仓库 G:\AIHUMAN\WorkSpaceSDD\TenEngine 的 T0-contracts 分支上，编辑 specs/_contracts/NNN-modulename-public-api.md，更新「API 雏形」与变更记录，提交并推送，提交信息如 contract(NNN-modulename): sync API sketch from plan NNN-modulename-<feature>。3) 回到当前 worktree，git fetch origin T0-contracts，git merge origin/T0-contracts。4) 确认契约已更新后再继续 /speckit.tasks。 |
| **1.5 生成任务列表** | 请从当前 worktree 推断 NNN-modulename。在当前 feature 分支下执行 /speckit.tasks。以 plan.md 与 specs/_contracts/NNN-modulename-public-api.md 为准，任务只暴露契约已声明的 API。 |
| **1.6 一致性分析（可选）** | 请从当前 worktree 推断 NNN-modulename。执行 /speckit.analyze，检查 specs/NNN-modulename-<feature>/ 与 docs/module-specs/NNN-modulename.md、specs/_contracts/NNN-modulename-public-api.md 的一致性。 |
| **1.7 执行实现** | 请从当前 worktree 推断 NNN-modulename。执行 /speckit.implement，按 specs/NNN-modulename-<feature>/tasks.md 实现；只暴露契约中已声明的类型与接口。提交信息使用英文。 |
| **1.8 合并回模块分支并推送** | 请从当前 worktree 推断 NNN-modulename。切换到 T0-NNN-modulename，合并 feature 分支 NNN-modulename-<feature>，提交信息英文（如 Merge feature NNN-modulename-<feature> (Spec Kit)），推送到 origin T0-NNN-modulename。 |

---

### 阶段 2：通用下游模块流程（在 TenEngine-NNN-modulename 下执行）

适用于**有上游依赖**的模块（如 002-Object 依赖 001-Core）。在**当前 worktree**（如 `TenEngine-002-object`）下执行；先拉取 T0-contracts（步骤 0.2）。**模块标识** `NNN-modulename` 由 worktree 名推断；**上游契约**从 `specs/_contracts/000-module-dependency-map.md` 与 `docs/module-specs/NNN-modulename.md` 的 **Dependencies** 解析，实现时**只使用**这些上游契约中已声明的类型与接口。

**`<本切片范围>`**：替换为本 feature 要实现的具项枚举（如 `类型注册 TypeRegistry::RegisterType、简单序列化接口`）。参见 **「阶段 1 / 2 示例切片」**。

| 步骤 | 给 AI 的提示词（通用） |
|------|------------------------|
| **2.1 拉取契约并确认上游契约** | 请从当前 worktree 推断 NNN-modulename。执行 git fetch origin T0-contracts 与 git merge origin/T0-contracts。阅读 specs/_contracts/000-module-dependency-map.md 与 docs/module-specs/NNN-modulename.md 的 Dependencies，确定上游模块；阅读各上游的 specs/_contracts/XXX-public-api.md（能力列表、类型与句柄、API 雏形），确认本模块实现只使用其中已声明的类型与接口。 |
| **2.2 创建 feature 与 spec** | 请从当前 worktree 推断 NNN-modulename 及上游契约列表。执行 /speckit.specify：本 feature 规约见 docs/module-specs/NNN-modulename.md，本模块契约见 specs/_contracts/NNN-modulename-public-api.md，依赖的上游 API 见各上游契约；本 feature 仅实现最小子集 **<本切片范围>**；实现时只使用上游契约已声明的类型与 API。spec.md 引用上述文件并只描述本切片范围。分支名须为 NNN- 开头（如 NNN-modulename-<feature>）。 |
| **2.3 澄清规格** | 请从当前 worktree 推断 NNN-modulename。执行 /speckit.clarify。以 docs/module-specs/NNN-modulename.md、本模块契约及上游契约为准；仅针对 **<本切片范围>** 中不明确处澄清。 |
| **2.4 生成计划（含契约更新）** | 请从当前 worktree 推断 NNN-modulename。执行 /speckit.plan：技术栈 C++17、CMake；规约与契约为 docs/module-specs/NNN-modulename.md、本模块与上游契约；仅使用上游契约已声明的类型与 API。计划结束时产出「契约更新」：本 feature 对外 API，格式可写入 specs/_contracts/NNN-modulename-public-api.md 的「API 雏形」，写在 plan.md 末尾或单独一节。 |
| **2.5 写回契约** | 请从当前 worktree 推断 NNN-modulename 及 feature 名。按 docs/workflow-two-modules-pilot.md §4.2.5：从 specs/NNN-modulename-<feature>/plan.md 提取对外 API；在主仓库 T0-contracts 上更新 specs/_contracts/NNN-modulename-public-api.md 的「API 雏形」与变更记录，提交并推送；回到当前 worktree 拉取 T0-contracts，再继续 /speckit.tasks。 |
| **2.6 生成任务并执行实现** | 请从当前 worktree 推断 NNN-modulename。依次执行 /speckit.tasks、（可选）/speckit.analyze、/speckit.implement。实现时只调用上游契约已声明的 API。提交信息使用英文。 |
| **2.7 合并回模块分支并推送** | 请从当前 worktree 推断 NNN-modulename。切换到 T0-NNN-modulename，合并 feature 分支 NNN-modulename-<feature>，提交信息英文，推送到 origin T0-NNN-modulename。 |

---

### 阶段 1 / 2 示例切片（替换 `<本切片范围>` 时参考）

| 模块 | 示例 `<本切片范围>` |
|------|---------------------|
| **001-core** | 内存 Alloc/Free、分级 Log |
| **002-object** | 类型注册 TypeRegistry::RegisterType、简单序列化接口 |
| **008-rhi** | 按规约能力列表选 1～2 项，如 Device 创建、CommandBuffer 分配；须显式枚举。 |

---

## 三、一次性总提示词（可选）

若希望 AI **按一整段总提示**从头执行（需 AI 支持长上下文与多步骤），可复制下面整段。**模块与 worktree** 由**当前工作目录**或你指定的 `TenEngine-NNN-modulename` 自识别；**本切片范围**按「阶段 1 / 2 示例切片」或规约填写。

```
请按 docs/workflow-pilot-ai-complete-guide.md 中的「二、完整提示词流程」顺序执行：

1. 步骤 0.0（规约与契约预补）：在主仓库 T0-contracts 上，为即将使用的模块对应契约（由 worktree 名 TenEngine-NNN-modulename 推断 NNN-modulename）补充「API 雏形」空小节（若尚无），提交并推送。
2. 步骤 0.1（Constitution）：主仓库执行 /speckit.constitution，确保含 T0 契约策略。
3. 步骤 0.2：对当前或即将使用的 TenEngine-NNN-modulename worktree 执行 git fetch origin T0-contracts 与 git merge origin/T0-contracts。
4. 阶段 1（通用模块）：在当前 worktree（TenEngine-NNN-modulename）下，按「〇、模块自识别约定」从 worktree 名推断 NNN-modulename；按「阶段 1 / 2 示例切片」确定 <本切片范围> 并替换到提示词中。依次执行 1.1～1.8（specify → clarify → plan → 写回契约 → tasks → [analyze] → implement → 合并推送）。
5. 若为下游模块：阶段 2（通用下游）。在同一 worktree 下，自识别 NNN-modulename 与上游契约；确定 <本切片范围> 后依次执行 2.1～2.7。

当前请先确认：主仓库为 G:\AIHUMAN\WorkSpaceSDD\TenEngine，worktree 为 TenEngine-NNN-modulename；规约与契约已就绪（见该文档「一、规约中需先补充的内容」）。每步若需在另一目录（如主仓库 T0-contracts）操作，请明确说明并执行。提交信息一律使用英文。
```

---

## 四、规约补充检查清单（执行前自检）

- [ ] `.specify/memory/constitution.md` 存在，且含「契约以 T0-contracts 为准」「实现只使用契约中声明的类型与接口」等表述（或执行步骤 0.1 补充）。
- [ ] `docs/module-specs/NNN-modulename.md`、`specs/_contracts/NNN-modulename-public-api.md` 存在且完整（**NNN-modulename** 由当前 worktree 名 `TenEngine-NNN-modulename` 推断，如 001-core、002-object）。
- [ ] 契约至少包含：适用模块、消费者、版本/ABI、能力列表、类型与句柄、调用顺序与约束、变更记录；并含有「API 雏形」小节（可先为空，或由步骤 0.0 补充）。
- [ ] `specs/_contracts/000-module-dependency-map.md` 存在且为 T0 依赖表。
- [ ] 主仓库已存在 **T0-contracts** 分支且已推送；当前使用的 **TenEngine-NNN-modulename** worktree 已创建并对应 **T0-NNN-modulename** 分支。

以上就绪后，将「二」中**阶段 1 或 2**的通用提示词按顺序发给 AI；**`<本切片范围>`** 替换为「阶段 1 / 2 示例切片」中的示例或按规约自填。更细的步骤说明与手动创建 spec 的方式见 `docs/workflow-pilot-ai-prompts.md`。
