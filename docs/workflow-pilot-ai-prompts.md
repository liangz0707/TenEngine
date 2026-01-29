# 双模块试点：AI 可执行的完整提示词流程

本文档给出**按顺序复制粘贴给 AI 的完整提示词**，便于 AI 帮你跑完 001-Core → 002-Object 试点。执行前请确认「一、规约中需先补充的内容」已就绪。

> **一站式入口**：规约需先补充的内容 + 完整提示词流程（含规约预补步骤）已整理到 **[docs/workflow-pilot-ai-complete-guide.md](workflow-pilot-ai-complete-guide.md)**，可优先使用该文档按步骤执行。

---

## 一、规约中需先补充的内容（AI 执行前请确认）

以下内容**必须已存在**，AI 才能按提示词正确执行：

| 项目 | 位置 | 要求 |
|------|------|------|
| **Constitution** | `.specify/memory/constitution.md` | 已包含：模块边界以 specs/_contracts 契约为准、多 Agent 从 T0-contracts 拉取契约。若无则用「阶段 0 步骤 1」的提示词生成。 |
| **模块规约** | `docs/module-specs/001-core.md`、`docs/module-specs/002-object.md` | 已有完整模块说明、功能、子模块、上下游（当前仓库应已有）。 |
| **契约-能力与类型** | `specs/_contracts/001-core-public-api.md`、`002-object-public-api.md` | 至少包含：适用模块、消费者、版本/ABI、**能力列表（提供方保证）**、**类型与句柄（跨边界）**、调用顺序与约束、变更记录。「API 雏形」小节可先为空，由 plan 产出后写回。 |
| **依赖图** | `specs/_contracts/000-module-dependency-map.md` | 已有 27 模块依赖表（当前仓库应已有）。 |
| **工作流说明** | `docs/workflow-two-modules-pilot.md` | 已有，供 AI 查阅 §4.2.5（写回契约）等步骤。 |

若契约中尚无「API 雏形」小节，可先在对应契约的「能力列表」后增加空小节：

```markdown
## API 雏形（简化声明）

（由 plan 产出后写回，或先手写最小声明如 Alloc/Free、Log。）
```

保存后在主仓库 **T0-contracts** 上提交并推送，再让 AI 执行下面流程。

---

## 二、阶段 0：准备（一次性）

### 步骤 0.1 主仓库：Constitution（若尚未有）

**执行位置**：主仓库 `G:\AIHUMAN\WorkSpaceSDD\TenEngine`（任意分支）。

**给 AI 的提示词**（复制整段）：

```
请执行 /speckit.constitution，创建或更新本引擎项目的原则。要求包含：代码质量与测试约定、ABI 与模块边界以 specs/_contracts 契约为准、多 Agent 协作时契约必须从 T0-contracts 拉取、实现时只使用契约中已声明的类型与接口。完成后确认 .specify/memory/constitution.md 已存在并符合 docs/multi-agent-interface-sync.md 中的 T0 契约策略。
```

---

### 步骤 0.2 两个 worktree：拉取契约

**执行位置**：在 **TenEngine-001-core** 和 **TenEngine-002-object** 分别执行（或由 AI 依次执行）。

**给 AI 的提示词**：

```
请按顺序执行以下操作，确保两个 worktree 都拉取到最新契约：

1. 在 G:\AIHUMAN\WorkSpaceSDD\TenEngine-001-core 下执行：git fetch origin T0-contracts，然后 git merge origin/T0-contracts。
2. 在 G:\AIHUMAN\WorkSpaceSDD\TenEngine-002-object 下执行：git fetch origin T0-contracts，然后 git merge origin/T0-contracts。

若有冲突，对 specs/_contracts/ 和 docs 下的文件采纳 origin/T0-contracts 的版本。执行后简要确认两个目录下的 specs/_contracts/001-core-public-api.md 和 002-object-public-api.md 已存在。
```

---

## 三、阶段 1：001-Core（上游）

**约定**：以下步骤 1.1～1.7 均在 **TenEngine-001-core** 的**当前工作目录**下执行（除非步骤 1.5 写回契约时切换到主仓库）。先在该 worktree 打开 Cursor。

---

### 步骤 1.1 创建 feature 与 spec（二选一：Spec Kit 或手动）

**给 AI 的提示词（方式 A：用 Spec Kit）**：

```
当前在 G:\AIHUMAN\WorkSpaceSDD\TenEngine-001-core。请执行 /speckit.specify，并按以下要求填写描述：

本 feature 的完整模块规约见 docs/module-specs/001-core.md，对外 API 契约见 specs/_contracts/001-core-public-api.md。本 feature 仅实现其中最小子集：内存分配与释放（Alloc/Free）、分级日志（Log）。spec.md 中必须引用上述两文件，并只描述本切片的范围（不重复写完整模块规约）。创建的分支名须为 001- 开头的 feature 分支（如 001-core-minimal）。
```

**给 AI 的提示词（方式 B：手动创建，若不想用 specify）**：

```
当前在 G:\AIHUMAN\WorkSpaceSDD\TenEngine-001-core。请在不切换仓库的前提下：

1. 创建并切换到新分支 001-core-minimal（若已存在则跳过创建）。
2. 创建目录 specs/001-core-minimal/。
3. 在 specs/001-core-minimal/spec.md 中写入以下内容（可略作润色）：本 feature 实现 001-Core 的最小可用子集：内存 Alloc/Free、分级 Log。完整模块规约见 docs/module-specs/001-core.md，对外 API 见 specs/_contracts/001-core-public-api.md。实现时只暴露契约中已声明的类型与接口。
```

---

### 步骤 1.2 澄清规格（可选但推荐）

**给 AI 的提示词**：

```
请执行 /speckit.clarify。澄清时以 docs/module-specs/001-core.md 与 specs/_contracts/001-core-public-api.md 为准，不偏离已有规约；仅针对本 feature 切片（Alloc/Free、Log）中不明确的地方提问并记录。
```

---

### 步骤 1.3 生成计划（plan 须产出「契约更新」）

**给 AI 的提示词**：

```
请执行 /speckit.plan，并在你的 plan 描述中明确加入以下要求：

- 技术栈：C++17、CMake。
- 规约输入：docs/module-specs/001-core.md 与 specs/_contracts/001-core-public-api.md；仅暴露契约中已声明的类型与 API；本切片实现内存 Alloc/Free、分级 Log。
- 计划结束时必须产出一份「契约更新」：列出本 feature 对外暴露的所有函数签名与类型（如 void* Alloc(size_t size, size_t alignment); void Free(void* ptr); 以及 Log 相关声明），格式为可直接粘贴到 specs/_contracts/001-core-public-api.md 的「API 雏形」小节中的 Markdown。将该「契约更新」写在 plan.md 末尾或单独一节，便于下一步写回契约。
```

---

### 步骤 1.4 写回契约（必须）

**执行位置**：先在主仓库 **T0-contracts** 上更新契约并推送，再回到 **TenEngine-001-core** 拉取。

**给 AI 的提示词**：

```
请按 docs/workflow-two-modules-pilot.md 中 §4.2.5「将 plan 产出的接口写回契约」执行：

1. 从当前 worktree 的 specs/001-core-minimal/plan.md（以及同目录下 plan 可能生成的 contracts/、data-model.md 等）中，提取本 feature 对外暴露的接口：函数签名、类型名。若 plan 中已有「契约更新」清单，直接使用该清单。
2. 打开主仓库 G:\AIHUMAN\WorkSpaceSDD\TenEngine，切换到 T0-contracts 分支并拉取最新。编辑 specs/_contracts/001-core-public-api.md：在「能力列表」后补充或更新「API 雏形」小节，将上述接口以简化声明形式写入；在「变更记录」中加一行（日期 + 说明：API 雏形由 plan 001-core-minimal 同步）。提交并推送到 origin T0-contracts。提交信息使用英文，例如：contract(001-core): sync API sketch from plan 001-core-minimal
3. 回到 worktree G:\AIHUMAN\WorkSpaceSDD\TenEngine-001-core，执行 git fetch origin T0-contracts 与 git merge origin/T0-contracts，确认 specs/_contracts/001-core-public-api.md 已含刚写回的 API 雏形。
4. 简要确认：写回契约后，再继续下一步 /speckit.tasks。
```

---

### 步骤 1.5 生成任务列表

**给 AI 的提示词**：

```
请在当前 worktree（TenEngine-001-core，feature 分支 001-core-minimal）下执行 /speckit.tasks。生成 tasks.md 时以当前 plan.md 和已更新契约 specs/_contracts/001-core-public-api.md 为准，任务中只暴露契约里已声明的 API。
```

---

### 步骤 1.6 一致性分析（可选）

**给 AI 的提示词**：

```
请执行 /speckit.analyze，检查 specs/001-core-minimal/ 下的 spec、plan、tasks 与 docs/module-specs/001-core.md、specs/_contracts/001-core-public-api.md 的一致性。
```

---

### 步骤 1.7 执行实现

**给 AI 的提示词**：

```
请执行 /speckit.implement，按 specs/001-core-minimal/tasks.md 依次执行。实现时只暴露 specs/_contracts/001-core-public-api.md 中已声明的类型与接口。提交信息使用英文。
```

---

### 步骤 1.8 合并回模块分支并推送

**给 AI 的提示词**：

```
当前在 TenEngine-001-core。请：1) 切换到分支 T0-001-core；2) 将 feature 分支 001-core-minimal 合并进来（git merge 001-core-minimal），提交信息英文，例如 Merge feature 001-core-minimal (Spec Kit)；3) 推送到 origin T0-001-core。
```

---

## 四、阶段 2：002-Object（下游）

**约定**：以下在 **TenEngine-002-object** 下执行。先在该 worktree 打开 Cursor，并确保已拉取 T0-contracts（若未做，先执行步骤 0.2 中针对 TenEngine-002-object 的部分）。

---

### 步骤 2.1 拉取契约并确认 Core 契约

**给 AI 的提示词**：

```
当前在 G:\AIHUMAN\WorkSpaceSDD\TenEngine-002-object。请执行 git fetch origin T0-contracts 与 git merge origin/T0-contracts。然后阅读 specs/_contracts/001-core-public-api.md 的「能力列表」「类型与句柄」「API 雏形」，确认实现 002-Object 时只使用其中已声明的类型与接口。
```

---

### 步骤 2.2 创建 feature 与 spec（Object 最小切片）

**给 AI 的提示词（方式 A）**：

```
请执行 /speckit.specify，描述如下：

本 feature 的模块规约见 docs/module-specs/002-object.md，依赖的 Core API 见 specs/_contracts/001-core-public-api.md，本模块对外契约见 specs/_contracts/002-object-public-api.md。本 feature 仅实现最小子集：类型注册（TypeRegistry::RegisterType）、简单序列化接口。实现时只使用 001-Core 契约中已声明的类型与 API（如 Alloc/Free、String、Log）。spec.md 中引用上述文件并只描述本切片范围。分支名须为 002- 开头（如 002-object-minimal）。
```

---

### 步骤 2.3 澄清规格

**给 AI 的提示词**：

```
请执行 /speckit.clarify。以 docs/module-specs/002-object.md 与 specs/_contracts/001-core-public-api.md、002-object-public-api.md 为准，仅针对本 feature 切片（类型注册、简单序列化）中不明确处澄清。
```

---

### 步骤 2.4 生成计划（含契约更新）

**给 AI 的提示词**：

```
请执行 /speckit.plan，并明确加入：

- 技术栈：C++17、CMake。
- 规约输入：docs/module-specs/002-object.md 与 specs/_contracts/001-core-public-api.md、002-object-public-api.md。仅使用 001-Core 契约中已声明的类型与 API，不依赖契约未声明的内部实现。
- 计划结束时产出一份「契约更新」：本 feature 对外暴露的函数签名与类型，格式可直接写入 specs/_contracts/002-object-public-api.md 的「API 雏形」小节，写在 plan.md 末尾或单独一节。
```

---

### 步骤 2.5 写回契约（002-Object）

**给 AI 的提示词**：

```
请按 docs/workflow-two-modules-pilot.md §4.2.5 的流程，将 plan 产出的 002-Object 对外接口写回契约：

1. 从 specs/002-object-minimal/plan.md（及同目录下 plan 产物）提取本 feature 对外 API；若有「契约更新」清单则直接使用。
2. 在主仓库 G:\AIHUMAN\WorkSpaceSDD\TenEngine 的 T0-contracts 分支上，编辑 specs/_contracts/002-object-public-api.md，在「API 雏形」小节补充上述接口，变更记录加一行。提交并推送，提交信息英文，例如：contract(002-object): sync API sketch from plan 002-object-minimal
3. 回到 worktree TenEngine-002-object，git fetch origin T0-contracts，git merge origin/T0-contracts。确认契约已更新后再继续 /speckit.tasks。
```

---

### 步骤 2.6 生成任务并执行实现

**给 AI 的提示词**：

```
请依次执行：1) /speckit.tasks，生成 tasks.md；2) 可选 /speckit.analyze；3) /speckit.implement，按 tasks.md 实现。实现时只调用 001-Core 契约中已声明的 API。提交信息使用英文。
```

---

### 步骤 2.7 合并回模块分支并推送

**给 AI 的提示词**：

```
当前在 TenEngine-002-object。请切换到 T0-002-object，合并 feature 分支 002-object-minimal，提交信息英文（如 Merge feature 002-object-minimal (Spec Kit)），并推送到 origin T0-002-object。
```

---

## 五、流程总览（给 AI 的「一次执行」总提示，可选）

若希望 AI **按一整段总提示**从头执行，可复制下面整段（需 AI 支持长上下文与多步骤）：

```
请按 docs/workflow-pilot-ai-prompts.md 中的「二、阶段 0」→「三、阶段 1」→「四、阶段 2」顺序执行。当前请先确认：1) 主仓库为 G:\AIHUMAN\WorkSpaceSDD\TenEngine，两个 worktree 为 TenEngine-001-core、TenEngine-002-object；2) 规约与契约已就绪（见该文档「一、规约中需先补充的内容」）。然后从阶段 0 步骤 0.1 开始，每完成一步再执行下一步；若某步需在另一目录（如主仓库 T0-contracts）操作，请明确说明并执行。提交信息一律使用英文。
```

---

## 六、规约中需先补充的内容（清单式）

若你要**先**在规约/契约里补全再让 AI 跑，可按此清单逐项检查：

- [ ] `.specify/memory/constitution.md` 存在，且含「契约以 T0-contracts 为准」「实现只使用契约中声明的类型与接口」等表述。
- [ ] `docs/module-specs/001-core.md`、`docs/module-specs/002-object.md` 存在且完整（模块说明、功能、子模块、上下游）。
- [ ] `specs/_contracts/001-core-public-api.md`、`002-object-public-api.md` 至少包含：适用模块、消费者、版本/ABI、**能力列表（提供方保证）**、**类型与句柄（跨边界）**、调用顺序与约束、变更记录；可选先增加空小节 **「API 雏形（简化声明）」**，或留空由 plan 写回。
- [ ] `specs/_contracts/000-module-dependency-map.md` 存在且为 T0 依赖表。
- [ ] 主仓库已存在 **T0-contracts** 分支且已推送；两个 worktree 已创建并分别对应 **T0-001-core**、**T0-002-object** 分支。

以上就绪后，将「二～四」中对应步骤的提示词按顺序发给 AI 即可。
