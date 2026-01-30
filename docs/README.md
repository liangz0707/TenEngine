# 文档索引与命名约定

## 命名约定

- **`engine-*`**：与**工程/引擎本身**相关（模块架构、构建规约、外部参考等）。
- **`agent-*`**：与 **Agent 开发/协作** 相关（契约与规约索引、接口同步、工作流、文档资产与代码生成等）。

新增文档时按上述前缀命名，便于区分用途与引用。

---

## 文档一览

| 文档 | 说明 |
|------|------|
| [engine-modules-and-architecture.md](engine-modules-and-architecture.md) | 完整模块与基础架构（27 模块、依赖图）。 |
| [engine-build-module-convention.md](engine-build-module-convention.md) | CMake 构建规约与目录约定。 |
| [agent-contracts-and-specs-T0.md](agent-contracts-and-specs-T0.md) | T0 规约与契约快速索引。 |
| [agent-interface-sync.md](agent-interface-sync.md) | 多 Agent 协作：接口同步策略与工作流。 |
| [agent-workflow-complete-guide.md](agent-workflow-complete-guide.md) | AI 完整执行指南（Spec Kit 步骤与提示词）。 |
| [agent-docs-as-assets-codegen.md](agent-docs-as-assets-codegen.md) | 文档作为资产、随时生成代码。 |
| [module-specs/](module-specs/) | 各模块详细规格（001-core … 027-xr）。 |
| [research/](research/) | 调研与对比类文档（Unity/Unreal 参考、历史提案等）。 |

---

## 重复内容确认

当前文档已做收敛，**无大段重复**：

- **agent-contracts-and-specs-T0.md** 为**快速索引**，仅保留核心要点与规约/契约文件列表，详细原则、工作流、契约写法、接口演进等统一指向 **agent-interface-sync.md**。
- **agent-interface-sync.md** 为多 Agent 协作的**唯一详细规范**，包含原则、目录与角色、工作流、契约怎么写、在规格中引用契约等完整内容。
- 模块与依赖的**权威描述**在 **engine-modules-and-architecture.md**；research 下为调研/历史参考，不与之重复。
