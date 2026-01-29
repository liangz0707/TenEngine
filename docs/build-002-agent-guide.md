# 002-Object 构建与自动测试配置：Agent 指南

以 **002-Object** 为例，说明：**需要哪些前置文件**、**Agent 要生成/修改哪些文件**、**复制给 Agent 的提示词**，使子模块在**自动测试时自动引入依赖库**并生成合适工程。

---

## 一、前置条件（配置前需已具备）

| 项目 | 说明 |
|------|------|
| **每个分支都有 cmake/** | **TenEngine-001-core** 与 **TenEngine-002-object** 各自目录下都有 `cmake/TenEngineHelpers.cmake`、`cmake/TenEngineModuleDependencies.cmake`；不依赖主仓库，只需同级上游。 |
| **当前目录** | 在 **TenEngine-002-object** worktree 根目录下操作（即 002 的单独目录）。 |
| **002 构建时引用 001 的 cmake** | 在 002 的 CMakeLists.txt 中设置 `TENENGINE_CMAKE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake"`，从同级上游 001 的 cmake 引用脚本。 |
| **依赖 001 源码** | 若用源码方式：同级目录存在 **TenEngine-001-core**（或通过 `TENENGINE_001_CORE_DIR` 能指到 001）。若用 DLL：已安装 001 到某前缀，配置时传 `TENENGINE_001_CORE_USE_SOURCE=OFF` 和 `TENENGINE_001_CORE_PREFIX`。 |

目录结构示例（与主仓库同级）：

```
WorkSpaceSDD/
├── TenEngine-001-core/           # 001（必有 cmake/，002 构建时从此引用）
│   └── cmake/
│       ├── TenEngineHelpers.cmake
│       └── TenEngineModuleDependencies.cmake
└── TenEngine-002-object/         # 当前 002 worktree（本分支也有 cmake/，构建时用 001 的 cmake）
    ├── CMakeLists.txt            # Agent 生成或修改
    ├── include/
    ├── src/
    └── tests/
        └── test_type_registry.cpp # Agent 可生成占位
```

---

## 二、Agent 要生成或修改的文件

| 文件 | 作用 |
|------|------|
| **CMakeLists.txt**（002 根目录） | 引入主仓库 cmake、解析 002 的直接依赖、声明 `te_object` 库并 link 依赖、用 **tenengine_add_module_test** 添加测试可执行文件（只 link `te_object`，依赖自动带入）。 |
| **tests/test_xxx.cpp**（可选） | 测试可执行文件的源文件；若不存在则 Agent 生成一个最小占位（如 `#include` 本模块头 + 一个简单 main 或测试断言）。 |

**不要**在 002 里手写 `add_subdirectory(../TenEngine-001-core)` 或 `find_package` 分支；**只**通过 `tenengine_resolve_my_dependencies("002-object" OUT_DEPS MY_DEPS)` 和 `target_link_libraries(te_object PRIVATE ${MY_DEPS})` 引入依赖。测试只 link `te_object`，由 CMake 传递依赖。

---

## 三、可直接复制给 Agent 的提示词

在 **TenEngine-002-object** 目录下打开 Cursor（或当前工作目录即为 002 的根目录），将下面整段复制到对话中发给 Agent：

```
当前在 TenEngine-002-object（002-Object 模块）根目录。请按以下要求配置 CMake，使「自动测试」时自动引入依赖库（001-Core），并生成可用的工程。

1) 阅读文档：docs/build-module-convention.md、docs/build-002-agent-guide.md（本文件）。构建脚本在**每个分支的 cmake/** 中；002 构建时从**同级上游 001** 的 cmake 引用，不依赖主仓库。路径为：${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake。

2) 在 002 根目录创建或修改 CMakeLists.txt，要求：
   - 使用 cmake_minimum_required(VERSION 3.16)、project(TenEngine-002-object LANGUAGES CXX)、set(CMAKE_CXX_STANDARD 17)。
   - 设置 TENENGINE_CMAKE_DIR 为 "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake" 并 include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)。
   - 调用 tenengine_resolve_my_dependencies("002-object" OUT_DEPS MY_DEPS)，得到本模块直接依赖的 target 列表。
   - 声明本模块库：add_library(te_object STATIC <本模块已有或你列出的 src 文件>)，target_include_directories(te_object PUBLIC include)，target_link_libraries(te_object PRIVATE ${MY_DEPS})。
   - 添加测试可执行文件（自动带入依赖）：使用 tenengine_add_module_test(NAME te_object_test MODULE_TARGET te_object SOURCES tests/test_type_registry.cpp ENABLE_CTEST)。不要对测试可执行文件再写 target_link_libraries 依赖 001；只 link te_object。
   - 若 tests/test_type_registry.cpp 不存在，则创建该文件，内容可为最小可编译占位（包含本模块一个头文件 + main 或简单断言）。

3) 确认：构建测试可执行文件 te_object_test 时，只会 target_link_libraries(te_object_test PRIVATE te_object)，依赖 001 通过 te_object 的 target_link_libraries(te_object PRIVATE ${MY_DEPS}) 自动传递，从而自动引入依赖库。

请直接生成或修改 CMakeLists.txt 和（若需要）tests/test_type_registry.cpp，并简要说明配置后如何在该目录下执行 cmake -B build 与 cmake --build build 以验证。
```

---

## 四、配置后如何验证

在 **TenEngine-002-object** 根目录执行：

```powershell
cmake -B build
cmake --build build
```

**构建产出目录**：上述命令会在 **TenEngine-002-object/build/** 下生成工程与编译结果（可执行文件、库等）。002 通过 add_subdirectory 拉入的 001 的编译结果也在该 build 树内（如 `build/TenEngine_001-core/` 等）。

若使用 001 源码且 001 与 002 同级，无需额外变量；CMake 会从 `../TenEngine-001-core` 拉取 001 并先编译，再编译 002 和测试。  
运行测试可执行文件：

```powershell
.\build\te_object_test.exe
```

或使用 CTest（若启用了 ENABLE_CTEST）：

```powershell
cd build
ctest
```

---

## 五、其他模块（004、005…）怎么配

把上面提示词里的 **002-object**、**TenEngine-002-object**、**te_object**、**te_object_test**、**test_type_registry.cpp** 等替换成对应模块的 id、工程名、target 名、测试名和测试源文件即可；约定与步骤相同，见 `docs/build-module-convention.md`。
