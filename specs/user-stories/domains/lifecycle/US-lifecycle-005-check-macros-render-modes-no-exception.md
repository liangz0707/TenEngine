# US-lifecycle-005：统一 Check 宏、编译选项控制 Check、容易错误处不用异常；渲染支持 Debug/Hybrid/Resource

- **标题**：**容易错误的地方不要使用异常捕获**；可通过统一的 **CheckWarning()**、**CheckError()** 宏进行校验；**可通过编译选项设置启用的 Check**。渲染支持 **Debug**、**Hybrid**、**Resource** 三种模式。
- **编号**：US-lifecycle-005

---

## 1. 角色/触发

- **角色**：引擎与游戏侧程序员
- **触发**：需要在易错路径（渲染、资源加载、RHI 调用等）进行校验时，**不使用异常捕获**，改用统一 **CheckWarning()**、**CheckError()** 宏；并**通过编译选项**控制是否启用这些 Check（如发布构建关闭或仅保留 Error，开发构建开启 Warning 与 Error）。渲染需支持 **Debug**、**Hybrid**、**Resource** 三种模式（可通过编译选项或运行时配置选择）。

---

## 2. 端到端流程与约定

1. **容易错误的地方不要使用异常捕获**：渲染路径、资源加载、RHI 调用等易错处，**不使用 try/catch**，改用返回值或统一 **CheckWarning()**、**CheckError()** 宏进行校验。
2. **统一 Check 宏**：**CheckWarning(condition[, message])** — 条件为假时记录 Warning；**CheckError(condition[, message])** — 条件为假时记录 Error 并可选中止/返回。由 001-Core 或公共头文件提供（见 001-core-ABI）。
3. **编译选项设置启用的 Check**：可通过编译选项设置是否启用 Check，例如 `TENENGINE_CHECK_WARNING`、`TENENGINE_CHECK_ERROR`，或统一 `TENENGINE_CHECK_LEVEL`（0=关闭，1=Warning，2=Error）；发布构建可关闭或仅保留 Error，开发/调试构建可开启 Warning 与 Error。
4. **渲染支持 Debug、Hybrid、Resource**：渲染支持三种模式（如 Debug=全量校验/调试绘制，Hybrid=部分校验，Resource=发布/最小校验）；可通过编译选项或运行时配置选择，与 Check 宏的启用程度协同（见 020-Pipeline 规格与契约）。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 001-Core | 提供 **CheckWarning()**、**CheckError()** 宏；编译选项 TENENGINE_CHECK_WARNING / TENENGINE_CHECK_ERROR 或 TENENGINE_CHECK_LEVEL 控制是否启用 |
| 020-Pipeline | 渲染支持 **Debug**、**Hybrid**、**Resource** 三种模式；容易错误处使用 Check 宏，不使用异常捕获 |
| 文档 | docs/engine-abi-interface-generation-spec.md：错误处理约定（易错处不用异常，用 Check 宏）；§1.1 统一 Check 宏与编译选项 |

---

## 4. 每模块职责与 I/O

### 001-Core

- **职责**：提供 **CheckWarning(condition[, message])**、**CheckError(condition[, message])** 宏；**可通过编译选项设置启用的 Check**（TENENGINE_CHECK_WARNING、TENENGINE_CHECK_ERROR 或 TENENGINE_CHECK_LEVEL）；容易错误处用此宏代替异常捕获。
- **输入**：编译选项（宏定义）；运行时条件与可选 message。
- **输出**：CheckWarning、CheckError 宏；条件为假时记录 Warning/Error，可选中止或返回。

### 020-Pipeline

- **职责**：渲染支持 **Debug**、**Hybrid**、**Resource** 三种模式；可通过编译选项或运行时配置选择；容易错误处**不要使用异常捕获**，使用统一 CheckWarning/CheckError 宏进行校验。
- **输入**：编译选项或运行时 RenderMode；易错路径上的条件校验。
- **输出**：RenderMode 枚举（Debug、Hybrid、Resource）；与 001-Core Check 宏协同。

---

## 5. 派生 ABI（与契约/ABI 对齐）

- **001-core-ABI**：CheckWarning、CheckError 宏；编译选项控制启用。
- **020-pipeline-ABI**：RenderMode 枚举（Debug、Hybrid、Resource）；易错处使用 Check 宏，不使用异常。
- **docs/engine-abi-interface-generation-spec.md**：§1 错误处理（易错处不用异常，用 Check 宏）；§1.1 统一 Check 宏与编译选项。

---

## 6. 验收要点

- 容易错误的地方（渲染、资源、RHI 等）**不使用异常捕获**，使用 **CheckWarning()**、**CheckError()** 宏进行校验。
- **可通过编译选项设置启用的 Check**（如 TENENGINE_CHECK_WARNING、TENENGINE_CHECK_ERROR 或 TENENGINE_CHECK_LEVEL）；发布构建可关闭或仅保留 Error。
- 渲染支持 **Debug**、**Hybrid**、**Resource** 三种模式，可通过编译选项或运行时配置选择。
