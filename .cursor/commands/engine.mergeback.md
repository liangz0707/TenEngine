---
description: 从 feature 分支合并指定路径回 T0-NNN-modulename，并提交推送
---

## 用途

当**当前处于 feature 分支**时，将本分支的代码与契约变更**合并回**对应的 **T0-NNN-modulename** 分支（如 `T0-012-mesh`）：先提交并推送 feature 全部内容，再切换到 T0 分支，仅用 feature 分支的指定路径覆盖，提交并推送到远程。

## 合并回 T0 的路径（仅这些）

- `Engine/`（若存在）
- `include/`
- `src/`
- `tests/`
- `CMakeLists.txt`
- `specs/_contracts/`
- `specs/user-stories/`
- `docs/`

未列出的路径（如 `build/`、`.cursor/`、`specs/012-mesh-full-module-001/` 等）不会覆盖 T0 分支。

## 前置条件

- 当前分支为 **feature 分支**（如 `012-mesh-full-module-001`），且可推断出 **T0-NNN-modulename**（如从 worktree 名 `TenEngine-012-mesh` 或分支名推断出 `T0-012-mesh`）。
- 若当前已是 `T0-NNN-modulename` 或无法推断 T0 分支名，则提示并中止。

## 执行步骤

1. **确认当前为 feature 分支并推断 T0 分支名**
   - 取当前分支名（如 `012-mesh-full-module-001`）。
   - 若分支名匹配 `NNN-modulename-*`（如 `012-mesh-full-module-001`），则 **T0 分支** 为 `T0-NNN-modulename`（如 `T0-012-mesh`）。
   - 或从 worktree 目录名推断：`TenEngine-012-mesh` → `T0-012-mesh`。
   - 若当前分支已是 `T0-*` 或 `master` 等，提示「当前不在 feature 分支，请先切换到 feature 分支再执行 mergeback」并结束。

2. **提交并推送 feature 分支所有内容到远程**
   ```bash
   git status
   git add -A
   git commit -m "<可选：描述本次实现或合并内容>"   # 若有未提交变更
   git push origin <feature 分支名>
   ```
   - 若无未提交变更，可跳过 commit，仅执行 `git push origin <feature 分支名>`（推送已有提交）。

3. **切换到 T0-NNN-modulename 并拉取最新**
   ```bash
   git fetch origin <T0 分支名>
   git checkout <T0 分支名>
   git pull origin <T0 分支名>
   ```

4. **用 feature 分支的指定路径覆盖当前工作区**
   - 将下面 `<feature 分支名>` 替换为步骤 1 记录的分支名。
   - 仅对**在 feature 分支存在的路径**执行 `git checkout`（若 feature 上无 `Engine/` 或 `specs/user-stories/` 则跳过对应项）。
   ```bash
   git checkout <feature 分支名> -- include src tests CMakeLists.txt specs/_contracts docs
   ```
   - 若 feature 分支存在 `Engine` 目录：
   ```bash
   git checkout <feature 分支名> -- Engine
   ```
   - 若 feature 分支存在 `specs/user-stories` 目录：
   ```bash
   git checkout <feature 分支名> -- specs/user-stories
   ```

5. **提交并推送到远程**
   ```bash
   git status
   git add include/ src/ tests/ CMakeLists.txt specs/_contracts/ docs/
   # 若存在则一并添加：Engine/  specs/user-stories/
   git add Engine/ specs/user-stories/ 2>/dev/null || true
   git commit -m "mergeback: sync from <feature 分支名> (include, src, tests, CMakeLists, specs/_contracts, specs/user-stories, docs)"
   git push origin <T0 分支名>
   ```
   - 只添加本次有变更的路径；无变更则可能无需 commit，仅推送已有提交即可。

6. **（可选）切回 feature 分支**
   ```bash
   git checkout <feature 分支名>
   ```

**说明**：mergeback 只把上述指定路径从 feature 同步到 T0-NNN-modulename，不改变 T0 分支上其它文件（如 .cursor、.specify、build 等），便于在 T0 上保留「可构建的模块实现 + 契约」子集。
