# T0 跨模块接口契约（Contracts）

本目录存放 **TenEngine T0 架构（27 模块）** 的跨模块接口契约，供多 Agent 协作时作为接口的单一事实来源。

**契约的官方维护分支**：**`T0-contracts`**（建议远程：`origin/T0-contracts`）。所有契约更新合并到该分支；各 **T0-NNN-modulename** 分支的 Agent 在工作前须从该分支拉取最新契约（`git pull origin T0-contracts` 或 `git fetch origin T0-contracts` + `git merge origin/T0-contracts`）。

## 使用方式

- **在任意 T0-* 模块分支上开始工作前**：先拉取最新契约：`git pull origin T0-contracts`（或 `git fetch origin T0-contracts` 后 `git merge origin/T0-contracts`）。
- **实现某模块前**：阅读本模块 **Dependencies** 中列出的契约文件（见 `000-module-dependency-map.md`），只使用契约中声明的类型与接口。
- **修改某模块对外接口时**：在 **`T0-contracts` 分支**上更新本目录下该模块对应的契约，并在 `000-module-dependency-map.md` 中确认下游模块，必要时创建跟进任务。

## 契约与依赖关系

- **依赖关系总览**：[000-module-dependency-map.md](./000-module-dependency-map.md)（27 模块依赖表与上下游）。
- **完整依赖图（Mermaid、矩阵、边列表）**：主仓库或 T0-contracts 分支下的 **`docs/dependency-graph-full.md`**。
- **模块详细规格**：主仓库 **`docs/module-specs/`**（001-core.md … 027-xr.md）；各 T0-NNN-modulename 分支仅含对应单模块描述 + 本目录 + 全局依赖图。

## 契约文件列表（T0 架构）

| 契约文件 | 说明 | 提供方模块 | 主要消费者 |
|----------|------|------------|------------|
| [core-public-api.md](./core-public-api.md) | Core 对外 API（内存、线程、平台、日志、数学、容器、模块加载） | 001-Core | Object, Application, Scene, Entity, RHI, Resource 等 |
| [rci-public-api.md](./rci-public-api.md) | RHI 图形抽象层对外 API（沿用 RCI 命名） | 008-RHI | RenderCore, Shader, PipelineCore, Pipeline, Editor |
| [pipeline-to-rci.md](./pipeline-to-rci.md) | 渲染管线 → RHI 的命令缓冲与提交约定 | 020-Pipeline ↔ 008-RHI | 008-RHI |

**说明**：T0 架构下可按需在 `T0-contracts` 分支新增 object-public-api、application-public-api、scene-public-api、entity-public-api、input-public-api、render-core-api、shader-public-api、resource-public-api 等；当前仍以 core-public-api、rci-public-api、pipeline-to-rci 为最小契约集，与 001–006 旧 spec 兼容过渡。

新增契约时请在本 README 和 `000-module-dependency-map.md` 中同步更新。
