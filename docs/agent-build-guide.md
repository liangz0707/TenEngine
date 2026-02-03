# 子模块构建与自动测试：Agent 指南（通用）

**用途**：通用说明与可直接复制给 Agent 的提示词模板，使任意子模块在**自动测试时自动引入依赖**并生成合规工程。规约见 `docs/engine-build-module-convention.md`。

---

## 〇、强制规则（不可违反）

- **依赖子模块不存在时须直接报错**：若本模块依赖的**子模块**（上游 worktree 或 `TENENGINE_xxx_DIR` 指向的目录）**不存在**或无法解析，构建**必须直接报错**（例如 CMake 在 `tenengine_resolve_my_dependencies` 或 `add_subdirectory` 阶段报错并退出），**不得**静默跳过或继续构建。
- **禁止创建占位符**：**禁止**为缺失的子模块创建占位符、stub、mock 或空目录/空目标以“通过构建”。缺失依赖只能通过**引入真实子模块源码**（克隆/拉取对应 worktree 或设置正确的 `TENENGINE_xxx_DIR`）解决。
- **build 测试时不引入子分支的测试工程**：构建/运行本模块测试时，**不引入、不构建**依赖子模块（上游 worktree）中的**测试工程**；仅构建本模块 `tests/` 下的测试，测试可执行文件只 link 本模块 target。不得对上游子模块执行 `add_subdirectory(上游/tests)` 或启用其 `enable_testing()`/测试 target。**不需要修改上游工程的 CMake 文件**：仅在本模块的 CMakeLists.txt 中不引入上游的 tests 即可；上游模块可保持其自身 tests/ 与 CMake 不变。

以上与 `specs/_contracts/README.md`、`.specify/memory/constitution.md` §VI 及 `docs/engine-build-module-convention.md` 一致。

---

## 一、通用前置条件

| 项目 | 说明 |
|------|------|
| **每个分支都有 cmake/** | 各模块 worktree 根目录下有 `cmake/TenEngineHelpers.cmake`、`cmake/TenEngineModuleDependencies.cmake`；构建时从**同级上游**的 cmake 引用，不依赖主仓库。 |
| **当前目录** | 在目标模块的 worktree 根目录下操作（如 `TenEngine-NNN-modulename`）。 |
| **TENENGINE_CMAKE_DIR** | 无上游：本目录 `cmake`。有上游：同级上游的 cmake，如 `"${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake"`。 |
| **依赖方式** | **统一使用源码**：同级存在上游 worktree 或通过 `TENENGINE_xxx_DIR` 指定；不使用 DLL/预编译库。 |

---

## 二、Agent 要生成或修改的文件（通用）

| 文件 | 作用 |
|------|------|
| **CMakeLists.txt**（模块根目录） | 设置 `TENENGINE_CMAKE_DIR`、`include(TenEngineHelpers.cmake)`、`tenengine_resolve_my_dependencies("<本模块 id>" ...)`、声明本模块库并 `target_link_libraries(本模块 PRIVATE ${MY_DEPS})`、用 **tenengine_add_module_test** 添加测试（只 link 本模块 target）。**不**引入子分支/上游的 tests 目录或测试工程。 |
| **tests/test_xxx.cpp**（可选） | 测试源文件；若不存在则生成最小占位。**测试逻辑须能覆盖**：① 上游模块能力（通过本模块接口间接调用或直接调用上游 API）；② 若依赖第三方库，须包含对第三方库的实际调用以验证集成有效。不得仅测本模块孤立逻辑。 |

**禁止**手写 `add_subdirectory` / `find_package` 分支；**只**通过 `tenengine_resolve_my_dependencies` 与 CMake 变量引入依赖。测试只 link 本模块 target；**build 测试时不引入子分支的测试工程**（不 `add_subdirectory(上游/tests)`，不启用上游的测试 target）。**测试内容**：须覆盖上游模块能力与第三方库调用能力，不限于本模块孤立逻辑。

---

## 三、可直接复制给 Agent 的提示词（通用模板）

在当前模块 worktree 根目录下，将下面整段复制给 Agent；**把占位符替换为实际值**：

```
当前在 TenEngine-<NNN-modulename>（<NNN-modulename> 模块）根目录。请按以下要求配置 CMake，使「自动测试」时自动引入依赖库，并生成可用的工程。

1) 阅读文档：docs/engine-build-module-convention.md、docs/agent-build-guide.md（含「〇、强制规则」）。构建脚本在**每个分支的 cmake/** 中；本模块构建时从**同级上游**的 cmake 引用，路径为：${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-<上游模块目录名>/cmake（无上游则用本目录 cmake）。

2) **依赖子模块不存在时**：若某依赖的上游子模块目录不存在或无法解析，CMake 配置**必须直接报错并退出**；**禁止**创建占位符、stub 或空目标以绕过缺失依赖。

3) 在本模块根目录创建或修改 CMakeLists.txt，要求：
   - 使用 cmake_minimum_required(VERSION 3.16)、project(TenEngine-<NNN-modulename> LANGUAGES CXX)、set(CMAKE_CXX_STANDARD 17)。
   - 设置 TENENGINE_CMAKE_DIR 并 include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)。
   - 调用 tenengine_resolve_my_dependencies("<NNN-modulename>" OUT_DEPS MY_DEPS)。
   - 声明本模块库：add_library(<te_模块名> STATIC ...)，target_include_directories(...)，target_link_libraries(<本模块> PRIVATE ${MY_DEPS})。
   - 使用 tenengine_add_module_test(NAME <测试名> MODULE_TARGET <本模块 target> SOURCES tests/<测试源文件> ENABLE_CTEST)。不要对测试可执行文件再写 target_link_libraries 依赖上游；只 link 本模块。
   - **build 测试时不引入子分支的测试工程**：不 add_subdirectory 上游的 tests、不启用上游的 enable_testing/测试 target；仅构建本模块 tests/ 下的测试。
   - 若测试源文件不存在，则创建最小可编译占位（**仅限测试源文件**；依赖的子模块缺失时仍须报错，不得占位）。
   - **测试逻辑须覆盖**：① 上游模块能力（调用本模块接口或直接调用上游 API）；② 若依赖第三方库，须包含对第三方库的实际调用（如 spdlog::info、glm::vec3、stb_image_load 等）以验证集成有效。不得仅测本模块孤立逻辑。

4) 确认：测试可执行文件只 link 本模块 target，依赖通过本模块的 target_link_libraries 自动传递；不引入、不构建上游子模块的测试工程。测试代码应主动调用上游与第三方 API。

请直接生成或修改 CMakeLists.txt 和（若需要）测试源文件，并简要说明配置后如何执行 cmake -B build 与 cmake --build build 以验证。
```

**占位符**：`<NNN-modulename>`（如 001-core、002-object）、`<上游模块目录名>`（如 TenEngine-001-core）、`<te_模块名>`（如 te_core、te_object）、`<测试名>`、`<测试源文件>`。

---

## 四、配置后验证（通用）

在模块根目录执行：

```powershell
cmake -B build
cmake --build build
```

构建产出在 **`<模块根目录>/build/`**。运行测试可执行文件或 `cd build && ctest`（若启用了 ENABLE_CTEST）。
