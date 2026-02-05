---
description: 拉取契约分支 T0-contracts 并合并到当前分支（仅更新本地，不推送）
---

## 用途

在开始写 spec、改契约或实现前，将 **T0-contracts** 上的最新契约合并到当前分支，保证本地 `specs/_contracts/` 等与契约分支一致。

## 执行步骤

1. 拉取远程契约分支并合并到**当前分支**（不推送）：
   ```bash
   git fetch origin T0-contracts
   git merge origin/T0-contracts
   ```
   或一条命令：`git pull origin T0-contracts`

2. 若有冲突，按提示在本地解决后再继续。

**说明**：契约的单一事实来源为分支 `T0-contracts`；主分支仅用于仓库与工程配置。本命令只更新本地当前分支，**不会**执行 `git push`。
