# 子模块构建与自动测试：Agent 指南（通用）

**用途**：通用说明与可直接复制给 Agent 的提示词模板，使任意子模块在**自动测试时自动引入依赖**并生成合规工程。规约见 `docs/engine-build-module-convention.md`。

---

## 一、通用前置条件

| 项目 | 说明 |
|------|------|
| **每个分支都有 cmake/** | 各模块 worktree 根目录下有 `cmake/TenEngineHelpers.cmake`、`cmake/TenEngineModuleDependencies.cmake`；构建时从**同级上游**的 cmake 引用，不依赖主仓库。 |
| **当前目录** | 在目标模块的 worktree 根目录下操作（如 `TenEngine-NNN-modulename`）。 |
| **TENENGINE_CMAKE_DIR** | 无上游：本目录 `cmake`。有上游：同级上游的 cmake，如 `"${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake"`。 |
| **依赖方式** | 源码：同级存在上游 worktree 或通过 `TENENGINE_xxx_DIR` 指定。DLL：配置时传 `TENENGINE_xxx_USE_SOURCE=OFF` 与 `TENENGINE_xxx_PREFIX`。 |

---

## 二、Agent 要生成或修改的文件（通用）

| 文件 | 作用 |
|------|------|
| **CMakeLists.txt**（模块根目录） | 设置 `TENENGINE_CMAKE_DIR`、`include(TenEngineHelpers.cmake)`、`tenengine_resolve_my_dependencies("<本模块 id>" ...)`、声明本模块库并 `target_link_libraries(本模块 PRIVATE ${MY_DEPS})`、用 **tenengine_add_module_test** 添加测试（只 link 本模块 target）。 |
| **tests/test_xxx.cpp**（可选） | 测试源文件；若不存在则生成最小占位。 |

**禁止**手写 `add_subdirectory` / `find_package` 分支；**只**通过 `tenengine_resolve_my_dependencies` 与 CMake 变量引入依赖。测试只 link 本模块 target。

---

## 三、可直接复制给 Agent 的提示词（通用模板）

在当前模块 worktree 根目录下，将下面整段复制给 Agent；**把占位符替换为实际值**：

```
当前在 TenEngine-<NNN-modulename>（<NNN-modulename> 模块）根目录。请按以下要求配置 CMake，使「自动测试」时自动引入依赖库，并生成可用的工程。

1) 阅读文档：docs/engine-build-module-convention.md、docs/agent-build-guide.md。构建脚本在**每个分支的 cmake/** 中；本模块构建时从**同级上游**的 cmake 引用，路径为：${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-<上游模块目录名>/cmake（无上游则用本目录 cmake）。

2) 在本模块根目录创建或修改 CMakeLists.txt，要求：
   - 使用 cmake_minimum_required(VERSION 3.16)、project(TenEngine-<NNN-modulename> LANGUAGES CXX)、set(CMAKE_CXX_STANDARD 17)。
   - 设置 TENENGINE_CMAKE_DIR 并 include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)。
   - 调用 tenengine_resolve_my_dependencies("<NNN-modulename>" OUT_DEPS MY_DEPS)。
   - 声明本模块库：add_library(<te_模块名> STATIC ...)，target_include_directories(...)，target_link_libraries(<本模块> PRIVATE ${MY_DEPS})。
   - 使用 tenengine_add_module_test(NAME <测试名> MODULE_TARGET <本模块 target> SOURCES tests/<测试源文件> ENABLE_CTEST)。不要对测试可执行文件再写 target_link_libraries 依赖上游；只 link 本模块。
   - 若测试源文件不存在，则创建最小可编译占位。

3) 确认：测试可执行文件只 link 本模块 target，依赖通过本模块的 target_link_libraries 自动传递。

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
