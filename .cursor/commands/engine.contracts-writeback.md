---
description: plan 结束后执行：将 plan 产出的「契约更新」写回 ABI；清除（删除）public-api 与 ABI 中已实现的 TODO（不勾选保留）；在 public-api 增加变更记录；提交并推送到 T0-contracts；可选合并到 T0-NNN-modulename
---

## 用途

在 **/speckit.plan 完成后、/speckit.tasks 之前**执行本命令，完成**写回契约**：

- 将 plan 产出的 **「契约更新」** 写回 **ABI 文件**（`specs/_contracts/NNN-modulename-ABI.md`）。
- **清除已实现的 TODO**：若 plan 实现了 public-api 的 TODO 列表中的条目，须**清除**（删除）对应内容：删除该 TODO 行或整段「## TODO 列表」区块（不勾选保留）；若 ABI 中有 TODO 行被实现，替换为正式 ABI 行。
- 在 public-api 的 **变更记录** 中增加本计划写回说明。
- 提交并推送到 **T0-contracts**；可选：将契约变更合并到 **T0-NNN-modulename** 并推送。

详见 `docs/agent-workflow-complete-guide.md`「2.0 写回契约」。

## 输入（可选）

- **模块与 feature**：若未提供，从当前 worktree 路径或打开的 `specs/<feature>/plan.md` 推断（如 `specs/013-resource-xxx/plan.md` → 013-Resource，feature 名 xxx）。

## 执行步骤

1. **确定 NNN-modulename 与 feature**  
   从用户输入、当前 worktree 目录或打开的 `specs/*/plan.md` 推断本模块与 feature 名。

2. **从 plan 产物提取「契约更新」与已实现范围**  
   - 打开 `specs/<feature>/plan.md`（及同目录 plan 产物）。  
   - 提取「契约更新」小节中**仅新增/修改**的 ABI 条目；若为空则提示无写回内容并结束。  
   - 对照 plan 的**实现范围**与 public-api 的 **TODO 列表**，列出本 plan **已实现的 TODO 项**（如：Addressing/ResolvePath、缓存/GetCached、Save 流程、Load 与注册、导入统一接口、序列化/反序列化相关等），用于步骤 4 清除（删除）这些 TODO 行或整段 TODO 区块。

3. **写回 ABI 文件**  
   - 编辑 `specs/_contracts/NNN-modulename-ABI.md`：  
     - **新增**条目 → 表中增补新行（表头与现有格式一致）。  
     - **修改**条目 → 按符号/头文件匹配替换已有行。  
     - **若 plan 实现的是 ABI 中某条 TODO**（如「TODO：下游 xxx 需要」）→ **清除该 TODO**：将该行替换为正式 ABI 行（完整符号与签名），或删除原 TODO 行并写入正式行。  
     - 不覆盖、不删除 plan 未涉及的其它条目。

4. **更新 public-api：清除 TODO + 变更记录**  
   - 编辑 `specs/_contracts/NNN-modulename-public-api.md`：  
     - **清除已实现的 TODO**：删除步骤 2 中列出的、本 plan 已实现的 TODO 行；若本 plan 已实现 TODO 列表中全部或绝大部分项，则**删除整段「## TODO 列表」区块**（含标题与所有条目），不在文件中保留勾选状态。  
     - **变更记录**：在「## 变更记录」表中增加一行：日期、变更说明为「ABI 写回（plan NNN-modulename-&lt;feature&gt;）：……（列出本次写回的符号或能力）」；若清除了 TODO 列表，再增加一行「清除 TODO 列表：plan NNN-modulename-&lt;feature&gt; 已完成，相关任务已从 public-api 移除」。

5. **提交并推送到 T0-contracts**  
   - 若当前不在 T0-contracts：`git fetch origin T0-contracts`，`git checkout T0-contracts`，`git pull origin T0-contracts`。  
   - 将写回后的 ABI/public-api 带入当前分支（若在 feature 分支修改的，可用 `git checkout <feature> -- specs/_contracts/NNN-modulename-ABI.md specs/_contracts/NNN-modulename-public-api.md`）。  
   - 只提交契约相关路径：  
     ```bash
     git add specs/_contracts/NNN-modulename-ABI.md specs/_contracts/NNN-modulename-public-api.md
     git commit -m "contract(NNN-modulename): sync from plan NNN-modulename-<feature>"
     git push origin T0-contracts
     ```

6. **（可选）合并契约到 T0-NNN-modulename**  
   - 当前 worktree：`git fetch origin T0-contracts`，`git merge origin/T0-contracts`，使 feature 分支拿到最新契约。  
   - 若需将契约单独同步到模块 T0 分支：切换到 `T0-NNN-modulename`，`git checkout T0-contracts -- specs/_contracts/ docs/ cmake/ .cursor/ .specify/`（或仅 `specs/_contracts/`），提交并 `git push origin T0-NNN-modulename`；或使用 **engine.contracts-push** 后再在 T0-NNN-modulename 上 merge T0-contracts。

7. **在当前 worktree 拉取更新后的契约**  
   - 在 feature 所在 worktree：`git checkout <feature>`，`git fetch origin T0-contracts`，`git merge origin/T0-contracts`。确认 ABI 与 public-api 已更新。

8. **后续提示**  
   - 告知：契约已写回、TODO 已清除、已推送至 T0-contracts；可继续 **/speckit.tasks**。  
   - 若在 public-api 中增加了 **变更记录**，可**主动运行** **engine.contracts-downstream-todo** 为下游 public-api 添加 TODO 兼容记录。

## 说明

- 写回仅增补或替换 plan 产出的新增/修改部分；**必须**清除（删除）public-api 与 ABI 中本 plan 已实现的 TODO，不采用勾选保留。  
- 推送范围遵循 **engine.contracts-push**（仅契约相关路径时只提交 `specs/_contracts/` 等允许路径）。
