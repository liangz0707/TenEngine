# 跨模块接口契约（Contracts）

本目录存放 **TenEngine 各模块之间的接口契约**，供多 Agent 协作时作为接口的单一事实来源。

**契约的官方维护分支**：**`contracts`**（`origin/contracts`）。所有契约更新合并到该分支；其他分支的 Agent 在工作前须从该分支拉取最新契约（`git pull origin contracts` 或 `git fetch origin contracts` + `git merge origin/contracts`）。

## 使用方式

- **在任意特性分支上开始工作前**：先拉取最新契约：`git pull origin contracts`（或 `git fetch origin contracts` 后 `git merge origin/contracts`）。
- **实现某模块前**：阅读本模块 **Dependencies** 中列出的契约文件，只使用契约中声明的类型与接口。
- **修改某模块对外接口时**：在 **`contracts` 分支**上更新本目录下该模块对应的契约，并在 `000-module-dependency-map.md` 中确认下游模块，必要时创建跟进任务。

## 契约列表与依赖关系

依赖关系总览见 **[000-module-dependency-map.md](./000-module-dependency-map.md)**。

| 契约文件 | 说明 | 提供方 spec | 主要消费者 |
|----------|------|-------------|------------|
| [core-public-api.md](./core-public-api.md) | Core 对外 API（内存、线程、ECS、平台、序列化） | 001-engine-core-module | RCI、Resource、Shader、Editor 等 |
| [rci-public-api.md](./rci-public-api.md) | RCI 渲染抽象层对外 API | 002-rendering-rci-interface | Render Pipeline、Editor、Shader |
| [pipeline-to-rci.md](./pipeline-to-rci.md) | 渲染流水线 → RCI 的命令缓冲与提交约定 | 006-render-pipeline-system | 002-rendering-rci-interface |

新增契约时请在本 README 和依赖图中同步更新。
