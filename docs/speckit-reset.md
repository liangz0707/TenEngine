# speckit 工作状态快速重置

使用 spec-cli / speckit 进行范式开发时，若希望将当前工作状态**快速重置**到执行 `/speckit.plan`、`/speckit.tasks` 或 `/speckit.implement` **之前**的状态，可采用以下方式。

---

## 1. 使用重置脚本（推荐）

在仓库根目录、且当前分支为特性分支（如 `001-engine-core-module`）时执行：

```powershell
# 重置到 /speckit.plan 之前：丢弃 plan.md、research.md、data-model.md、quickstart.md、contracts/
.specify/scripts/powershell/speckit-reset.ps1 -To before-plan

# 重置到 /speckit.tasks 之前：丢弃 tasks.md
.specify/scripts/powershell/speckit-reset.ps1 -To before-tasks

# 重置到 /speckit.implement 之前：丢弃所有未提交的修改（git reset --hard HEAD）
.specify/scripts/powershell/speckit-reset.ps1 -To before-implement
# 跳过确认提示：
.specify/scripts/powershell/speckit-reset.ps1 -To before-implement -Force
```

**说明**：

- **before-plan**：仅还原/删除当前特性目录下由 `/speckit.plan` 产生的文件（`plan.md`、`research.md`、`data-model.md`、`quickstart.md`、`contracts/`）。已纳入版本控制的会从 `HEAD` 恢复，未跟踪的会删除。
- **before-tasks**：仅还原/删除当前特性目录下的 `tasks.md`（由 `/speckit.tasks` 产生）。
- **before-implement**：对整个仓库执行 `git reset --hard HEAD`，丢弃所有未提交修改；会提示确认（可用 `-Force` 跳过）。

**前提**：当前为 Git 仓库，且已切换到对应特性分支。

---

## 2. 使用 Git 手动重置

若在运行 speckit 命令**之前**做过提交或打标签，可直接用 Git 回退到该提交/标签：

```bash
# 回退到指定提交（丢弃该提交之后的所有修改）
git reset --hard <commit>

# 示例：回退到上一个提交
git reset --hard HEAD~1

# 若曾打标签（例如运行 plan 前执行过 git tag speckit-before-plan）
git reset --hard speckit-before-plan
```

**建议习惯**：在运行 `/speckit.plan`、`/speckit.tasks` 或 `/speckit.implement` 前先做一次提交或打标签，便于精确回退：

```bash
git add -A
git commit -m "checkpoint before speckit.plan"
# 然后执行 /speckit.plan；若要回退：git reset --hard HEAD~1
```

---

## 3. 仅丢弃某几类文件的未提交修改

不运行脚本、仅用 Git 还原部分路径到 `HEAD` 状态（不删除未跟踪文件）：

```bash
# 仅还原 plan 相关文件（需替换 001-xxx 为当前特性目录名）
git checkout HEAD -- specs/001-xxx/plan.md specs/001-xxx/research.md specs/001-xxx/data-model.md specs/001-xxx/quickstart.md specs/001-xxx/contracts/

# 仅还原 tasks.md
git checkout HEAD -- specs/001-xxx/tasks.md

# 还原整个工作区到上次提交（丢弃所有未提交修改）
git reset --hard HEAD
```

---

## 4. 小结

| 目标状态           | 推荐方式 |
|--------------------|----------|
| plan 之前          | `speckit-reset.ps1 -To before-plan` |
| tasks 之前         | `speckit-reset.ps1 -To before-tasks` |
| implement 之前    | `speckit-reset.ps1 -To before-implement` 或 `git reset --hard HEAD` |
| 精确回到某次提交   | 事先提交/打标签，再 `git reset --hard <ref>` |

脚本帮助信息：

```powershell
.specify/scripts/powershell/speckit-reset.ps1 -Help
```
