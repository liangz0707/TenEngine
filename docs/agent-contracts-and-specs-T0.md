# T0 规约与契约说明

本文档是 **T0 架构（27 模块）** 下规约文件与契约文件的**快速索引**。详细的工作流与协作规范见 **`docs/agent-interface-sync.md`**。

---

## 1. 核心要点（速览）

| 要点 | 说明 |
|------|------|
| **契约唯一来源** | **`T0-contracts`** 分支（`git pull origin T0-contracts`）；主分支 `_contracts/` 仅作备份，不作为契约依据。 |
| **模块** | 001-Core … 027-XR（见 `docs/engine-modules-and-architecture.md`）。 |
| **分支** | `T0-contracts`（契约发布源）、`T0-NNN-modulename`（各模块独立分支）。 |
| **工作前** | 必须 `git pull origin T0-contracts` 拉取最新契约。 |

详细原则、目录与角色、工作流、契约写法、接口演进等 → **`docs/agent-interface-sync.md`**。

---

## 2. 规约文件

| 位置 | 内容 |
|------|------|
| `docs/engine-modules-and-architecture.md` | 完整模块与基础架构（27 模块、功能域映射、依赖表、Mermaid/矩阵/边列表、导出说明）。 |
| `docs/module-specs/` | 各模块详细规格（001-core.md … 027-xr.md），含模块说明、功能、难度、资源类型、子模块、上下游、外部依赖。 |
| `docs/research/engine-reference-unity-unreal-modules.md` | Unity/Unreal 模块划分与依赖参考、三引擎对照（调研）。 |
| `docs/research/engine-proposed-module-architecture.md` | 整合式模块划分提案（调研/可选参考）。 |
| `docs/agent-workflow-complete-guide.md` | AI 完整执行指南、写回契约、阶段视角与契约变更（可选参考）。 |

---

## 3. 契约文件

**契约仅以 T0-contracts 分支为准**；主分支上的 `specs/_contracts/` 不作为契约来源。

| 位置 | 内容 |
|------|------|
| **分支 T0-contracts** | 契约与全局依赖图的**唯一权威维护分支**；各 T0-* 分支必须从此拉取契约。 |
| `specs/_contracts/README.md` | 契约目录说明、从 T0-contracts 拉取契约的方式、契约列表。 |
| `specs/_contracts/000-module-dependency-map.md` | T0 模块依赖图（27 模块依赖表与上下游）。 |
| `specs/_contracts/001-core-public-api.md` | Core 对外 API 契约。 |
| `specs/_contracts/008-rhi-public-api.md` | RHI/RCI 对外 API 契约。 |
| `specs/_contracts/pipeline-to-rci.md` | 管线 → RHI 提交约定。 |
| 其他 | `002-object-public-api.md` … `027-xr-public-api.md`（每模块一契约，NNN 前缀统一命名）。 |

新增契约时在 **T0-contracts** 分支上更新，并在 `_contracts/README.md` 与 `000-module-dependency-map.md` 中同步更新。

---

## 4. 多 Agent 协作

详见 **`docs/agent-interface-sync.md`**：

- **工作流**：拉取 T0-contracts → 读依赖契约 → 实现 → 改接口时**先在 ABI 文件中更新完整 ABI 条目**再同步契约，并通知下游。
- **契约更新流程（ABI 先行）**：模块对外接口须在 **ABI 文件**（`NNN-modulename-ABI.md`）中更新**完整 ABI 条目**；若下游需要某接口而上游尚未提供，须在**上游模块的 ABI 文件**中增加该接口的 **TODO** 条目，待实现时转为正式行。见 `specs/_contracts/README.md`「契约更新流程」。
- **契约写法**：文件命名、建议结构（适用模块、消费者、版本/ABI、类型与句柄、能力列表、调用顺序与约束、变更记录）；接口符号与签名以对应 ABI 文件为准。
- **接口演进**：契约（能力与类型）+ ABI 文件（命名空间、头文件、符号、完整签名）→ 真实 API（头文件）。
- **发现依赖变化**：工作前、每个 task 开始前拉取 T0-contracts，并查阅契约/ABI 变更记录后做适配（Follow-up 与 Issue 的具体操作已废弃）。

---

## 5. 相关文档

| 文档 | 说明 |
|------|------|
| `docs/agent-interface-sync.md` | 多 Agent 协作的完整工作流与规范。 |
| `docs/agent-workflow-complete-guide.md` | AI 完整执行指南、写回契约、阶段视角。 |
| `docs/engine-modules-and-architecture.md` | 完整模块与依赖图。 |
| `docs/engine-build-module-convention.md` | CMake 构建规约与目录约定。 |

---

**旧版**：001-engine-core-module … 006-thirdparty-integration-tool 分支及对应 spec 已弃用，由 T0 架构替代。
