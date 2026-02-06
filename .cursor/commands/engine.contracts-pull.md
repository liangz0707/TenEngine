---
description: 拉取契约分支 T0-contracts 并合并到当前分支（含 Engine，不含 Engine/*/build；仅更新本地，不推送）
---

## 用途

在开始写 spec、改契约或实现前，将 **T0-contracts** 上的最新契约与 **Engine 源码**合并到当前分支，保证本地 `specs/_contracts/`、`docs/`、`Engine/` 等与契约分支一致。契约分支上的 Engine **不包含** 各模块下的 `build/` 目录（由 contracts-push 排除）。

## 执行步骤

1. 拉取远程契约分支并合并到**当前分支**（不推送）：
   ```bash
   git fetch origin T0-contracts
   git merge origin/T0-contracts
   ```
   或一条命令：`git pull origin T0-contracts`

2. 若有冲突，按提示在本地解决后再继续。

3. （可选）合并后若本地存在 `Engine/*/build/` 且希望不再跟踪，可确保 `.gitignore` 含 `build/` 或 `Engine/*/build/`，再按需执行 `git clean` 或 `git rm -r --cached` 清理。

**说明**：契约的单一事实来源为分支 `T0-contracts`（含 specs/_contracts、docs、cmake、.cursor、.specify、**Engine**，不含 Engine 下 build）。本命令只更新本地当前分支，**不会**执行 `git push`。
