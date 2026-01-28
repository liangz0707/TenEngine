# T0 规约与契约说明

本文档说明 **T0 架构（27 模块）** 下规约文件与契约文件的组织方式及约束分支的使用。

---

## 1. 最新架构（T0）

- **主分支仅用于配置**：**master**（或 main）仅用于仓库与工程配置（构建、CI、文档索引等）；**契约不以主分支为来源**，各模块开发与接口同步不依赖主分支上的 `specs/_contracts/`。
- **契约必须从 T0-contracts 拉取**：契约的**唯一权威来源**是 **`T0-contracts`** 分支；所有 T0-* 模块分支**必须**从 **`origin/T0-contracts`** 拉取契约（`git pull origin T0-contracts`），不得以主分支上的 `_contracts/` 为依据。
- **模块**：001-Core … 027-XR（见 `docs/tenengine-full-module-spec.md`、`docs/module-specs/`）。
- **分支**：
  - **T0-contracts**：约束/契约的**唯一发布源**，维护 `specs/_contracts/` 与 `docs/dependency-graph-full.md`。
  - **T0-NNN-modulename**：各模块独立分支（如 T0-001-core、T0-008-rhi），仅含必要约束、该模块描述与全局依赖；工作前须从 T0-contracts 拉取契约。
- **工作前**：在任意 T0-* 分支上工作前，**必须**执行 `git pull origin T0-contracts` 拉取最新契约。

---

## 2. 规约文件

| 位置 | 内容 |
|------|------|
| `docs/tenengine-full-module-spec.md` | 全功能模块规格（27 模块、功能域映射、依赖表）。 |
| `docs/module-specs/` | 各模块详细规格（001-core.md … 027-xr.md），含模块说明、功能、难度、资源类型、子模块、上下游、外部依赖。 |
| `docs/dependency-graph-full.md` | 完整依赖图（Mermaid、矩阵、ASCII、边列表）。 |
| `docs/proposed-module-architecture.md` | 整合式模块划分提案（可选参考）。 |
| `docs/three-engines-modules-and-dependencies.md` | 三引擎对照与依赖（可选参考）。 |

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

详见 **`docs/multi-agent-interface-sync.md`**：拉取 T0-contracts、读依赖契约、改接口时更新契约并通知下游。

---

**旧版**：001-engine-core-module … 006-thirdparty-integration-tool 分支及对应 spec 已弃用，由 T0 架构替代。
