---
description: 修改契约或实现前，按 API 为源、ABI 为据的规则自检（见 .cursor/rules/contracts-api-abi.mdc）
---

## 用途

在执行 **修改 specs/_contracts 下文件** 或 **plan/task/implement 实现** 前，可运行本命令，确保遵循 API/ABI 约定。

## 必须遵守的规则（与 .cursor/rules/contracts-api-abi.mdc 一致）

### 1. 修改 _contracts 时

- 各模块的 **public-api** 是对应 **ABI** 的**唯一来源**（除非用户明确另有说明）。
- API 内容**不得反向引用** ABI（不得在 public-api 里写“见 ABI”“以 ABI 为准”等）。
- 更新或新增 ABI 时，须以该模块的 public-api 为准，不得仅凭 ABI 反推 public-api。

### 2. 实现 plan / task / implement 时

- **先**根据**上游模块的 API 文件**（public-api）理解其**能力与类型**。
- **再**根据 **ABI** 的**具体接口**（命名空间、头文件、符号、调用约定）进行实现。
- 即：功能与语义以 API 为准，具体接口形式以 ABI 为准。

## 执行

若用户未附带具体文件或任务：

1. 若当前上下文涉及 **specs/_contracts/** 下文件，检查本次修改是否以 public-api 为 ABI 来源、且 API 未反向引用 ABI。
2. 若当前上下文涉及 **实现**（plan/task/implement），确认已查阅相关上游的 public-api，再按 ABI 实现。

若用户附带了文件路径或模块编号（如 `004-scene`），则针对该模块或所列文件应用上述规则并给出简短自检结论或修改建议。
