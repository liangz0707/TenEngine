# TenEngine 子模块构建规约（Agent 规约）

**用途**：本文档为 **Agent 规约**，仅规定必须遵守的构建规则与约束；不包含问答式解释。Agent 在生成或修改 CMakeLists.txt、plan.md、测试/可执行工程时**必须**按本规约执行。

**引用**：依赖图与契约见 `specs/_contracts/000-module-dependency-map.md`。002 示例与提示词见 `docs/build-002-agent-guide.md`。

---

## 1. 规约适用范围与强制性

- **适用范围**：所有子模块（TenEngine-001-core、TenEngine-002-object、…）的 **CMake 配置、编译、测试**。
- **强制性**：任何**编译、测试**的 build **必须**使用本规约规定的方式构建依赖与测试/可执行目标；**禁止**使用与本规约冲突的构建方式（见 §9）。

### 1.1 用户说「要构建工程」时的澄清义务（必须）

当用户提出**要构建工程**（或等价表述，如「帮我构建」「配置/编译这个工程」）时，若以下信息**不明确**，Agent **必须**向用户询问，不得擅自假设：

1. **需要使用哪种构建方式**：每个直接依赖采用 **源码** / **DLL** / **不引入外部库**；若用户未指定，**必须**询问（或逐依赖列出请用户确认）。未得到确认时，按规约**默认使用源码引入**。
2. **根目录在哪里**：被构建的**模块根目录**（如 `TenEngine-002-object` 的完整路径），以及（若使用源码且依赖为相对路径时）**TENENGINE_ROOT** 或各依赖的 **TENENGINE_xxx_DIR** 所在位置。若当前工作目录、打开的仓库或对话上下文无法唯一确定「在哪个目录下执行 cmake -B build」，**必须**询问用户根目录（或工程路径）。

只有在上述两项均明确（用户已指定或可从上下文唯一推断）后，Agent 方可执行配置与构建。

---

## 2. plan.md 澄清要求（必须）

- 在 **plan.md** 中，**必须**对每个**直接依赖**明确写出引入方式之一：
  - **源码**：通过本规约的依赖解析（add_subdirectory / TenEngineHelpers）引入上游源码构建。
  - **DLL**：通过本规约的预构建方式（TENENGINE_xxx_USE_SOURCE=OFF、PREFIX）引入上游已安装的库。
  - **不引入外部库**：不链接该依赖，仅用本模块内实现或桩；**可能编译报错**，需在 plan 中说明原因与后续处理。
- **默认**：未在 plan.md 中写明时，**默认使用源码引入**。
- 建议在 plan.md 中设一小节（如「依赖引入方式」），列出每个依赖及其选择（源码 / DLL / 不引入）。

---

## 3. 构建方式强制约定

- 所有**编译、测试**的 build **必须**按以下方式构建自己的依赖与测试/可执行目标：
  1. 使用**每个分支自带的** `cmake/TenEngineHelpers.cmake`、`cmake/TenEngineModuleDependencies.cmake`；从**同级上游**的 `cmake/` 引用（无上游则用本目录 `cmake/`）。
  2. 通过 **TENENGINE_CMAKE_DIR** 指定脚本目录，**include** TenEngineHelpers.cmake。
  3. 通过 **tenengine_resolve_my_dependencies("<本模块 id>" OUT_DEPS var)** 解析直接依赖，**target_link_libraries(本模块 PRIVATE ${var})**。
  4. 测试可执行文件**必须**使用 **tenengine_add_module_test(NAME ... MODULE_TARGET <本模块> SOURCES ...)**，不显式 link 依赖。
  5. 应用程序可执行文件**必须**使用 **tenengine_add_module_app(NAME ... MODULE_TARGET <本模块> SOURCES ...)**，不显式 link 依赖。
- 构建产出目录 **build** 位于**被构建的模块根目录下**（如 TenEngine-002-object/build/）。

---

## 4. 目录与路径

| 项目 | 规约 |
|------|------|
| 子模块目录 | TenEngine-001-core、TenEngine-002-object、…，同级并列；不依赖主仓库。 |
| 脚本位置 | 每个分支 **cmake/** 下必须有 TenEngineHelpers.cmake、TenEngineModuleDependencies.cmake。 |
| TENENGINE_CMAKE_DIR | 无上游：`"${CMAKE_CURRENT_SOURCE_DIR}/cmake"`。有上游：同级上游的 cmake，如 `"${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake"`。 |

---

## 5. 依赖引入方式变量（源码 vs DLL）

| 变量 | 含义 | 默认 |
|------|------|------|
| TENENGINE_&lt;NNN&gt;_&lt;NAME&gt;_USE_SOURCE | ON=源码，OFF=预构建 DLL | ON |
| TENENGINE_&lt;NNN&gt;_&lt;NAME&gt;_DIR | USE_SOURCE=ON 时源码目录 | 默认 `../TenEngine-&lt;id&gt;` 或 TENENGINE_ROOT/... |
| TENENGINE_&lt;NNN&gt;_&lt;NAME&gt;_PREFIX | USE_SOURCE=OFF 时安装前缀 | 由调用方指定 |

变量名：模块 id `001-core` → 前缀 `TENENGINE_001_CORE_`（数字+名大写、横线改下划线）。通过 cmake 配置时 `-D` 传入，**不在** CMakeLists.txt 中手写 if(USE_SOURCE) 分支。

---

## 6. CMakeLists.txt 必须内容

- **必须**：project、cmake_minimum_required、set(CMAKE_CXX_STANDARD 17)。
- **必须**：set(TENENGINE_CMAKE_DIR ...)、include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)。
- **有上游时必须**：tenengine_resolve_my_dependencies("<本模块 id>" OUT_DEPS var)、add_library(...)、target_link_libraries(本模块 PRIVATE ${var})、target_include_directories(本模块 PUBLIC ...)。
- **无上游时**：不调用 tenengine_resolve_my_dependencies；add_library、target_include_directories 照常。
- **测试/可执行**：**必须**使用 tenengine_add_module_test / tenengine_add_module_app，只传 NAME、MODULE_TARGET、SOURCES；**禁止**对测试/可执行 target 再写 target_link_libraries 依赖上游。

---

## 7. 测试与可执行目标必须用法

- **测试**：`tenengine_add_module_test(NAME <名> MODULE_TARGET <本模块 target> SOURCES <源文件> [ENABLE_CTEST])`。
- **应用程序**：`tenengine_add_module_app(NAME <名> MODULE_TARGET <本模块 target> SOURCES <源文件>)`。
- 依赖仅通过本模块 target 传递；**禁止**在测试/可执行上再 link 上游 target。

---

## 8. 最小示例（001 / 002 / 003 链）

**001（无上游）**

```cmake
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
add_library(te_core STATIC ...)
target_include_directories(te_core PUBLIC include)
tenengine_add_module_test(NAME te_core_test MODULE_TARGET te_core SOURCES tests/... ENABLE_CTEST)
```

**002（上游 001）**

```cmake
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake" CACHE PATH "")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
tenengine_resolve_my_dependencies("002-object" OUT_DEPS MY_DEPS)
add_library(te_object STATIC ...)
target_include_directories(te_object PUBLIC include)
target_link_libraries(te_object PRIVATE ${MY_DEPS})
tenengine_add_module_test(NAME te_object_test MODULE_TARGET te_object SOURCES tests/... ENABLE_CTEST)
```

**003（上游 002，如 004-scene 或 003-app）**

```cmake
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-002-object/cmake" CACHE PATH "")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
tenengine_resolve_my_dependencies("004-scene" OUT_DEPS MY_DEPS)
add_library(te_scene STATIC ...)
target_include_directories(te_scene PUBLIC include)
target_link_libraries(te_scene PRIVATE ${MY_DEPS})
tenengine_add_module_test(NAME te_scene_test MODULE_TARGET te_scene SOURCES tests/... ENABLE_CTEST)
```

本模块 id 与依赖表一致（如 001-core、002-object、004-scene）；依赖表见 cmake/TenEngineModuleDependencies.cmake。

---

## 9. 禁止项（Agent 必须遵守）

- **禁止**在本模块 CMakeLists.txt 中手写对上游的 `add_subdirectory` / `find_package` 分支；**必须**通过 TenEngineHelpers 的 tenengine_resolve_my_dependencies 与 CMake 变量（USE_SOURCE/DIR/PREFIX）由脚本统一处理。
- **禁止**对测试或可执行 target 显式 `target_link_libraries(..., 上游 target)`；**必须**只 link 本模块 target。
- **禁止**使用与本规约不一致的构建脚本路径（例如从主仓库根目录引用 cmake，除非规约后续允许）。
- **禁止**在未在 plan.md 中澄清依赖引入方式时，擅自采用「不引入外部库」导致必然编译失败；若采用不引入，**必须**在 plan.md 中写明并默认其余依赖为源码引入。

---

## 10. 相关文档

- 依赖图与契约：`specs/_contracts/000-module-dependency-map.md`
- 002 Agent 配置与提示词：`docs/build-002-agent-guide.md`
- 工作流：`docs/workflow-two-modules-pilot.md` §6.5 / §6.6
