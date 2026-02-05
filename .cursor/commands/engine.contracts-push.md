---
description: 将当前分支的契约相关变更推送到 T0-contracts（仅 specs/_contracts、docs、cmake、.cursor、.specify），不含 build/include/source/tests/stub
---

## 用途

将**当前分支**（如 master 或 feature 分支）上的**契约相关路径**的变更，同步到 **T0-contracts** 分支并推送到远程。只包含允许的路径，不包含代码与构建产物。

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

1. **记录当前分支并确保契约变更已提交**
   - 记下当前分支名（如 `master`），后续称为 **源分支**。
   - 若当前分支有**未提交**的契约相关修改，先只提交允许的路径：
     ```bash
     git add specs/_contracts/ docs/ cmake/ .cursor/ .specify/
     git status
     git commit -m "<描述契约/文档/配置的变更>"
     ```
   - 若契约相关变更已在当前分支的提交中，则无需再 commit，直接下一步。

2. **切换到 T0-contracts 并拉取最新**
   ```bash
   git fetch origin T0-contracts
   git checkout T0-contracts
   git pull origin T0-contracts
   ```

3. **将源分支的契约相关路径覆盖到当前工作区**
   - 用上面记下的**源分支**名替换 `<源分支>`：
     ```bash
     git checkout <源分支> -- specs/_contracts docs cmake .cursor .specify
     ```
   - 效果：T0-contracts 工作区中上述路径与源分支一致，其它路径不变。

4. **若有变更则提交并推送**
   ```bash
   git status
   git add specs/_contracts/ docs/ cmake/ .cursor/ .specify/
   git commit -m "contract: sync from <源分支> (specs/_contracts, docs, cmake, .cursor, .specify)"
   git push origin T0-contracts
   ```
   - 若 `git status` 无变更，说明 T0-contracts 与源分支在这些路径上已一致，无需 commit，可直接推送已有提交（若有未推送提交则 `git push origin T0-contracts`）。

5. **切回源分支**
   ```bash
   git checkout <源分支>
   ```

**说明**：契约分支上应只包含契约、文档与工程配置的变更；本命令通过 `git checkout <源分支> -- <允许路径>` 只取源分支的契约相关路径，不会把 build/include/source/tests/stub 带入 T0-contracts。
