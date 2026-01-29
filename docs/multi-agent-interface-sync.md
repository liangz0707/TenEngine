# 多 Agent 协作：接口同步策略（T0 架构）

本文档说明在多个 Agent 协同开发 TenEngine **T0 架构（27 模块）** 时，如何**定义、引用和同步**跨模块接口，避免接口不一致导致的集成失败。

---

## 1. 原则

- **主分支仅用于配置**：**master**（或 main）分支仅用于仓库与工程配置（如构建、CI、文档索引等）；**契约内容不以主分支为来源**，各模块开发与接口同步不依赖主分支上的 `specs/_contracts/`。
- **契约必须从 T0-contracts 拉取**：契约的**唯一权威来源**是 **`T0-contracts`** 分支。所有 T0-* 模块分支的 Agent 必须且只能从 **`origin/T0-contracts`** 拉取契约（`git pull origin T0-contracts`），不得以主分支上的 `_contracts/` 作为契约依据。
- **单一事实来源**：跨模块的“谁提供什么、谁消费什么”写在 `specs/_contracts/` 的契约文件中（以 **T0-contracts** 分支为准）；各模块的规格引用契约而非重复描述接口细节。
- **契约先行**：下游模块（消费者）依赖的接口，由上游模块（提供方）的契约定义；Agent 在实现或改规格前先读契约（从 T0-contracts 拉取后的本地 `_contracts/`）。
- **变更即同步**：任何 Agent 修改某模块的**对外接口**时，必须更新对应契约文件（并合并到 `T0-contracts` 分支），并检查依赖该契约的模块列表，必要时更新其规格或创建 follow-up 任务。

---

## 2. T0 约束分支（T0-contracts）

- **分支名**：`T0-contracts`（建议远程：`origin/T0-contracts`）
- **用途**：专门维护 `specs/_contracts/` 下的契约文件、`docs/dependency-graph-full.md`（完整依赖图）及本文档；作为 **T0 架构下契约的单一发布源**。
- **各 T0-* 分支工作前**：在任意 T0-NNN-modulename 分支（如 `T0-001-core`、`T0-008-rhi`）上开始工作前，必须先从 `T0-contracts` 拉取最新契约，再基于最新契约实现或改规格。

```bash
# 在本地 T0-* 模块分支上拉取最新契约（二选一即可）
git fetch origin T0-contracts
git merge origin/T0-contracts

# 或
git pull origin T0-contracts
```

- **更新契约时**：在 `T0-contracts` 分支上修改契约文件，或通过 PR 将契约变更合并到 `T0-contracts`；其他分支通过上述拉取获得更新。

---

## 3. 目录与角色（T0 架构）

| 位置 | 用途 |
|------|------|
| **分支 master / main** | **仅用于配置**：仓库配置、构建、CI、文档索引等；**不**作为契约的来源，Agent 不得从主分支拉取契约。 |
| **分支 `T0-contracts`** | **契约的唯一权威来源**；契约与全局依赖图的官方维护分支；各 T0-* 分支**必须**从此分支拉取最新契约。 |
| **分支 `T0-NNN-modulename`** | 各模块的独立分支（如 T0-001-core … T0-027-xr），仅含约束 + 该模块描述 + 全局依赖；工作前须先 `git pull origin T0-contracts`。 |
| `specs/_contracts/`（以 T0-contracts 为准） | 存放**跨模块接口契约**：API 边界、数据类型、调用顺序、版本；**仅以 T0-contracts 分支上的内容为准**。 |
| `specs/_contracts/000-module-dependency-map.md` | T0 模块依赖图：27 模块谁依赖谁、契约一览。 |
| `docs/dependency-graph-full.md` | 完整依赖图（Mermaid、矩阵、边列表）；可在 T0-contracts 或各 T0-* 分支上查阅。 |
| `docs/module-specs/` | 各模块详细规格（001-core.md … 027-xr.md）；T0-contracts 或各 T0-NNN-* 分支可含对应内容。 |
| `.specify/memory/constitution.md` | 全局原则（如 ABI 版本、模块边界）；契约不得违反宪法。 |

每个 Agent 的工作范围通常对应一个 **T0-NNN-modulename** 分支；跨模块边界的接口**以从 T0-contracts 拉取后的契约为准**。

---

## 4. 工作流

### 4.1 开发/实现前

1. **拉取最新契约**：在当前 T0-* 分支上执行 `git fetch origin T0-contracts` 后 `git merge origin/T0-contracts`（或 `git pull origin T0-contracts`），确保本地的 `specs/_contracts/` 与约束分支一致。
2. 打开本模块对应的 `docs/module-specs/NNN-modulename.md`，查看 **Dependencies** 与上下游。
3. 阅读 `specs/_contracts/000-module-dependency-map.md`，确认本模块的上游（依赖谁）和下游（被谁依赖）。
4. 对每一个**上游依赖**，阅读其契约文件（如 `specs/_contracts/001-core-public-api.md`），在实现时只使用契约中已声明的类型与接口。

### 4.2 修改本模块对外接口时

1. 在 **`T0-contracts` 分支**上更新本模块对应的契约文件（见 `_contracts/` 下以模块或边界命名的文件），或通过 PR 将契约变更合并到 `T0-contracts`。
2. 在契约中注明**版本或变更说明**（与 Constitution 的版本/ABI 要求一致）。
3. 在 `000-module-dependency-map.md` 中查看**依赖本模块**的下游列表；若接口有破坏性变更，则：
   - 在对应下游的规格或 checklist 中留下“需随 XXX 契约变更做适配”的待办，或
   - 创建明确的 follow-up 任务/issue 供负责下游模块的 Agent 处理。

**若使用 Spec Kit /speckit.plan**：plan 产出的对外接口设计（函数签名、类型）**必须写回契约**。在 plan 完成后、tasks 之前，将 plan.md 或 plan 产出的「契约更新」清单同步到 `specs/_contracts/NNN-*-public-api.md` 的「API 雏形」小节，在 T0-contracts 上提交并推送；再在 worktree 拉取 T0-contracts 后继续 tasks/implement。详见 `docs/workflow-two-modules-pilot.md` §4.2.5。

### 4.3 评审/合并前

1. 检查：本 PR 是否改动某模块的**公开 API 或跨模块数据结构**？
2. 若有，确认 `specs/_contracts/` 下对应契约已更新，且依赖图与下游说明已更新或已创建跟进任务。

### 4.4 Follow-up 与 Issue 的具体操作

当你在 4.2 中更新契约并需要通知下游时，“在规格或 checklist 中留下待办”或“创建 follow-up 任务/issue”对应下列**具体操作**（任选其一或组合使用）：

| 方式 | 位置 | 操作 | 作用 |
|------|------|------|------|
| **规格待办** | 下游模块规格 `docs/module-specs/NNN-modulename.md` | 增加 **待办 / TODO** 条文，如：`- **待办**：需随 \`008-rhi\` 契约变更做适配（契约变更日期：YYYY-MM-DD；变更摘要：…）。` | 负责该模块的 Agent 打开规格即可看到需适配的契约。 |
| **Checklist 项** | 该模块对应 feature 的 checklist（如 `specs/<feature>/checklists/requirements.md`） | 增加未勾选项，如：`- [ ] 适配 008-rhi 契约变更（见 \`specs/_contracts/008-rhi-public-api.md\` 变更记录）。` | 以 checklist 形式跟踪，便于逐项完成。 |
| **tasks.md 任务** | 该模块对应 feature 的 `tasks.md` | 按既有格式追加，如：`- [ ] T0XX Adapter 009-RenderCore to 008-rhi contract changes (see \`specs/_contracts/008-rhi-public-api.md\`)` | 任务可被 speckit 流程消费；配合 `speckit.taskstoissues` 可转为 GitHub Issue。 |
| **GitHub Issue** | 本仓库对应的 GitHub（`git config --get remote.origin.url`） | 在 GitHub 新建 Issue：标题如 `Adapter 009-RenderCore to 008-rhi contract change`；描述中附契约链接、变更摘要、需适配模块。 | 可分配、讨论、关联 PR，适合多人/多 Agent 协作。 |

**选用建议**：轻量变更可仅用**规格待办**；有 feature checklist 时可加 **Checklist 项** 或 **tasks.md 任务**；需分配、追踪或跨团队协作时用 **GitHub Issue**。

### 4.5 接口如何确定（伪代码 → API 雏形 → 真实 API）

当前契约主要描述**能力列表**与**类型/句柄**（如 IDevice、ICommandList、分配器、数学类型等），以及**调用顺序与约束**；多数尚未包含**具体函数签名**。本节明确**伪代码、API 雏形、真实 API** 三者的区别与演进流程。

#### 4.5.1 定义

| 层级 | 精确程度 | 示例 | 作用 |
|------|----------|------|------|
| **伪代码** | 低 | `Allocator.Alloc(size, alignment) → ptr or null` | 描述**意图与语义**，便于快速沟通，不可编译 |
| **API 雏形** | 中 | `void* Alloc(size_t size, size_t alignment);` | 接近真实代码的**简化声明**：函数名、参数类型、返回值；下游可据此写"桩调用" |
| **真实 API（头文件）** | 高 | 带 namespace、模板、错误处理、文档注释的完整声明 | **可编译**、正式交付 |

#### 4.5.2 各阶段职责与产物

| 阶段 | 产物 | 写在哪里 | 谁负责 | 何时 |
|------|------|----------|--------|------|
| **设计** | 能力列表 + 可选**伪代码** | 契约的「能力列表」小节或独立「伪代码」小节 | 模块负责人（人工或 Agent） | 模块实现**之前**；可在 /speckit.plan 或讨论中完成 |
| **细化** | **API 雏形**（简化声明） | 契约中增加「API 雏形」小节 | 同上 | 模块实现**开始前**或第一轮迭代完成后 |
| **实现** | **真实头文件** | 模块代码（如 `include/`） | 实现方 Agent / 人工 | 模块实现**进行中** |
| **交付/维护** | 契约「API 雏形」与真实 API 保持同步 | 契约 + 代码 | 实现方 | 每次对外 API 变更时同步更新契约 |

#### 4.5.3 下游模块基于什么？

| 上游状态 | 下游应基于 | 说明 |
|----------|-----------|------|
| 上游**尚未开始实现** | 契约中的 **API 雏形**（若无则看伪代码理解意图） | 下游可先写"桩调用"或 mock；接受后续小幅签名调整 |
| 上游**已实现** | 上游的**真实头文件**（与 API 雏形一致） | 直接 include 或按真实签名调用 |

**总结**：下游优先看 **API 雏形**（更接近可编译签名），伪代码主要用于快速理解意图。

#### 4.5.4 演进责任：谁、何时转换

| 转换 | 谁负责 | 何时 |
|------|--------|------|
| 能力列表 → **伪代码** | 模块负责人（设计阶段） | 讨论/plan 时，可选 |
| 伪代码 → **API 雏形** | 模块负责人 | 模块实现开始前完成；写进契约 |
| API 雏形 → **真实 API** | 实现方 | 模块实现进行中；真实头文件提交到模块分支 |
| 真实 API 变更 → **更新契约 API 雏形** | 实现方 | 每次对外 API 变更时同步到 T0-contracts |

#### 4.5.5 演进流程图

```
设计阶段         细化阶段               实现阶段                   交付/维护
   │                │                      │                          │
   ▼                ▼                      ▼                          ▼
能力列表 ──▶ (可选) 伪代码 ──▶ API 雏形（契约） ──▶ 真实 API（头文件） ──▶ 同步更新契约
                              ▲                          │
                              │                          │
                              └────────── 若真实 API 变更 ◀┘
```

**建议**：**L0 / L1 根模块**（001-Core、002-Object、003-Application、008-RHI 等）**契约先行**——在实现前把 API 雏形写进契约，再实现；下游实现时只依赖这些已定的接口。其他模块可按依赖顺序逐步补充契约中的 API 雏形。

### 4.6 Agent 何时、如何发现依赖接口变化

| 时机 | 操作 | 说明 |
|------|------|------|
| **开始工作前** | `git pull origin T0-contracts`，阅读本模块依赖的契约及**变更记录** | 确保本地契约最新；若有变更，根据变更记录判断是否影响本模块。 |
| **每个 task / 每次会话开始前** | 再次 `git pull origin T0-contracts`（可选但推荐） | 若任务执行较长时间，上游可能已更新契约；任务开始前拉取可减少“做到一半才发现接口变了”的情况。 |
| **收到 follow-up 时** | 先拉 T0-contracts，再打开该契约的**变更记录**，做适配 | 规格待办、checklist、GitHub Issue 中若写明「需随 XXX 契约变更做适配」，执行前**必须先**拉取最新契约，阅读该契约的变更说明，再实现适配。 |

**总结**：Agent **不会自动**发现“拉取之后”发生的契约变更；依赖**拉取时机**（工作前、任务前）与 **follow-up 通知**（规格待办、issue）来获知变化。因此务必**按任务粒度或会话开始时拉取 T0-contracts**，并在处理跟进任务时**先拉再适配**。

---

## 5. 契约文件怎么写

- **文件名**：统一为 `NNN-modulename-public-api.md`（如 `001-core-public-api.md`、`008-rhi-public-api.md`）；边界契约为 `pipeline-to-rci.md`。
- **建议结构**（可随需要增删）：
  - **适用模块**：本契约由哪一模块（如 001-Core）实现并负责。
  - **消费者**：哪些模块依赖本契约（与 `000-module-dependency-map.md` 一致）。
  - **版本/ABI**：当前契约对应的版本或 ABI 承诺。
  - **类型与句柄**：跨边界使用的关键类型、句柄、枚举（名称、语义、生命周期）。
  - **接口/能力列表**：提供方保证提供的能力；用自然语言或**伪代码**简述意图（如 `Allocator.Alloc(size, alignment) → ptr or null`）。
  - **API 雏形**（可选但推荐）：用接近真实代码的**简化声明**描述关键 API（如 `void* Alloc(size_t size, size_t alignment);`）；下游可据此写桩调用。不要求一开始写全，可随实现迭代补充。真实 API 变更时需同步更新此小节。
  - **调用顺序与约束**：如“必须先初始化再创建资源”“命令缓冲在帧末提交”。
  - **变更记录**：重要变更的日期与简要说明。

契约以**稳定、可被多 Agent 引用**为目标，避免实现细节；实现细节留在各模块的规格或代码中。

---

## 6. 在规格中引用契约

在各模块的 `docs/module-specs/NNN-modulename.md` 或对应 spec 中建议包含：

- **本模块提供/实现的契约**：例如“本模块实现契约 `specs/_contracts/001-core-public-api.md`”。
- **本模块依赖的契约**：在 **Dependencies** 或 **模块上下游** 中列出，例如“依赖 `specs/_contracts/001-core-public-api.md`、`specs/_contracts/008-rhi-public-api.md`”。

这样，任何 Agent 打开该规格时都能直接跳转到契约文件，保证实现与接口定义一致。

---

## 7. 小结

- **主分支（master/main）仅用于配置**，不作为契约来源；**契约必须从 T0-contracts 拉取**，不得以主分支上的 `_contracts/` 为依据。
- **T0-contracts** 是 T0 架构下契约的**唯一权威发布源**；各 **T0-NNN-modulename** 分支工作前**必须**从此分支拉取最新契约（`git pull origin T0-contracts`）。
- 接口的**单一事实来源**在 `specs/_contracts/`（**仅以 T0-contracts 分支上的内容为准**）。
- 每个 Agent：**工作前**从 **T0-contracts** 拉取契约，**读**自己依赖的契约，**写/改**自己负责的契约（在 **T0-contracts** 上更新），**改接口时**更新契约并通知下游。
- **接口如何确定**：**伪代码**（意图）→ **API 雏形**（简化声明）→ **真实 API**（头文件）；详见 **4.5**。L0/L1 建议**契约先行**：实现前把 API 雏形写进契约，实现严格按契约，下游基于 API 雏形或真实头文件。
- **Agent 何时发现依赖接口变化**：工作前、**每个 task 开始前**拉取 T0-contracts；收到 follow-up 时先拉再适配。详见 **4.6**。
- 依赖图 `000-module-dependency-map.md` 与 `docs/dependency-graph-full.md` 用于快速查“谁依赖谁”和“改了这个会影响谁”。

**旧版说明**：原 `contracts` 分支与 001–006 旧 spec 对应；现以 **T0-contracts** 与 **T0-001-core … T0-027-xr** 为最新架构，旧分支与旧 spec 已弃用。
