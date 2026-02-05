---
description: plan 结束后执行：将 plan 产出的 ABI/API 修改写回 ABI 与 public-api 文件；若实现的是 ABI 中的 TODO 须清除该 TODO；然后提交并推送到 T0-contracts
---

## 用途

在 **/speckit.plan 完成后、/speckit.tasks 之前**执行本命令，完成**写回契约**：

- 将 plan 产出的 **ABI 与 API 的修改**写回 `specs/_contracts/` 下的 **ABI 文件**（`NNN-modulename-ABI.md`）和 **public-api 文件**（`NNN-modulename-public-api.md`）。
- 若 plan 是在**实现 ABI 中已有的 TODO 条目**（如「TODO：下游 xxx 需要」），写回时须**清除该 TODO**：将对应行改为正式 ABI 行（或替换/删除原 TODO 行），并同步 public-api。
- 完成后**提交并推送到 T0-contracts**，再在当前 worktree 拉取，即可继续 /speckit.tasks。

详见 `docs/agent-workflow-complete-guide.md`「2.0 写回契约」。

## 输入（可选）

- **模块与 feature**：若未提供，从当前 worktree 路径或打开的 `specs/<feature>/plan.md` 推断（如 `specs/013-resource-xxx/plan.md` → 013-Resource，feature 名 xxx）。

## 执行步骤

1. **确定 NNN-modulename 与 feature**  
   从用户输入、当前 worktree 目录或打开的 `specs/*/plan.md` 推断本模块与 feature 名。

2. **从 plan 产物提取「契约更新」清单**  
   - 打开 `specs/<feature>/plan.md`（及同目录 plan 产物）。  
   - 提取「契约更新」小节中**仅新增/修改**的 ABI 条目；若为空则提示无写回内容并结束。  
   - 确认每条含：模块名、命名空间、类名、接口说明、头文件、符号、说明/完整函数签名（与现有 ABI 表头一致）。

3. **在主仓库 T0-contracts 上写回并提交**  
   - 主仓库（或当前仓库即主仓库）：`git fetch origin T0-contracts`，`git checkout T0-contracts`，`git pull origin T0-contracts`。  
   - **ABI 文件** `specs/_contracts/NNN-modulename-ABI.md`：  
     - **新增**条目 → 表中增补新行。  
     - **修改**条目 → 按符号/头文件匹配替换已有行。  
     - **若 plan 实现的是本模块 ABI 中某条 TODO**（如「TODO：下游 NNN-xxx 需要」）→ **清除该 TODO**：将该行替换为正式 ABI 行（完整符号与签名），或删除原 TODO 行并写入正式行。  
     - 不覆盖、不删除 plan 未涉及的其它条目。  
   - **public-api 文件** `specs/_contracts/NNN-modulename-public-api.md`：按 plan 产出更新能力/类型与变更记录。  
   - 只提交契约相关路径（符合 **engine.contracts-push**）：  
     ```bash
     git add specs/_contracts/NNN-modulename-ABI.md specs/_contracts/NNN-modulename-public-api.md
     git commit -m "contract(NNN-modulename): sync from plan NNN-modulename-<feature>"
     git push origin T0-contracts
     ```

4. **在当前 worktree 拉取更新后的契约**  
   - 在当前 feature 所在 worktree：`git fetch origin T0-contracts`，`git merge origin/T0-contracts`。确认 ABI 与 public-api 已更新。

5. **后续提示**  
   - 告知：契约已写回并推送，可继续 **/speckit.tasks**。  
   - 若在 public-api 中增加了 **## 变更记录**，可**主动运行** **engine.contracts-downstream-todo** 为下游 public-api 添加 TODO 兼容记录。

## 说明

- 写回仅增补或替换 plan 产出的新增/修改部分，并清除已实现的 TODO；不整表覆盖。  
- 推送范围遵循 **engine.contracts-push**（仅契约相关路径）。
