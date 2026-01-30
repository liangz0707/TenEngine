# CMake 构建规约与目录约定

本文档定义 TenEngine **T0 架构（27 模块）** 下各模块的 CMake 构建方式、目录结构与依赖管理约定。Agent 在执行 `/speckit.implement` 或任何涉及 CMake 的任务前，**必须**先澄清本文档中的关键配置。

---

## 1. 核心原则

| 原则 | 说明 |
|------|------|
| **单模块单 worktree** | 每个模块在独立的 worktree（如 `TenEngine-008-rhi`）中开发；worktree 根目录即构建根目录。 |
| **依赖源码优先** | 模块间依赖**优先**以源码方式引入（`add_subdirectory` 或 `FetchContent`）；仅当有 ABI/二进制分发需求时使用 DLL/静态库。 |
| **契约约束接口** | 构建脚本只暴露契约（`specs/_contracts/`）中声明的头文件路径与目标；内部实现不对外导出。 |
| **跨平台** | CMake 脚本须兼容 Windows、Linux、macOS；避免平台特定硬编码路径。 |

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

---

## 3. 构建方式澄清（implement 前必问）

在执行 `/speckit.implement` 或任何涉及 CMake 生成/配置的任务**之前**，Agent **必须**向用户澄清以下内容（若文档或上下文中无明确说明）：

### 3.1 构建根目录

| 问题 | 说明 |
|------|------|
| **worktree 路径** | 当前工作的 worktree 绝对路径（如 `G:\AIHUMAN\WorkSpaceSDD\TenEngine-008-rhi`）。 |
| **CMakeLists.txt 位置** | 是否使用 worktree 根目录下的 `CMakeLists.txt`？若有嵌套，需明确。 |
| **out-of-source build** | 构建输出目录（如 `build/`、`out/`）位于何处？推荐 `{worktree}/build`。 |

### 3.2 依赖构建方式

| 依赖方式 | 适用场景 | CMake 示例 |
|----------|----------|------------|
| **源码（add_subdirectory）** | 需要调试上游、频繁迭代、CI 全量构建。 | `add_subdirectory(deps/001-core)` |
| **FetchContent** | 同上，但从 Git 拉取而非本地路径。 | `FetchContent_Declare(core GIT_REPOSITORY ... GIT_TAG ...)` |
| **预编译库（find_package）** | 稳定依赖、ABI 兼容、减少编译时间。 | `find_package(TenEngine001Core REQUIRED)` |

**Agent 需澄清**：本模块对上游模块（如 001-Core、002-Object）采用**源码**还是**预编译库**方式？

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

# === 依赖（根据 §3.2 澄清结果选择方式） ===
# 方式 A：源码
# add_subdirectory(deps/001-core)

# 方式 B：FetchContent
# include(FetchContent)
# FetchContent_Declare(TenEngine001Core GIT_REPOSITORY ... GIT_TAG ...)
# FetchContent_MakeAvailable(TenEngine001Core)

# 方式 C：预编译库
# find_package(TenEngine001Core REQUIRED)

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

- [ ] 已明确 **worktree 路径**与 **构建根目录**。
- [ ] 已澄清对上游模块的 **依赖方式**（源码 / FetchContent / 预编译库）。
- [ ] `CMakeLists.txt` 中的 `target_include_directories` 与契约中的公开头文件路径一致。
- [ ] `target_link_libraries` 中的依赖与 `000-module-dependency-map.md` 一致。
- [ ] 已配置 **out-of-source build**（如 `build/`）。

---

## 8. 小结

| 项目 | 说明 |
|------|------|
| **构建根目录** | worktree 根目录（如 `TenEngine-NNN-modulename/`）。 |
| **依赖方式** | 源码（add_subdirectory / FetchContent）或预编译库（find_package）；需在 implement 前澄清。 |
| **目录结构** | `include/`（公开头）、`src/`（实现）、`tests/`、`apps/`（可选）。 |
| **契约对应** | 公开头文件与目标名须与 `specs/_contracts/` 一致。 |
