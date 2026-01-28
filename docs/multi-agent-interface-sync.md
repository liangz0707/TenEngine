# 多 Agent 协作：接口同步策略（T0 架构）

本文档说明在多个 Agent 协同开发 TenEngine **T0 架构（27 模块）** 时，如何**定义、引用和同步**跨模块接口，避免接口不一致导致的集成失败。

---

## 1. 原则

- **单一事实来源**：跨模块的“谁提供什么、谁消费什么”写在 `specs/_contracts/` 的契约文件中；各模块的规格引用契约而非重复描述接口细节。
- **T0 约束分支**：契约文件的官方维护分支为 **`T0-contracts`**；所有契约的更新都合并到该分支，各 **T0-NNN-modulename** 分支的 Agent 从该分支拉取最新契约后再开始工作。
- **契约先行**：下游模块（消费者）依赖的接口，由上游模块（提供方）的契约定义；Agent 在实现或改规格前先读契约。
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
| **分支 `T0-contracts`** | 契约与全局依赖图的官方维护分支；各 T0-* 分支从此拉取最新契约。 |
| **分支 `T0-NNN-modulename`** | 各模块的独立分支（如 T0-001-core … T0-027-xr），仅含约束 + 该模块描述 + 全局依赖。 |
| `specs/_contracts/` | 存放**跨模块接口契约**：API 边界、数据类型、调用顺序、版本。 |
| `specs/_contracts/000-module-dependency-map.md` | T0 模块依赖图：27 模块谁依赖谁、契约一览。 |
| `docs/dependency-graph-full.md` | 完整依赖图（Mermaid、矩阵、边列表）。 |
| `docs/module-specs/` | 各模块详细规格（001-core.md … 027-xr.md）；主仓库或 T0-contracts 可含全量，各 T0-NNN-* 分支仅含对应单模块。 |
| `.specify/memory/constitution.md` | 全局原则（如 ABI 版本、模块边界）；契约不得违反宪法。 |

每个 Agent 的工作范围通常对应一个 **T0-NNN-modulename** 分支；跨模块边界的接口以契约为准。

---

## 4. 工作流

### 4.1 开发/实现前

1. **拉取最新契约**：在当前 T0-* 分支上执行 `git fetch origin T0-contracts` 后 `git merge origin/T0-contracts`（或 `git pull origin T0-contracts`），确保本地的 `specs/_contracts/` 与约束分支一致。
2. 打开本模块对应的 `docs/module-specs/NNN-modulename.md`，查看 **Dependencies** 与上下游。
3. 阅读 `specs/_contracts/000-module-dependency-map.md`，确认本模块的上游（依赖谁）和下游（被谁依赖）。
4. 对每一个**上游依赖**，阅读其契约文件（如 `specs/_contracts/core-public-api.md`），在实现时只使用契约中已声明的类型与接口。

### 4.2 修改本模块对外接口时

1. 在 **`T0-contracts` 分支**上更新本模块对应的契约文件（见 `_contracts/` 下以模块或边界命名的文件），或通过 PR 将契约变更合并到 `T0-contracts`。
2. 在契约中注明**版本或变更说明**（与 Constitution 的版本/ABI 要求一致）。
3. 在 `000-module-dependency-map.md` 中查看**依赖本模块**的下游列表；若接口有破坏性变更，则：
   - 在对应下游的规格或 checklist 中留下“需随 XXX 契约变更做适配”的待办，或
   - 创建明确的 follow-up 任务/issue 供负责下游模块的 Agent 处理。

### 4.3 评审/合并前

1. 检查：本 PR 是否改动某模块的**公开 API 或跨模块数据结构**？
2. 若有，确认 `specs/_contracts/` 下对应契约已更新，且依赖图与下游说明已更新或已创建跟进任务。

---

## 5. 契约文件怎么写

- **文件名**：以模块或边界命名，如 `core-public-api.md`、`rci-public-api.md`、`pipeline-to-rci.md`；T0 架构下可扩展 `object-public-api.md`、`rhi-public-api.md` 等。
- **建议结构**（可随需要增删）：
  - **适用模块**：本契约由哪一模块（如 001-Core）实现并负责。
  - **消费者**：哪些模块依赖本契约（与 `000-module-dependency-map.md` 一致）。
  - **版本/ABI**：当前契约对应的版本或 ABI 承诺。
  - **类型与句柄**：跨边界使用的关键类型、句柄、枚举（名称、语义、生命周期）。
  - **接口/能力列表**：提供方保证提供的能力；可写伪代码或简短描述。
  - **调用顺序与约束**：如“必须先初始化再创建资源”“命令缓冲在帧末提交”。
  - **变更记录**：重要变更的日期与简要说明。

契约以**稳定、可被多 Agent 引用**为目标，避免实现细节；实现细节留在各模块的规格或代码中。

---

## 6. 在规格中引用契约

在各模块的 `docs/module-specs/NNN-modulename.md` 或对应 spec 中建议包含：

- **本模块提供/实现的契约**：例如“本模块实现契约 `specs/_contracts/core-public-api.md`”。
- **本模块依赖的契约**：在 **Dependencies** 或 **模块上下游** 中列出，例如“依赖 `specs/_contracts/core-public-api.md`、`specs/_contracts/rci-public-api.md`”。

这样，任何 Agent 打开该规格时都能直接跳转到契约文件，保证实现与接口定义一致。

---

## 7. 小结

- **T0-contracts** 是 T0 架构下契约文件的单一发布源；各 **T0-NNN-modulename** 分支工作前须从此分支拉取最新契约。
- 接口的**单一事实来源**在 `specs/_contracts/`（以 `T0-contracts` 分支上的内容为准）。
- 每个 Agent：**工作前**从 `T0-contracts` 拉取契约，**读**自己依赖的契约，**写/改**自己负责的契约（在 `T0-contracts` 上更新），**改接口时**更新契约并通知下游。
- 依赖图 `000-module-dependency-map.md` 与 `docs/dependency-graph-full.md` 用于快速查“谁依赖谁”和“改了这个会影响谁”。

**旧版说明**：原 `contracts` 分支与 001–006 旧 spec 对应；现以 **T0-contracts** 与 **T0-001-core … T0-027-xr** 为最新架构，旧分支与旧 spec 已弃用。
