# 双模块试点工作流：001-Core → 002-Object（含 Spec Kit）

以 **001-Core**（无依赖）和 **002-Object**（仅依赖 Core）为试点，结合 [Spec Kit](https://github.com/github/spec-kit) 的 Spec-Driven Development 流程，跑通「契约 → API 雏形 → 规约（已有）→ 计划 → 任务 → 实现 → 下游消费」。

---

## 0. 规约来源：直接使用 docs/module-specs

**模块的完整规约已存在于 `docs/module-specs/`**（如 `001-core.md`、`002-object.md`），包含：模块说明、详细功能、实现难度、资源类型、子模块、上下游、外部依赖等。**不重复造规约**，Spec Kit 的 plan / tasks / implement 以以下为输入：

- **模块规约**：`docs/module-specs/NNN-modulename.md`（如 `docs/module-specs/001-core.md`）
- **契约**：`specs/_contracts/NNN-modulename-public-api.md`（能力列表、类型与句柄、API 雏形）
- **本 feature 范围**：在 `specs/<feature>/spec.md` 中只写「本切片要做的部分」（如「本 feature 实现 001-Core 中的：内存 Alloc/Free、日志 Log」），并**引用**上述模块规约与契约。

这样 plan、tasks、implement 都**直接依据** docs/module-specs 与契约，不在 Spec Kit 里再写一套完整规约。

---

## 1. Spec Kit 指令与调用时机总览

| 指令 | 何时 | 何处（目录 / 分支） | 说明 |
|------|------|---------------------|------|
| **/speckit.constitution** | 项目初始化时（一次性） | **主仓库** `G:\AIHUMAN\WorkSpaceSDD\TenEngine`（任意分支，如 master） | 创建或更新 `.specify/memory/constitution.md`。本项目若已有可跳过。 |
| **规约来源** | 始终 | — | **直接使用** `docs/module-specs/NNN-*.md` 与 `specs/_contracts/NNN-*-public-api.md`，不在 specify 里重写完整模块规约。 |
| **/speckit.specify** 或 **手动创建 feature spec** | 开始做**某个模块的一个交付切片**时 | **对应模块 worktree**（如 `TenEngine-001-core`） | 先拉取 T0-contracts。**二选一**：(1) 运行 /speckit.specify，prompt 中写明「规约见 docs/module-specs/001-core.md 与契约 001-core-public-api.md，本 feature 仅实现最小子集：…」；(2) 或手动创建 feature 分支（如 `001-core-minimal`）和 `specs/001-core-minimal/spec.md`，内容为「本 feature 范围 + 引用 docs/module-specs/001-core.md 与 specs/_contracts/001-core-public-api.md」。Spec Kit 要求 feature 分支名为 `001-xxx`。 |
| **/speckit.clarify** | specify 之后、**plan 之前**（推荐） | **同一 worktree，当前 feature 分支** | 澄清时以 **docs/module-specs** 与契约为准，不偏离已有规约。 |
| **/speckit.plan** | clarify 之后 | **同一 worktree，当前 feature 分支** | Prompt 中写明：技术栈；**以 docs/module-specs/NNN-*.md 与契约为规约输入**；仅使用契约中已声明的类型与接口；**plan 须按 docs/build-module-convention.md 在 plan.md 中澄清「依赖引入方式」**（每个直接依赖：源码 / DLL / 不引入外部库，默认源码）；**plan 结束时必须产出一份「契约更新」**：列出本模块对外 API（函数签名、类型），供写回契约。生成 plan.md 等。 |
| **写回契约**（**必须**） | **plan 之后、tasks 之前** | **主仓库 T0-contracts**，然后 worktree 拉取 | 将 plan 产出的对外接口（plan.md 或 contracts/ 中的 API）**同步到** `specs/_contracts/NNN-*-public-api.md` 的「API 雏形」小节；在 T0-contracts 上提交并推送；worktree 再 `git merge origin/T0-contracts`。这样 tasks/implement 与下游都以契约为准。 |
| **/speckit.tasks** | 写回契约之后 | **同一 worktree，当前 feature 分支** | 根据 plan 生成 tasks.md；此时契约已含 plan 产出的 API 雏形。 |
| **/speckit.analyze** | tasks 之后、implement 之前（可选） | **同一 worktree，当前 feature 分支** | 检查与 docs/module-specs、契约的一致性。 |
| **/speckit.implement** | tasks（及可选 analyze）之后 | **同一 worktree，当前 feature 分支** | 按 tasks.md 执行实现。 |
| **契约更新（API 雏形）** | 上游模块 **plan 之前**（或与 feature 创建并行） | **主仓库**，分支 **T0-contracts** | 编辑 `specs/_contracts/NNN-*-public-api.md`，增加「API 雏形」等；提交并推送。非 Spec Kit 指令，但必须做。 |

**重要**：Spec Kit 的 plan/tasks/implement 依赖 **feature 分支名** 为 `001-xxx`，且 `FEATURE_DIR = specs/<当前分支名>`。feature 下的 **spec.md 只描述本切片范围并引用 docs/module-specs 与契约**；完整规约**直接使用** docs/module-specs，不重复写。**构建规约**：任何编译、测试的 build **必须**按 `docs/build-module-convention.md` 的方式构建依赖与测试/可执行目标；Agent **禁止**使用与该规约冲突的构建方式。

---

## 2. 模块与依赖

| 模块 | 依赖 | 契约 | Worktree |
|------|------|------|----------|
| **001-Core** | 无（根） | `001-core-public-api.md` | `G:\AIHUMAN\WorkSpaceSDD\TenEngine-001-core` |
| **002-Object** | 001-Core | `001-core-public-api.md`（读）、`002-object-public-api.md`（写） | `G:\AIHUMAN\WorkSpaceSDD\TenEngine-002-object` |

下游实现时**只使用** 001-Core 契约中已声明的类型与接口。

---

## 3. 阶段 0：准备（一次性）

### 3.1 主仓库：Constitution（若尚未有）

- **何处**：主仓库 `G:\AIHUMAN\WorkSpaceSDD\TenEngine`（可在 master 或 T0-contracts）。
- **何时**：项目初始化时一次。
- **操作**：在 Cursor（或已配置 Spec Kit 的 Agent）中执行：
  ```
  /speckit.constitution 创建本引擎项目的原则：代码质量、测试与 ABI 约定、模块边界以 specs/_contracts 契约为准、多 Agent 协作时从 T0-contracts 拉取契约。
  ```
- 确认 `.specify/memory/constitution.md` 已存在并符合 T0 契约策略（见 `docs/multi-agent-interface-sync.md`）。

### 3.2 两个 worktree：拉取契约

- **何处**：`TenEngine-001-core`、`TenEngine-002-object`。
- **何时**：每次开始在该 worktree 上做新 feature 前。
- **操作**：
  ```powershell
  cd G:\AIHUMAN\WorkSpaceSDD\TenEngine-001-core
  git fetch origin T0-contracts
  git merge origin/T0-contracts

  cd G:\AIHUMAN\WorkSpaceSDD\TenEngine-002-object
  git fetch origin T0-contracts
  git merge origin/T0-contracts
  ```

### 3.3 阅读材料（规约与契约）

- **模块规约**：`docs/module-specs/001-core.md`、`docs/module-specs/002-object.md`（**直接作为 Spec Kit plan/tasks 的规约输入**）。
- **契约**：`specs/_contracts/001-core-public-api.md`、`002-object-public-api.md`：能力、类型与句柄、API 雏形。
- **依赖**：`specs/_contracts/000-module-dependency-map.md`。

---

## 4. 阶段 1：001-Core（上游）

**目标**：在契约中补全 **API 雏形**，再通过 Spec Kit 完成「规格 → 计划 → 任务 → 实现」的最小切片（例如：内存分配 + 日志）。

### 4.1 在 T0-contracts 上补充 Core 的 API 雏形（先于或与 4.2 并行）

- **何时**：在 Core 的 **/speckit.plan 之前**完成，以便 plan/tasks 引用契约。
- **何处**：主仓库 `G:\AIHUMAN\WorkSpaceSDD\TenEngine`，分支 **T0-contracts**。
- **操作**：
  1. `git checkout T0-contracts`（若在主仓库）。
  2. 编辑 `specs/_contracts/001-core-public-api.md`：在「能力列表」后增加 **「API 雏形」** 小节（如 Alloc/Free、Log、容器声明），并在「变更记录」中加一行。
  3. 提交并推送：`git add specs/_contracts/001-core-public-api.md`，`git commit -m "contract(001-core): add API sketch for alloc, log, containers"`，`git push origin T0-contracts`。
  4. 回到 Core worktree，再次 `git fetch origin T0-contracts`、`git merge origin/T0-contracts`（若在 T0-001-core 或 feature 分支上）。

### 4.2 Spec Kit 流程（在 Core worktree，规约用 docs/module-specs）

**何处**：自始至终在 **`G:\AIHUMAN\WorkSpaceSDD\TenEngine-001-core`**。先确保已拉取 T0-contracts（见 3.2）。**规约以 `docs/module-specs/001-core.md` 与契约为准**，不重复写。

| 步骤 | 何时 | Spec Kit 指令 / 动作 | 说明 |
|------|------|----------------------|------|
| 1 | 开始 Core 最小切片 | **创建 feature + spec** | **方式 A**：运行 `/speckit.specify`，prompt 写：<br>「本 feature 的**完整模块规约**见 `docs/module-specs/001-core.md`，**对外 API** 见 `specs/_contracts/001-core-public-api.md`。本 feature 仅实现其中**最小子集**：内存 Alloc/Free、分级日志 Log。spec.md 中引用上述两文件并只描述本切片范围。」<br>**方式 B**：手动创建 feature 分支 `001-core-minimal`、目录 `specs/001-core-minimal/` 和 `specs/001-core-minimal/spec.md`，内容为：本 feature 实现 001-Core 最小子集（Alloc/Free、Log）；完整规约见 docs/module-specs/001-core.md，契约见 specs/_contracts/001-core-public-api.md。 |
| 2 | 之后 | **/speckit.clarify** | 澄清时以 **docs/module-specs/001-core.md** 与契约为准。 |
| 3 | 之后 | **/speckit.plan** | Prompt 写明：技术栈 C++17/CMake；规约输入为 docs/module-specs/001-core.md 与契约；仅暴露契约中已声明的类型与 API；本切片实现 Alloc/Free、Log。**并要求 plan 结束时产出一份「契约更新」**：本模块本切片对外 API（函数签名、类型），用于写回契约。生成 plan.md 等。 |
| **3.5** | **plan 之后、tasks 之前** | **写回契约（必须）** | 见下方 **4.2.5 将 plan 产出的接口写回契约**。 |
| 4 | 之后 | **/speckit.tasks** | 根据 plan 生成 tasks.md（此时契约已含 plan 的 API 雏形）。 |
| 5 | 可选 | **/speckit.analyze** | 检查与 docs/module-specs、契约的一致性。 |
| 6 | 最后 | **/speckit.implement** | 按 tasks.md 执行实现。 |

### 4.2.5 将 plan 产出的接口写回契约（必须）

**目的**：保证 plan 生成的对外接口**一定**出现在契约中，下游与后续 tasks/implement 都以契约为准。

**何时**：**/speckit.plan 完成后、/speckit.tasks 之前**。

**步骤**：

1. **从 plan 产物中提取对外 API**
   - 打开 `specs/<feature>/plan.md`（以及 plan 可能生成的 `specs/<feature>/contracts/`、data-model.md 等）。
   - 提取本模块**对外**暴露的接口：函数签名、类型名、枚举、错误码等（与本 feature 切片一致即可，不必全模块）。
   - 若 plan 按 prompt 产出了「契约更新」清单，直接使用该清单。

2. **在主仓库 T0-contracts 上更新契约**
   - 打开**主仓库** `G:\AIHUMAN\WorkSpaceSDD\TenEngine`，`git checkout T0-contracts`，`git pull origin T0-contracts`。
   - 编辑 `specs/_contracts/NNN-modulename-public-api.md`（如 001-core-public-api.md）：
     - 在「能力列表」后补充或更新 **「API 雏形」** 小节，将 plan 产出的接口以简化声明形式写入（如 `void* Alloc(size_t size, size_t alignment);`、`void Free(void* ptr);`）。
     - 在「变更记录」中加一行：日期 +「API 雏形：由 plan（feature 001-core-minimal）同步」。
   - 提交并推送：`git add specs/_contracts/...`，`git commit -m "contract(001-core): sync API sketch from plan 001-core-minimal"`，`git push origin T0-contracts`。

3. **在 worktree 拉取更新后的契约**
   - 在 **TenEngine-001-core**（当前 feature 分支）：`git fetch origin T0-contracts`，`git merge origin/T0-contracts`。
   - 确认 `specs/_contracts/001-core-public-api.md` 已含刚写回的 API 雏形。

4. **再跑 /speckit.tasks**
   - 此后 tasks 与 implement 都以**已更新契约**为准，实现时只暴露契约中的 API。

5. **（必须）若契约变更影响下游，执行 Follow-up**
   - 打开 `specs/_contracts/000-module-dependency-map.md`，在「谁被谁依赖」表中查出**依赖本模块**的下游列表。
   - 若本次契约变更可能影响下游（如 API 签名变更、删除、行为变更），则对**每个下游**在其规约中增加待办：
     - 在对应下游的 **`docs/module-specs/NNN-modulename.md`** 中增加一条待办，如：`- **待办**：需随 \`001-core\` 契约变更做适配（契约变更日期：YYYY-MM-DD；变更摘要：…）。`
   - **仅采用规约待办**：不直接修改下游的 feature 分支（如 checklist、tasks.md），以便多分支下通过规约文件同步即可；下游根据规约中的待办与契约变更记录，自行判断适配时间与修改内容。
   - 若无下游或仅为新增、无破坏性变更，可跳过本步。

**Prompt 约定**：在调用 /speckit.plan 时，在 prompt 末尾加上一句：**「Plan 结束时请产出一份「契约更新」：列出本 feature 对外暴露的函数签名与类型，格式可直接用于写入 specs/_contracts/NNN-*-public-api.md 的 API 雏形小节。」** 这样 Agent 会在 plan 输出中给出写回契约所需内容，便于执行步骤 1–2。

### 4.3 合并回模块分支并推送

- **何时**：implement 完成后。
- **何处**：仍在 **TenEngine-001-core**。
- **操作**：
  ```powershell
  git checkout T0-001-core
  git merge 001-core-minimal -m "Merge feature 001-core-minimal (Spec Kit)"
  git push origin T0-001-core
  ```
  （若 feature 分支名为其他，替换 `001-core-minimal`。）

---

## 5. 阶段 2：002-Object（下游）

**目标**：只使用 001-Core 契约中的类型与接口，通过 Spec Kit 完成 Object 的**最小子集**（例如：类型注册 + 简单序列化）。

### 5.1 拉取契约

- **何处**：`G:\AIHUMAN\WorkSpaceSDD\TenEngine-002-object`。
- **操作**：`git fetch origin T0-contracts`，`git merge origin/T0-contracts`。阅读 `specs/_contracts/001-core-public-api.md`。

### 5.2 可选：为 002-Object 补充 API 雏形

- **何时**：Object 的 plan 之前。
- **何处**：主仓库 **T0-contracts**。编辑 `specs/_contracts/002-object-public-api.md`，增加「API 雏形」小节，推送后回到 Object worktree 拉取。

### 5.3 Spec Kit 流程（在 Object worktree，规约用 docs/module-specs）

**何处**：自始至终在 **`G:\AIHUMAN\WorkSpaceSDD\TenEngine-002-object`**。**规约以 `docs/module-specs/002-object.md` 与契约（001-core、002-object）为准**。

| 步骤 | 何时 | Spec Kit 指令 / 动作 | 说明 |
|------|------|----------------------|------|
| 1 | 开始 Object 最小切片 | **创建 feature + spec** | **方式 A**：/speckit.specify，prompt 写：本 feature 的**模块规约**见 `docs/module-specs/002-object.md`，**依赖的 Core API** 见 `specs/_contracts/001-core-public-api.md`，本 feature 仅实现最小子集（类型注册、简单序列化）；spec.md 引用上述文件并只描述本切片。<br>**方式 B**：手动创建 feature 分支 `002-object-minimal` 和 `specs/002-object-minimal/spec.md`，引用 docs/module-specs/002-object.md 与两个契约。 |
| 2 | 之后 | **/speckit.clarify** | 以 docs/module-specs/002-object.md 与契约为准。 |
| 3 | 之后 | **/speckit.plan** | Prompt 写明：规约输入为 docs/module-specs/002-object.md 与契约；仅使用 001-Core 契约中已声明的类型与 API；技术栈 C++/CMake。**并要求 plan 结束时产出一份「契约更新」**（本模块本切片对外 API），用于写回 002-object-public-api.md。 |
| **3.5** | **plan 之后、tasks 之前** | **写回契约（必须）** | 将 plan 产出的对外接口同步到 `specs/_contracts/002-object-public-api.md` 的「API 雏形」；在 T0-contracts 上提交并推送；worktree 拉取后再跑 tasks。 |
| 4 | 之后 | **/speckit.tasks** | 生成 tasks.md。 |
| 5 | 可选 | **/speckit.analyze** | 与 docs/module-specs、契约一致性。 |
| 6 | 最后 | **/speckit.implement** | 执行实现。 |

### 5.4 合并回模块分支并推送

- **何处**：**TenEngine-002-object**。
- **操作**：`git checkout T0-002-object`，`git merge 002-object-minimal -m "Merge feature 002-object-minimal (Spec Kit)"`，`git push origin T0-002-object`。

---

## 6. 阶段 3：契约变更时的同步（可选）

- 若在实现 **001-Core** 时**修改了对外 API**：
  1. 在 **T0-contracts** 上更新 `001-core-public-api.md`（能力列表 / API 雏形 / 变更记录），推送。
  2. 在 `000-module-dependency-map.md` 中确认下游（含 002-Object）；若有破坏性变更，在 `docs/module-specs/002-object.md` 中增加**待办**（仅规约待办，见 `docs/multi-agent-interface-sync.md` §4.4）。
  3. 002-Object 侧：拉取 T0-contracts，根据契约变更做适配（可再跑一轮 specify/plan/tasks 或直接改代码）。

---

## 6.5 002 如何引用 001（构建与链接）

在 001、002 均已实现后，要让 002 在**编译与链接**时使用 001 的内容，可采用以下两种方式之一（或按工程已有约定）。002 源码中**只使用** `specs/_contracts/001-core-public-api.md` 中声明的类型与 API；头文件路径与命名以 001 实际实现为准。

### 方式一：同机多 worktree，CMake 直接引用 001 源码目录

假设 worktree 与主仓库为同级目录（如 `G:\AIHUMAN\WorkSpaceSDD\TenEngine-001-core`、`TenEngine-002-object`）。在 **002 的 CMakeLists.txt** 中：

1. **把 001 的根目录加入子工程**（可用变量便于改路径）：
   ```cmake
   set(TENENGINE_001_CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core" CACHE PATH "Path to 001-Core worktree")
   add_subdirectory(${TENENGINE_001_CORE_DIR} ${CMAKE_BINARY_DIR}/tenengine_001_core)
   ```
2. **002 的 target 链接 001 的 library，并带上 001 的 include**：
   - 若 001 导出的 target 名为 `te_core`（或 `TenEngine::Core` 等）：
     ```cmake
     target_link_libraries(te_object PRIVATE te_core)
     ```
   - 若 001 已用 `target_include_directories(te_core PUBLIC <path>)`，002 会自动获得 include；否则在 002 中显式：
     ```cmake
     target_include_directories(te_object PRIVATE ${TENENGINE_001_CORE_DIR}/include)
     ```

002 源码中按 001 契约的 API 包含头文件，例如：`#include <tenengine/core/allocator.hpp>` 或 001 实际提供的路径。

### 方式二：先构建并安装 001，002 再引用安装结果

1. **在 001 的 worktree 中构建并安装到统一前缀**：
   ```powershell
   cd G:\AIHUMAN\WorkSpaceSDD\TenEngine-001-core
   cmake -B build -DCMAKE_INSTALL_PREFIX=G:/AIHUMAN/WorkSpaceSDD/tenengine-install
   cmake --build build
   cmake --install build
   ```
2. **在 002 的 CMake 中通过前缀找到 001**：
   - 若 001 安装了 `TenEngineCoreConfig.cmake`：`find_package(TenEngineCore REQUIRED)`，再 `target_link_libraries(te_object PRIVATE TenEngine::Core)`。
   - 否则手动指定 include 与 lib：
     ```cmake
     set(TENENGINE_INSTALL_DIR "G:/AIHUMAN/WorkSpaceSDD/tenengine-install" CACHE PATH "TenEngine install prefix")
     target_include_directories(te_object PRIVATE ${TENENGINE_INSTALL_DIR}/include)
     target_link_directories(te_object PRIVATE ${TENENGINE_INSTALL_DIR}/lib)
     target_link_libraries(te_object PRIVATE tenengine_core)   # 或 001 导出的库名
     ```

适合 CI 或“先装 001、再编 002”的流程。

### 小结

| 项目 | 说明 |
|------|------|
| **接口边界** | 002 只使用契约 `001-core-public-api.md` 中声明的类型与 API；不依赖 001 内部实现细节。 |
| **include** | 002 的编译必须能找到 001 的**公开头文件**（方式一通过 001 的 target 或显式路径，方式二通过 install 的 include）。 |
| **链接** | 002 的 target 须链接 001 的库（静态库或动态库，与 001 的 CMake 导出一致）。 |

**统一约定（源码 + DLL，测试/工程自动引入依赖）**：所有子模块在单独目录下构建时，应同时支持「源码 include」与「预构建 DLL」两种依赖方式，并在构建测试/可执行工程时**只 link 本模块 target**，由 CMake 传递依赖。构建脚本在**每个分支的 cmake/** 中，从**同级上游**的 cmake 引用，不依赖主仓库。详见 **`docs/build-module-convention.md`**（含 `tenengine_resolve_my_dependencies`、`tenengine_add_module_test` / `tenengine_add_module_app` 用法）。

---

## 6.6 子工程嵌套与长依赖树（27 模块）的构建策略

### 子工程是否支持嵌套

**支持。** CMake 的 `add_subdirectory` 可以嵌套：002 里可以 `add_subdirectory(001)`，003 里可以 `add_subdirectory(001)` 和 `add_subdirectory(002)`（或只 `add_subdirectory(002)` 若 002 已传递依赖 001）。依赖关系是 **DAG**，同一依赖可被多处 `add_subdirectory`，CMake 会复用同一 target，不会重复编译。

注意：若 002、003 都 `add_subdirectory(001)`，两次加入的路径必须一致（否则会生成两份 001 target）。推荐做法见下「方式一」：**由顶层统一 add_subdirectory，子工程只声明 target_link_libraries**。

### 长依赖树下「在 build 过程中配置依赖工程」的两种方式

T0 有 27 个模块、多层级依赖（见 `specs/_contracts/000-module-dependency-map.md`）。有两种常见做法，都不需要手写「先配置 001、再配置 002、再…」的脚本。

#### 方式一：单次配置、一次构建（推荐：同机多 worktree 或 monorepo）

**思路**：顶层只有一个 CMake 入口，按依赖顺序把**所有**模块都 `add_subdirectory` 进来；每个模块的 CMakeLists.txt **只声明本模块的 target 和 `target_link_libraries(本模块 PRIVATE 直接依赖…)`**，不（或不必）在自己里面 `add_subdirectory` 上游。

- **谁来做 add_subdirectory**：在主仓库或一个「聚合构建」目录放一个顶层 `CMakeLists.txt`，根据依赖图按顺序添加各模块（例如先 001，再 002、003，再 004…027；若 004 依赖 001、002，只要 001、002 已在前面 add 过即可）。
- **配置与构建**：只跑一次 `cmake -B build`、一次 `cmake --build build`。CMake 在**配置阶段**会处理所有 `add_subdirectory`，在**构建阶段**按 target 依赖自动决定编译顺序（先 001，再 002，再 004…），无需在 build 过程中再「启动配置」别的工程。
- **路径**：各模块可以是 worktree 路径（如 `../TenEngine-001-core`），或主仓库下的子目录（如 `modules/001-core`）；顶层用变量或列表统一写一遍即可。

示例（顶层 CMakeLists.txt 片段，路径按实际调整）：

```cmake
# 按依赖顺序添加：无依赖的先加，有依赖的后加
set(TENENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})  # 或 worktrees 的公共父路径
add_subdirectory(${TENENGINE_ROOT}/TenEngine-001-core ${CMAKE_BINARY_DIR}/001-core)
add_subdirectory(${TENENGINE_ROOT}/TenEngine-002-object ${CMAKE_BINARY_DIR}/002-object)
# 004 依赖 001,002；001/002 已加入，004 里只需 target_link_libraries(te_scene PRIVATE te_core te_object)
add_subdirectory(${TENENGINE_ROOT}/TenEngine-004-scene ${CMAKE_BINARY_DIR}/004-scene)
# ... 其余模块同理
```

这样**不需要**在 build 过程中再启动配置依赖工程；一次配置、一次构建即可。

#### 方式二：Super-build（在 build 时配置并构建依赖）

若希望「每个模块是独立工程、在**构建时**才配置并编译其依赖」（例如 002 的 build 会触发先配置并 build 001），可用 **ExternalProject** 或 **FetchContent** 在**构建阶段**跑 CMake。

- **ExternalProject_Add**：在顶层（或 002 的）CMake 里为 001 写一个 `ExternalProject_Add(te_core_ep ...)`，指定 001 的源码路径、build 目录、install 前缀；再为 002 写一个 `ExternalProject_Add(te_object_ep DEPENDS te_core_ep ...)`。执行 `cmake --build .` 时，会先配置并编译 001，再配置并编译 002。适合「每个模块单独一个 repo 或独立 build 目录、希望 build 时自动拉齐依赖」的场景。
- **FetchContent**：若依赖来自网络或统一仓库，可用 `FetchContent_Declare` + `FetchContent_MakeAvailable` 在**配置阶段**拉取并 add_subdirectory，本质仍是「一次配置、一次构建」，只是依赖源码由 CMake 自动取回。

小结：若 27 个模块都在本机（或同一聚合目录），**推荐方式一**，结构简单、一次配置一次构建；若坚持「每个模块在 build 时才配置其依赖」，用方式二的 ExternalProject 链式 DEPENDS。

### 小结

| 问题 | 结论 |
|------|------|
| 子工程是否支持嵌套 | 支持；`add_subdirectory` 可多层嵌套，依赖为 DAG，同一路径只应 add 一次，由顶层统一 add 更清晰。 |
| 长依赖树如何少手写「先配 A 再配 B」 | **方式一**：顶层按依赖顺序 `add_subdirectory` 全部模块，一次配置、一次构建，构建顺序由 target 依赖自动决定。**方式二**：用 ExternalProject_Add 的 DEPENDS 在 build 时依次配置并构建 001→002→…。 |

---

## 7. 检查清单（每个 feature 开始前）

- [ ] 当前 worktree 已执行 `git fetch origin T0-contracts` + `git merge origin/T0-contracts`
- [ ] 上游契约（本模块依赖的）已包含需要的 **API 雏形**
- [ ] Spec Kit 在 **feature 分支**（`001-xxx` / `002-xxx`）上运行，plan/tasks/implement 使用 `specs/<feature>/`
- [ ] /speckit.plan 的 prompt 中写明：**仅使用契约中已声明的类型与接口**
- [ ] 实现完成后将 feature 分支 **合并回 T0-NNN-modulename** 并推送
- [ ] 修改对外 API 时：先在 T0-contracts 更新契约并推送，再改代码或通知下游

---

## 8. 小结：何时何处调用

| 阶段 | 何处 | 调用/动作 |
|------|------|------------|
| 0 | 主仓库 | /speckit.constitution（一次性）；两个 worktree 拉取 T0-contracts |
| **规约来源** | — | **直接使用** `docs/module-specs/NNN-*.md` 与契约；feature 下 spec.md 只写本切片范围并引用上述文件 |
| 1 契约 | 主仓库 T0-contracts | 编辑 001-core-public-api.md 增加 API 雏形，推送 |
| 1 Spec Kit | TenEngine-001-core | 创建 feature + spec → clarify → plan（规约输入为 docs/module-specs + 契约，**plan 产出契约更新**）→ **写回契约（T0-contracts）** → tasks → [analyze] → implement；合并 feature 到 T0-001-core，推送 |
| 2 契约 | 主仓库 T0-contracts | 可选：002-object-public-api.md 增加 API 雏形 |
| 2 Spec Kit | TenEngine-002-object | 拉取契约后：创建 feature + spec → clarify → plan（规约输入为 docs/module-specs + 契约，**plan 产出契约更新**）→ **写回契约（T0-contracts）** → tasks → [analyze] → implement；合并 feature 到 T0-002-object，推送 |
| 3 | 按需 | 契约变更时在 T0-contracts 更新，下游拉取并适配 |

完成本试点后，可将同一流程复用到其他 L0/L1 模块。Spec Kit 文档与 CLI 见：<https://github.com/github/spec-kit>。
