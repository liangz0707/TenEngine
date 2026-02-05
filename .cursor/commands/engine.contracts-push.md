---
description: 向契约分支 T0-contracts 推送时，仅提交并推送契约相关路径（specs/_contracts、docs、cmake、.cursor、.specify），不含 build/include/source/tests/stub
---

## 用途

向 **T0-contracts** 分支推送时，只提交并推送契约相关目录的变更，不包含代码与构建产物。

## 允许推送的路径（仅这些）

- `specs/_contracts/`
- `docs/`
- `cmake/`
- `.cursor/`
- `.specify/`

## 不得包含的路径

- `build/`
- `include/`
- `source/`
- `tests/`
- `stub/`

## 执行步骤

1. **确认分支**：当前分支应为 `T0-contracts`，或确认要推送到 `origin T0-contracts`。

2. **只提交允许的路径**：
   - 若有未提交修改，只 add 允许的路径再 commit，例如：
     ```bash
     git add specs/_contracts/ docs/ cmake/ .cursor/ .specify/
     git status
     git commit -m "<描述契约/文档/配置的变更>"
     ```
   - 若已有 commit 中混入了 `build/`、`include/`、`source/`、`tests/`、`stub/` 的变更，需先修正（如拆分 commit、或只保留契约相关文件的提交）再推送。

3. **推送**：
   ```bash
   git push origin T0-contracts
   ```

**说明**：契约分支上应只包含契约、文档与工程配置的变更；代码与测试等应在功能分支（如 T0-N-xxx）上提交，不要推送到 T0-contracts。
