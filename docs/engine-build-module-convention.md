# CMake 构建规约与目录约定

本文档定义 TenEngine **T0 架构**下各模块的 CMake 构建方式、目录结构与依赖管理约定（**build 规约**）。Agent 在执行 `/speckit.implement` 或任何涉及 CMake 的任务前，**必须**先澄清本文档中的关键配置。

**Build 规约要点**：① **目录**：单仓时所有代码在 **`Engine/TenEngine-NNN-modulename/`** 下（§2.5）；多 worktree 时每模块独立根目录（§2）。② **构建根**：单仓为仓库根，多 worktree 为 worktree 根（§3.1）。③ **依赖**：统一源码方式（§3.2）。④ **Target 命名**：与 ABI 一致（如 `te_core`、`te_rhi`）（§1）。

**章节说明**：构建方式澄清（根目录、依赖方式等）见 **§3**；单仓 Engine 目录规约见 **§2.5**。

---

## 1. 核心原则

| 原则 | 说明 |
|------|------|
| **单模块单 worktree** | 每个模块在独立的 worktree（如 `TenEngine-008-rhi`）中开发；worktree 根目录即构建根目录。 |
| **子模块统一源码构建** | 当前**所有子模块构建均使用源码方式**（`add_subdirectory` 或 `FetchContent` 引入上游）；不使用 DLL/预编译库方式。 |
| **构建须引入子模块代码** | 构建过程**必须**通过引入**真实子模块源码**（`add_subdirectory` 或 `FetchContent` 拉取对应模块）来满足对上游模块的依赖；**不得**用本模块内的 stub、mock 或占位实现代替上游模块（见 `specs/_contracts/README.md` 与 `.specify/memory/constitution.md` §VI）。 |
| **依赖子模块不存在时须直接报错** | 若依赖的**子模块**（上游 worktree 或指定目录）**不存在**或无法解析，构建**必须直接报错**（CMake 配置阶段报错并退出），**禁止**创建占位符、stub、空目标或静默跳过以“通过构建”。详见 `docs/agent-build-guide.md`「〇、强制规则」。 |
| **build 测试时不引入子分支的测试工程** | 构建/运行本模块测试时，**不引入、不构建**依赖子模块（上游 worktree）中的**测试工程**；仅构建本模块 `tests/` 下的测试，测试只 link 本模块 target。不得对上游执行 `add_subdirectory(上游/tests)` 或启用其测试 target。**不需要修改上游工程的 CMake 文件**，仅在本模块 CMake 中不引入上游 tests 即可。详见 `docs/agent-build-guide.md`「〇、强制规则」。 |
| **完整 ABI 实现** | 各模块必须实现其 ABI 文件（`specs/_contracts/NNN-modulename-ABI.md`）中列出的**全部**符号与能力；禁止长期以 stub 或代替方案作为正式实现。 |
| **契约约束接口** | 构建脚本只暴露契约（`specs/_contracts/`）中声明的头文件路径与目标；内部实现不对外导出。 |
| **跨平台** | CMake 脚本须兼容 Windows、Linux、macOS；避免平台特定硬编码路径。 |
| **Target 命名一致性** | 各模块的 CMake target 名称以其 ABI 文档顶部「CMake Target 名称」小节为准（如 001-Core → **`te_core`**，008-RHI → **`te_rhi`**，009-RenderCore → **`te_rendercore`**）。下游在 `target_link_libraries` 中必须使用**实际 target 名称**，不得臆测（如不能用 `TenEngine_Core` 代替 `te_core`）。详见各模块 ABI（`specs/_contracts/NNN-modulename-ABI.md`）。 |

---

## 2. 目录结构（单模块 worktree）

```
TenEngine-NNN-modulename/         # worktree 根目录（即构建根目录）
├── CMakeLists.txt                # 顶层 CMake 配置
├── cmake/                        # CMake 辅助脚本（如 Find*.cmake、平台检测）
├── include/
│   └── TenEngine/
│       └── NNN/                  # 模块公开头文件（与契约对应）
├── src/                          # 模块实现源文件
├── tests/                        # 单元测试 / 集成测试
├── apps/                         # 示例 / 工具应用（可选）
├── deps/                         # 本地依赖源码（可选，若使用 add_subdirectory）
├── specs/                        # 规约与契约（从 T0-contracts 拉取）
│   └── _contracts/
├── docs/                         # 文档（模块级或 worktree 级）
└── .specify/                     # Spec Kit 配置与脚本
```

### 2.5 单仓多模块：Engine 目录规约

当采用**单仓（mono repo）**、所有代码集中在同一仓库时，**全部代码相关目录**须置于 **`Engine/`** 下，按模块分目录；仓库根仅保留顶层 CMake、文档、契约与配置。

| 约定 | 说明 |
|------|------|
| **Engine 为代码根** | 所有可构建模块的源码、头文件、测试、模块级 cmake 与 CMakeLists.txt 均放在 `Engine/` 下，不得在仓库根直接放置 `src/`、`include/`、`tests/`（根目录仅保留 `Engine/`、`docs/`、`specs/`、`cmake/` 等）。 |
| **模块目录命名** | 每个模块占一个子目录：**`Engine/TenEngine-NNN-modulename/`**（如 `Engine/TenEngine-001-core`、`Engine/TenEngine-008-rhi`）。NNN 为三位数模块编号，modulename 与契约/依赖图一致。 |
| **模块内必选目录与文件** | 每个 `Engine/TenEngine-NNN-modulename/` 下须包含：**`src/`**、**`include/`**、**`tests/`**、**`cmake/`**、**`CMakeLists.txt`**。可选：`apps/`、`deps/`。 |
| **根 CMakeLists.txt** | 仓库根目录的 `CMakeLists.txt` 仅做顶层驱动：`project(TenEngine)`、`add_subdirectory(Engine/TenEngine-001-core)`、按需 `add_subdirectory(Engine/TenEngine-NNN-...)`。不在此处定义具体库或可执行文件。 |
| **构建根目录** | 在单仓模式下，**构建根目录**为**仓库根目录**；out-of-source 构建目录推荐为仓库根下的 **`build/`**（如 `TenEngine/build/`）。 |
| **模块 CMake 自包含** | 每个 `Engine/TenEngine-NNN-modulename/CMakeLists.txt` 以**该模块目录为当前作用域**，使用相对路径 `src/`、`include/`、`tests/`；依赖上游模块时通过 `add_subdirectory(../TenEngine-xxx)` 或 TenEngineHelpers 解析同仓内其他 `Engine/TenEngine-xxx`。 |

**单仓目录示例**：

```
TenEngine/                        # 仓库根 = 构建根
├── CMakeLists.txt                # 仅 add_subdirectory(Engine/...)
├── build/                        # out-of-source 构建输出（推荐）
├── Engine/
│   ├── TenEngine-001-core/
│   │   ├── CMakeLists.txt
│   │   ├── src/
│   │   ├── include/
│   │   ├── tests/
│   │   └── cmake/
│   └── TenEngine-008-rhi/
│       ├── CMakeLists.txt
│       ├── src/
│       ├── include/
│       ├── tests/
│       └── cmake/
├── docs/
├── specs/
└── ...
```

**与 §2 的关系**：§2 描述单模块单 worktree（每个模块独立仓库/分支）；本节描述单仓内多模块共存的目录与构建规约。两种方式下，**模块内部**的 `src/`、`include/`、`tests/`、`cmake/`、`CMakeLists.txt` 语义一致，仅顶层驱动方式不同。

---

## 3. 构建方式澄清（implement 前必问）

在执行 `/speckit.implement` 或任何涉及 CMake 生成/配置的任务**之前**，Agent **必须**向用户澄清以下内容（若文档或上下文中无明确说明）：

### 3.1 构建根目录

| 问题 | 说明 |
|------|------|
| **单仓 vs 多 worktree** | **单仓**：构建根目录为**仓库根**（如 `TenEngine/`），代码在 `Engine/TenEngine-NNN-modulename/` 下，见 **§2.5**。**多 worktree**：构建根目录为当前模块 worktree 根（如 `TenEngine-008-rhi/`），见 §2。 |
| **worktree / 仓库路径** | 当前工作的 worktree 或仓库绝对路径（如 `G:\AIHUMAN\WorkSpaceSDD\TenEngine` 或 `TenEngine-008-rhi`）。 |
| **CMakeLists.txt 位置** | 单仓：根目录 `CMakeLists.txt` 仅 `add_subdirectory(Engine/...)`；各模块在 `Engine/TenEngine-NNN-xxx/CMakeLists.txt`。多 worktree：worktree 根目录下的 `CMakeLists.txt`。 |
| **out-of-source build** | 构建输出目录（如 `build/`、`out/`）位于何处？推荐：单仓为 **`{仓库根}/build`**，多 worktree 为 `{worktree}/build`。 |

### 3.2 依赖构建方式

**当前约定**：**所有子模块构建均使用源码方式**，无需在源码/预编译库之间选择。

| 方式 | 说明 | CMake 示例 |
|------|------|------------|
| **源码（add_subdirectory）** | 同级上游 worktree 或 `TENENGINE_xxx_DIR` 指定路径。 | `add_subdirectory(../TenEngine-001-core)` 或由 TenEngineHelpers 解析 |
| **FetchContent** | 从 Git 拉取上游模块源码（可选）。 | `FetchContent_Declare(...)` + `FetchContent_MakeAvailable(...)` |

**Agent 需澄清**：仅需确认**构建根目录**（在哪个 worktree 执行构建）与上游模块路径；依赖方式统一为**源码**。

### 3.3 平台与编译器

| 项目 | 默认 | 可选 |
|------|------|------|
| **编译器** | MSVC（Windows）、Clang/GCC（Linux/macOS） | 可指定 `-DCMAKE_CXX_COMPILER=...` |
| **C++ 标准** | C++17 | 可升级至 C++20 |
| **构建类型** | Debug / Release | `-DCMAKE_BUILD_TYPE=Release` |

---

## 4. CMakeLists.txt 模板（顶层）

```cmake
cmake_minimum_required(VERSION 3.20)
project(TenEngine-NNN-modulename VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === 依赖（当前统一使用源码方式，见 §3.2） ===
# add_subdirectory(deps/001-core) 或由 TenEngineHelpers.cmake / tenengine_resolve_my_dependencies 解析

# === 本模块库 ===
add_library(TenEngine_NNN_modulename
    src/module.cpp
    # ...
)
target_include_directories(TenEngine_NNN_modulename
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
)
target_link_libraries(TenEngine_NNN_modulename
    PUBLIC TenEngine_001_core   # 根据依赖调整
)

# === 测试（可选） ===
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# === 示例 / 工具（可选） ===
option(BUILD_APPS "Build example applications" OFF)
if(BUILD_APPS)
    add_subdirectory(apps)
endif()
```

---

## 5. 测试与应用

### 5.1 测试目录

```
tests/
├── CMakeLists.txt
├── test_alloc.cpp
├── test_log.cpp
└── ...
```

测试使用 `add_test()` 或集成 GoogleTest / Catch2。

**测试逻辑规约**：测试须能覆盖 ① **上游模块能力**（通过本模块接口间接调用或直接调用上游 API，验证依赖链正确）；② **第三方库调用能力**（若本模块依赖第三方库，须包含对第三方库的实际调用以验证集成有效）。不得仅测本模块孤立逻辑。build 测试时不引入子分支的测试工程；测试只 link 本模块 target，依赖经 `target_link_libraries` 传递。

### 5.2 示例应用

```
apps/
├── CMakeLists.txt
└── demo_app/
    ├── main.cpp
    └── ...
```

---

## 6. 与 T0 架构的衔接

| 项目 | 说明 |
|------|------|
| **契约** | 模块公开头文件路径与目标名称须与 `specs/_contracts/NNN-modulename-public-api.md` 一致。 |
| **依赖图** | 上下游依赖须与 `000-module-dependency-map.md` 一致；构建脚本不得引入未声明的依赖。 |
| **worktree** | 每个 worktree 对应一个 T0-NNN-modulename 分支；构建脚本假设 worktree 根目录为构建根目录。 |

---

## 7. 检查清单（构建配置前）

- [ ] 已明确 **单仓（Engine/）** 或 **多 worktree** 模式，以及 **构建根目录**（仓库根或 worktree 根）。
- [ ] 若单仓：根目录仅保留 `CMakeLists.txt`（add_subdirectory Engine/...）、`Engine/`、`docs/`、`specs/` 等；各模块在 `Engine/TenEngine-NNN-modulename/` 下含 `src/`、`include/`、`tests/`、`cmake/`、`CMakeLists.txt`。
- [ ] 已确认对上游模块**统一使用源码方式**（add_subdirectory / FetchContent）。
- [ ] 各模块 `CMakeLists.txt` 中的 `target_include_directories` 与契约中的公开头文件路径一致。
- [ ] `target_link_libraries` 中的依赖与 `000-module-dependency-map.md` 一致。
- [ ] 已配置 **out-of-source build**（如 `build/`）。

---

## 8. Agent 强制规约（TenEngineHelpers 实现）

当使用 **TenEngineHelpers.cmake**、**TenEngineModuleDependencies.cmake** 时，须遵守以下强制约定：

- **用户说「要构建工程」时**：若**根目录**（在哪个模块目录执行构建）不明确，**必须**先问用户，不得擅自假设。**各子模块均使用源码方式**引入依赖。
- **plan.md**：须列出**直接依赖**并设「依赖引入方式」小节；当前均按**源码**方式引入，无需标注 DLL/不引入。
- **禁止**：在 CMakeLists.txt 中手写对上游的 `add_subdirectory` / `find_package` 分支；须通过 `tenengine_resolve_my_dependencies` 与 CMake 变量由脚本统一处理。**禁止**对测试/可执行 target 显式 link 上游；须只 link 本模块 target。**build 测试时不引入子分支的测试工程**：不 `add_subdirectory(上游/tests)`，不启用上游的测试 target。

通用说明与提示词见 **`docs/agent-build-guide.md`**。依赖图见 `specs/_contracts/000-module-dependency-map.md`。

---

## 9. 小结

| 项目 | 说明 |
|------|------|
| **构建根目录** | worktree 根目录（如 `TenEngine-NNN-modulename/`）。 |
| **依赖方式** | **统一使用源码**（add_subdirectory / FetchContent）；implement 前仅需确认构建根目录与上游路径。 |
| **目录结构** | `include/`（公开头）、`src/`（实现）、`tests/`、`apps/`（可选）。 |
| **契约对应** | 公开头文件与目标名须与 `specs/_contracts/` 一致。 |
