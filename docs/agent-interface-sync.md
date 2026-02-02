# 多 Agent 协作：接口同步策略（T0 架构）

本文档说明在多个 Agent 协同开发 TenEngine **T0 架构（27 模块）** 时，如何**定义、引用和同步**跨模块接口，避免接口不一致导致的集成失败。

---

## 1. 原则

- **主分支仅用于配置**：**master**（或 main）分支仅用于仓库与工程配置（如构建、CI、文档索引等）；**契约内容不以主分支为来源**，各模块开发与接口同步不依赖主分支上的 `specs/_contracts/`。
- **契约必须从 T0-contracts 拉取**：契约的**唯一权威来源**是 **`T0-contracts`** 分支。所有 T0-* 模块分支的 Agent 必须且只能从 **`origin/T0-contracts`** 拉取契约（`git pull origin T0-contracts`），不得以主分支上的 `_contracts/` 作为契约依据。
- **单一事实来源**：跨模块的“谁提供什么、谁消费什么”写在 `specs/_contracts/` 的契约文件中（以 **T0-contracts** 分支为准）；各模块的规格引用契约而非重复描述接口细节。
- **契约先行**：下游模块（消费者）依赖的接口，由上游模块（提供方）的契约定义；Agent 在实现或改规格前先读契约（从 T0-contracts 拉取后的本地 `_contracts/`）。
- **变更即同步**：任何 Agent 修改某模块的**对外接口**时，必须更新对应契约与 ABI 文件（并合并到 `T0-contracts` 分支）；下游通过拉取 T0-contracts 与契约/ABI 变更记录获知变化（Follow-up 与 Issue 的具体操作已废弃）。

---

## 2. T0 约束分支（T0-contracts）

- **分支名**：`T0-contracts`（建议远程：`origin/T0-contracts`）
- **用途**：专门维护 `specs/_contracts/` 下的契约文件、`docs/engine-modules-and-architecture.md`（完整模块与依赖图）及本文档；作为 **T0 架构下契约的单一发布源**。
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
| `docs/engine-modules-and-architecture.md` | 完整模块与依赖图（Mermaid、矩阵、边列表）；可在 T0-contracts 或各 T0-* 分支上查阅。 |
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

1. **须在 ABI 文件中增补或替换对应 ABI 条目**：在 **`T0-contracts` 分支**上，**必须**在对应 **ABI 文件**（`specs/_contracts/NNN-modulename-ABI.md`）中**增补**新行或**按符号/头文件匹配替换**已有行；每行须为完整 ABI 表行（模块名、命名空间、类名、接口说明、头文件、符号、说明/完整函数签名）。不得仅在 `NNN-modulename-public-api.md` 中描述而 ABI 表缺失或不全。plan 只产出新增/修改部分，写回时也仅写入该部分。接口符号与签名的权威来源是 ABI 文件。
2. 同步更新本模块对应的契约文件（`NNN-modulename-public-api.md`）：能力/类型与变更记录；在契约中注明**版本或变更说明**（与 Constitution 的版本/ABI 要求一致）。
3. **下游所需接口在上游 ABI 中以 TODO 登记**：若**下游模块**需要某接口而本模块（上游）尚未提供，须在**本模块的 ABI 文件**中增加该接口的 **TODO** 条目（在说明列标明「TODO：下游 NNN-xxx 需要」及拟议签名），待实现时转为正式 ABI 行并移除 TODO。下游不得长期依赖未在上游 ABI 中登记（含 TODO）的接口。
4. 在 `000-module-dependency-map.md` 中确认**依赖本模块**的下游列表。下游通过**拉取 T0-contracts** 与查阅上游 ABI/契约的**变更记录**获知变化；不再使用规格待办、checklist 或 GitHub Issue 的 follow-up 流程（已废弃）。

**若使用 Spec Kit /speckit.plan**：plan **只产出新增/修改的 ABI 条目**，写回时**仅在** ABI 文件中**增补或替换**这些条目，不覆盖整个 ABI 表。在 plan 完成后、tasks 之前，将 plan 产出的「契约更新」清单（仅新增/修改部分）同步到 `specs/_contracts/NNN-modulename-ABI.md` 与 `specs/_contracts/NNN-*-public-api.md` 的能力/类型，在 T0-contracts 上提交并推送；再在 worktree 拉取 T0-contracts 后继续 tasks/implement。详见 `docs/agent-workflow-complete-guide.md`「2.0 写回契约」。

### 4.3 评审/合并前

1. 检查：本 PR 是否改动某模块的**公开 API 或跨模块数据结构**？
2. 若有，确认 `specs/_contracts/` 下对应契约与 ABI 已更新并推送，且依赖图（`000-module-dependency-map.md`）已正确。下游通过拉取 T0-contracts 与契约/ABI 变更记录获知变化（**Follow-up 与 Issue 的具体操作已废弃**，不再使用规格待办、checklist 或 GitHub Issue 通知下游）。

### 4.4 接口如何确定（契约 + ABI 文件 → 真实 API）

契约描述**能力列表**与**类型/句柄**（如 IDevice、ICommandList、分配器、数学类型等）及**调用顺序与约束**；**ABI 文件**（`NNN-modulename-ABI.md`）列出**命名空间、头文件、符号**，为下游 include 与 link 的唯一依据。真实 API 在模块头文件中实现；真实 API 变更时需同步更新契约与 ABI 文件。

| 层级 | 内容 | 作用 |
|------|------|------|
| **契约** | 能力列表、类型与句柄、调用顺序与约束 | 意图与边界 |
| **ABI 文件** | 模块名、命名空间、类名、接口说明、头文件、符号、说明 | 下游 include/link 依据 |
| **真实 API（头文件）** | 可编译的完整声明 | 正式交付 |

下游以 **ABI 文件** 为准确定命名空间与头文件；上游已实现时直接 include 真实头文件并按 ABI 符号调用。

### 4.5 Agent 何时、如何发现依赖接口变化

| 时机 | 操作 | 说明 |
|------|------|------|
| **开始工作前** | `git pull origin T0-contracts`，阅读本模块依赖的契约及**变更记录** | 确保本地契约最新；若有变更，根据变更记录与上游 ABI 判断是否影响本模块。 |
| **每个 task / 每次会话开始前** | 再次 `git pull origin T0-contracts`（可选但推荐） | 若任务执行较长时间，上游可能已更新契约；任务开始前拉取可减少“做到一半才发现接口变了”的情况。 |

**总结**：Agent **不会自动**发现“拉取之后”发生的契约变更；依赖**拉取时机**（工作前、任务前）与**契约/ABI 变更记录**获知变化。**Follow-up 与 Issue 的具体操作已废弃**，不再通过规格待办、checklist 或 GitHub Issue 通知下游。务必**按任务粒度或会话开始时拉取 T0-contracts**，并查阅上游契约与 ABI 的变更说明后做适配。

---

## 5. 契约文件怎么写

- **文件名**：统一为 `NNN-modulename-public-api.md`（如 `001-core-public-api.md`、`008-rhi-public-api.md`）；边界契约为 `pipeline-to-rci.md`。
- **ABI 先行**：模块对外接口的**权威来源**是 **ABI 文件**（`NNN-modulename-ABI.md`）。契约更新时**必须**在 ABI 文件中**增补或替换**对应的 ABI 条目（每行含符号与说明/完整函数签名）；plan 只产出新增/修改部分，写回时也仅写入该部分。若下游需要某接口而上游尚未实现，须在**上游模块的 ABI 文件**中增加该接口的 **TODO** 条目，待实现时转为正式行。
- **建议结构**（可随需要增删）：
  - **适用模块**：本契约由哪一模块（如 001-Core）实现并负责。
  - **消费者**：哪些模块依赖本契约（与 `000-module-dependency-map.md` 一致）。
  - **版本/ABI**：当前契约对应的版本或 ABI 承诺。
  - **类型与句柄**：跨边界使用的关键类型、句柄、枚举（名称、语义、生命周期）。
  - **接口/能力列表**：提供方保证提供的能力；用自然语言或**伪代码**简述意图（如 `Allocator.Alloc(size, alignment) → ptr or null`）。
  - **ABI 引用**：契约引用本模块对应的 `NNN-modulename-ABI.md`；**接口符号与签名以 ABI 文件为准**。真实 API 或契约变更时**须在 ABI 文件中增补或替换对应的 ABI 条目**，不得仅改 public-api。plan 只产出新增/修改部分，写回时也仅写入该部分。
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
- **接口如何确定**：契约（能力与类型）+ **ABI 文件**（命名空间、头文件、符号、完整签名）→ **真实 API**（头文件）；详见 **4.4**。下游以 ABI 文件为准。**契约更新时须在 ABI 文件中增补或替换对应的 ABI 条目**；plan 只产出新增/修改部分，写回时也仅写入该部分。下游所需接口须在上游 ABI 中以 TODO 登记。
- **Agent 何时发现依赖接口变化**：工作前、**每个 task 开始前**拉取 T0-contracts，并查阅契约/ABI 变更记录后做适配。**Follow-up 与 Issue 的具体操作已废弃**。详见 **4.5**。
- 依赖图 `000-module-dependency-map.md` 与 `docs/engine-modules-and-architecture.md` 用于快速查“谁依赖谁”和“改了这个会影响谁”。

**旧版说明**：原 `contracts` 分支与 001–006 旧 spec 对应；现以 **T0-contracts** 与 **T0-001-core … T0-027-xr** 为最新架构，旧分支与旧 spec 已弃用。
