# 文档作为资产、随时生成代码

本文档说明如何将 **规约与契约等文档视为可复现资产**，在需要时**从文档生成或复现代码**。供后续实现「文档 → 代码」流水线、脚本或 AI 流程时参考。

---

## 1. 目标与定位

### 1.1 核心目标

- **文档即资产**：规约、契约、spec、plan 等**稳定、可版本化**的文档，是生成代码的**权威输入**。
- **随时复现**：在任意时刻，给定**资产路径或标识**（如模块 ID、feature 名），可**按文档重新生成**对应代码，而不依赖「必须一个子集一个子集」的增量实现流程。
- **可配置粒度**：支持按**模块**、按 **feature 切片**、或按**多模块/系统**生成，粒度由文档与配置共同决定。

### 1.2 与现有流程的关系

| 现有流程 | 与「文档 → 代码」的关系 |
|----------|--------------------------|
| **docs/module-specs**、**specs/_contracts** | 直接作为**文档资产**；生成器读取规约 + 契约作为输入。 |
| **Spec Kit**（specify / plan / tasks / implement） | 可产出 **spec、plan、tasks**，这些同样视为资产；**implement** 即一次「从文档生成代码」的执行。 |
| **T0-contracts**、**Constitution** | 契约以 T0-contracts 为准；生成代码时**只使用契约中已声明的类型与接口**，符合 Constitution 的契约约束。 |

**文档资产**与 **Spec Kit / T0 工作流**兼容：既可用 Spec Kit 迭代产出资产再生成，也可**仅**用已有规约+契约直接生成，无需先跑 specify/plan。完整执行流程见 **`docs/agent-workflow-complete-guide.md`**。

---

## 2. 文档资产的定义与存放

### 2.1 资产目录结构

| 资产类型 | 位置（示例） | 说明 |
|----------|--------------|------|
| **模块规约** | `docs/module-specs/NNN-modulename.md` | 模块说明、功能、子模块、上下游、依赖、实现约束等。 |
| **契约** | `specs/_contracts/NNN-modulename-public-api.md` | 适用模块、消费者、版本/ABI、类型与句柄、能力列表、**API 雏形**、调用顺序与约束、变更记录。契约以 **T0-contracts** 分支为准。 |
| **依赖图** | `specs/_contracts/000-module-dependency-map.md` | 模块依赖关系；生成多模块时用于解析上下游。 |
| **Feature 切片**（可选） | `specs/<feature>/spec.md` | 本 feature 的实现范围（如「Alloc/Free、Log」），引用规约与契约。 |
| **计划**（可选） | `specs/<feature>/plan.md` | 设计、数据结构、对外 API 清单等；plan 产出的「契约更新」可写回契约。 |
| **任务列表**（可选） | `specs/<feature>/tasks.md` | 可执行任务列表；implement 按此生成代码。 |

### 2.2 资产优先级与最小集

- **生成代码的最小输入**：**模块规约** + **契约**（含能力列表、类型与句柄、**API 雏形**）。
- **若有 feature 切片**：再加 **spec.md**（明确本切片范围）；可选 **plan.md**、**tasks.md**，用于更细粒度的生成与任务拆分。

生成器**至少**读取规约 + 契约；按需再读 spec/plan/tasks。

---

## 3. 生成粒度：如何控制

### 3.1 三种粒度

| 粒度 | 含义 | 输入资产 | 典型用途 |
|------|------|----------|----------|
| **模块** | 整个模块（如 001-Core） | `docs/module-specs/001-core.md` + `specs/_contracts/001-core-public-api.md` | 整模块复现、大范围重构 |
| **Feature 切片** | 模块的一个子集（如 Alloc/Free、Log） | 规约 + 契约 + `specs/<feature>/spec.md`（+ 可选 plan/tasks） | 试点、增量交付、单 feature 复现 |
| **系统 / 多模块** | 多个模块组合（如 Core + Object） | 多个规约 + 多个契约 + 依赖图 | 跨模块联调、端到端生成 |

### 3.2 粒度在哪控制

目前**没有**单独的「生成粒度」配置项；粒度由以下**共同**决定：

1. **调用时的参数或配置**  
   - 例如：`--scope=module --module=001-core`，或 `--scope=feature --feature=001-core-minimal`。  
   - 生成脚本 / AI 流程根据 `scope` 决定读取哪些资产。

2. **Feature 的 spec.md**（当 `scope=feature` 时）  
   - `spec.md` 中**显式写出本切片范围**（如「本 feature 仅实现：Alloc、Free、Log」）。  
   - **必须枚举具体项**，避免仅写「本切片」「只描述范围」等抽象表述，否则模型易过度收缩（如只做 Alloc）。详见 **§6 提示词规范**。

3. **规约 + 契约的已有内容**  
   - 契约的「能力列表」「API 雏形」限定可生成的接口；规约限定功能边界。  
   - 生成时**不得**超出契约已声明、规约已描述的范围。

**建议**：  
- 若实现「文档 → 代码」脚本或配置，增加**显式粒度选项**（如 `scope: module | feature | system`）和对应参数（`--module`、`--feature` 等），便于复现与自动化。  
- 在 **spec.md** 中可增加可选字段，如 `scope: feature`、`slice: [Alloc, Free, Log]`，供生成器解析。

---

## 4. 复现流程：输入、步骤、输出

### 4.1 输入

- **必选**：  
  - 模块规约路径（如 `docs/module-specs/001-core.md`）。  
  - 契约路径（如 `specs/_contracts/001-core-public-api.md`）；契约以 **T0-contracts** 为准，生成前需拉取最新。
- **可选**：  
  - Feature 标识与 `specs/<feature>/spec.md`（当 `scope=feature`）。  
  - `plan.md`、`tasks.md`（若存在且生成流程使用）。  
  - 依赖图（多模块生成时）。

### 4.2 步骤（建议）

1. **确定粒度**：按 `scope`（module / feature / system）选择要生成的边界。  
2. **解析依赖**：若为多模块，按 `000-module-dependency-map.md` 解析上下游；按依赖顺序生成或至少按依赖校验接口。  
3. **读取资产**：拉取 T0-contracts 后，读取对应规约、契约；若为 feature，读取 spec（及可选 plan/tasks）。  
4. **校验**：生成内容**只使用**契约中已声明的类型与接口；不引入未在规约/契约中描述的能力。  
5. **生成**：依规约 + 契约（+ spec/plan/tasks）生成源码、工程文件等；输出到约定目录（如各模块 worktree 或主仓库的模块路径）。  
6. **后续**：可选跑构建、测试；若有契约或规约变更，可重新执行本流程以复现。

### 4.3 输出

- **代码**：与所选粒度对应的实现（如模块级目录结构、头文件/源文件、CMake 等）。  
- **位置**：与现有工程布局一致（如 `TenEngine-NNN-modulename` worktree 内），或脚本/配置指定的输出根目录。

### 4.4 触发方式

- **人工**：在文档或脚本中写明「复现 001-Core」「复现 feature 001-core-minimal」等，再执行对应命令或 AI 流程。  
- **脚本**：例如 `./scripts/codegen-from-docs --scope=module --module=001-core`（具体命令以实际实现为准）。  
- **AI 流程**：提供固定提示词模板，填入模块/feature 等参数，由 AI 按本文档与规约、契约执行读取 → 校验 → 生成。

---

## 5. 与 T0、Spec Kit、Constitution 的衔接

### 5.1 T0-contracts 与契约

- 契约的**唯一权威来源**为 **T0-contracts** 分支。  
- 生成代码前，应在对应 worktree 或主仓库**拉取** `origin/T0-contracts`，再读取 `specs/_contracts/`。  
- 生成结果**不得**使用契约未声明的类型或接口；若契约无「API 雏形」，可先用能力列表与类型/句柄，或先在 T0-contracts 上补充 API 雏形再生成。

### 5.2 Constitution

- 生成流程须符合 **Constitution**（如 `.specify/memory/constitution.md`）中的契约约束与模块边界。  
- 仅使用 `specs/_contracts/` 中已声明的类型与接口；不依赖内部或未文档化 API。

### 5.3 Spec Kit

- **spec / plan / tasks** 可由 Spec Kit 产出，并视为文档资产的一部分。  
- **implement** 即一次「从 tasks（+ plan + spec + 规约 + 契约）生成代码」；复现时亦可**不**经 Spec Kit，直接按规约 + 契约（+ spec/plan/tasks）生成。  
- 若使用 Spec Kit，feature 分支名、`FEATURE_DIR = specs/<feature>` 等约定仍适用；与「文档 → 代码」的 feature 粒度一致。

---

## 6. 提示词规范：范围枚举与避免过度收缩

### 6.1 问题

仅写「本 feature 的规约见 …，契约见 …；spec.md 只描述本切片的范围、不重复完整规约」等，**不枚举**本切片具体包含哪些能力或 API 时，模型易将「本切片」理解为**极小范围**，导致生成内容过窄（如只做 Alloc）。

### 6.2 规范

- **显式枚举范围**：在 specify / plan 的 prompt 中，**明确写出**本 feature 或本切片包含的内容。  
  - 例如：「本 feature 仅实现**最小子集**：内存 **Alloc、Free** 与分级 **Log**（三者皆包含）。」  
  - 或：「本切片包含：**Alloc、Free、Log**；spec.md 引用规约与契约，只描述此范围。」
- **避免仅用抽象表述**：如单独使用「本切片」「只描述范围」「不重复完整规约」而不列出具体项，不足以保证生成范围正确。
- **与契约一致**：枚举的范围应在契约的「能力列表」或「API 雏形」中有对应；生成时不得超出契约。

### 6.3 推荐 prompt 片段（可粘贴到 specify / plan）

```
本 feature 的完整模块规约见 docs/module-specs/001-core.md，对外 API 契约见 specs/_contracts/001-core-public-api.md。
本 feature 仅实现其中最小子集：内存 Alloc、Free 与分级 Log（三者皆包含）。
spec.md 中必须引用上述两文件，并只描述本切片的范围（不重复写完整模块规约）。
```

可根据实际模块与切片替换规约路径、契约路径及枚举列表。

---

## 7. 检查清单（复现前）

- [ ] 已拉取 **T0-contracts**，`specs/_contracts/` 为最新。  
- [ ] 目标**模块规约**、**契约**存在且已阅读；契约含「能力列表」及（若生成需要）「API 雏形」。  
- [ ] 若为 **feature 粒度**：`specs/<feature>/spec.md` 存在，且其中**显式枚举**本切片范围（如 Alloc、Free、Log）。  
- [ ] 生成粒度（module / feature / system）已确定，且与所选资产一致。  
- [ ] 生成流程约定：**只使用**契约中已声明的类型与接口；不引入规约/契约外的能力。  
- [ ] 输出目录或 worktree 已确定；符合 Constitution 与 T0 约定。

---

## 8. 后续扩展建议

- **生成脚本**：实现 `codegen-from-docs`（或等价脚本），支持 `--scope`、`--module`、`--feature` 等参数，按本文档读取资产并生成代码。  
- **配置化**：在 `.specify` 或项目配置中增加 `generation_scope`、默认模块/feature 等，便于 CI 或本地一键复现。  
- **文档资产索引**：维护一份「可生成」资产列表（模块 ID、feature 名、路径），便于发现与批量复现。  
- **与 Spec Kit 集成**：将「文档 → 代码」作为 Spec Kit implement 的另一种后端（如「从规约+契约直接生成」模式），与现有 tasks 驱动实现并存。

---

## 9. 小结

| 项目 | 说明 |
|------|------|
| **文档资产** | 规约（`docs/module-specs`）、契约（`specs/_contracts`）、可选 spec/plan/tasks（`specs/<feature>/`）。 |
| **生成粒度** | 模块 / feature / 系统；由调用参数与 spec 中的**显式范围枚举**共同控制。 |
| **复现流程** | 确定粒度 → 拉取 T0-contracts → 读取规约+契约（+ spec/plan/tasks）→ 校验 → 生成 → 输出到约定位置。 |
| **提示词** | 必须**显式枚举**本切片范围（如 Alloc、Free、Log），避免仅用「本切片」「只描述范围」等抽象表述。 |
| **约束** | 仅使用契约已声明的类型与接口；符合 Constitution 与 T0-contracts。 |

以上约定可使**文档作为资产、随时生成代码**在实际项目中可执行、可复现，并与现有 T0、Spec Kit、Constitution 流程兼容。
