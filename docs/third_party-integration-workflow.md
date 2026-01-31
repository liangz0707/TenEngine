# 第三方库集成工作流（TenEngine）

本文档定义：**模块如何声明第三方依赖**、**Plan 指令如何自动纳入**、**Task 过程中必须执行的步骤**、**引入方式与 CMake 的对应关系**，以及**从 `docs/third_party/*.md` 自动识别引入方式**的规则。参考实现见 `G:\AIHUMAN\WorkSpace\FirstEngine`（third_party 子模块 + 各模块 CMake 引用）。

---

## 一、引入方式分类（必须明确）

每个第三方库在 `docs/third_party/<id>-<name>.md` 中**必须**标明 **引入方式**，且仅能属于以下四类之一：

| 引入方式 | 含义 | CMake 典型写法 | 自动下载方式 |
|----------|------|----------------|---------------|
| **header-only** | 仅头文件，无独立编译产物；单/多头文件，可能需在某一 .cpp 中 `#define XXX_IMPLEMENTATION` 后 include | `add_library(xxx INTERFACE)` + `target_include_directories(xxx INTERFACE <path>)`；或直接 `target_include_directories(本模块 PRIVATE <path>)` | FetchContent / git submodule / 脚本拉取到 `third_party/<id>/` |
| **source** | 源码纳入工程，随主工程一起编译（子目录或 FetchContent 拉取） | `add_subdirectory(third_party/xxx)` 或 `FetchContent_Declare` + `FetchContent_MakeAvailable(xxx)` | FetchContent（推荐）或 git submodule / 脚本，**必须**在配置前完成拉取 |
| **sdk** | 预编译库或官方 SDK（DLL/静态库 + 头文件），不随工程编译 | `find_package(xxx)` 或 `find_path`/`find_library` + `target_link_libraries`；或 `ExternalProject_Add` 后使用产物目录 | 脚本/文档指定下载 URL 或安装路径；**必须**在配置前存在或通过脚本安装 |
| **system** | 系统/平台提供的库（如 Vulkan SDK、D3D12、Metal 框架） | `find_package(Vulkan)`、`find_package(glfw3)` 等；或平台宏 + 系统路径 | 不自动下载；文档中说明安装方式（如安装 Vulkan SDK、系统包管理器） |

**自动识别规则**：  
- 若文档中写明「引入方式: header-only」或「单头文件」「header-only」→ **header-only**。  
- 若文档中写明「引入方式: source」或「add_subdirectory」「FetchContent」「源码引入」→ **source**。  
- 若文档中写明「引入方式: sdk」或「find_package」「预编译」「SDK」→ **sdk**。  
- 若文档中写明「引入方式: system」或「系统库」「平台 SDK」→ **system**。  
- 未写明时，由「CMake 集成」小节推断：仅有 `target_include_directories` 且无 `add_library(xxx STATIC/SHARED)` → header-only；存在 `add_subdirectory`/`FetchContent_MakeAvailable` → source；存在 `find_package` 且说明为外部安装 → sdk 或 system。

---

## 二、模块中如何体现第三方依赖

1. **在模块 spec 或 ABI/契约中声明**  
   在 `specs/NNN-module-name/spec.md` 或 `specs/_contracts/NNN-module-name-public-api.md` 的「依赖」或「技术栈」中列出本模块所需的第三方库 **ID**（与 `docs/third_party/` 表一致），例如：`第三方: gtest, spdlog, glm, stb`。

2. **在 plan 中必须列出**  
   若本 feature 涉及某模块，且该模块声明了第三方依赖，则 **plan.md 必须包含「第三方依赖」小节**（见 §三），不得遗漏；Plan 指令（speckit.plan）会据此自动加入该小节。

3. **在 CMake 中最终体现**  
   通过「配置实现」步骤（见 §四）在模块或根的 CMake 中完成：`target_link_libraries(本模块 PRIVATE <第三方_target>)` 或 `target_include_directories` + 宏定义；引入方式决定具体写法（见 §一表格）。

---

## 三、Plan 指令中自动加入第三方依赖

- **触发条件**：本 feature 对应的模块（或 spec/contracts）中声明了第三方库 ID。  
- **必须产出**：在 plan.md 的「依赖引入方式」之后（或其中）增加 **「第三方依赖」** 小节，格式如下：

```markdown
## 第三方依赖（本 feature 涉及模块所需）

| 第三方 ID | 引入方式 | 文档 | 说明 |
|------------|----------|------|------|
| stb        | header-only | [stb.md](docs/third_party/stb.md) | 图像加载，单头文件 |
| glm        | header-only | [glm.md](docs/third_party/glm.md) | 数学库 |
| assimp     | source      | [assimp.md](docs/third_party/assimp.md) | 模型导入，源码构建 |
```

- **填写规则**：  
  - 从 `docs/third_party/README.md` 或各 `<id>-<name>.md` 的「引入方式」读取；若无则按 §一自动识别规则推断。  
  - 每个 ID 对应一份 `docs/third_party/<id>-<name>.md`，plan 中必须可引用。  
- **无第三方时**：若本 feature 不涉及任何第三方，可写「本 feature 无第三方依赖」或省略该小节。

---

## 四、Task 过程中必须的步骤（7 步）

对 plan 中列出的 **每一个** 第三方依赖，tasks.md 中**必须**包含以下 7 类工作（可合并为少量任务项，但缺一不可）：

| 步骤 | 必须 | 内容 |
|------|------|------|
| 1. **版本适配/选择** | 是 | 根据 `docs/third_party/<id>-<name>.md` 中的推荐版本、标签与兼容性，确定使用的版本/commit/tag；若有多个可选版本，在任务中注明选择理由或采用文档推荐。 |
| 2. **自动下载** | **必须** | 通过 FetchContent、git submodule、或项目约定脚本拉取到工程内（如 `third_party/<id>/`）；**禁止**假设该库已存在，必须在任务中显式执行下载/拉取。 |
| 3. **配置** | 必须 | CMake 选项、平台开关（如关闭测试/文档、静态库等）；与 `docs/third_party` 中「可选配置」一致。 |
| 4. **安装** | 必须 | 确定安装路径或与主工程的衔接方式（header-only 可为「无安装步骤，仅需 include 路径」；source 多为随主工程构建无单独 install；sdk 为解压/安装到指定目录）。 |
| 5. **编译测试** | 必须 | 构建该第三方库（若为 source）；若其自带测试且可安全运行，则执行其测试以验证集成；header-only 可为「无需编译，验证 include 通过即可」。**本模块 tests/** 中须包含对第三方库的实际调用（如 spdlog::info、glm::vec3、stb_image_load 等），以验证第三方调用能力；见 `docs/agent-build-guide.md` 与 speckit.tasks「测试逻辑」。 |
| 6. **部署进工程** | 必须 | 将产物或头文件部署到主工程可用的位置（如 `third_party/<id>` 已存在即视为已部署；或 copy 到 build 的 include/lib 目录）；确保后续 CMake 能通过统一路径找到。 |
| 7. **配置实现** | 必须 | 在主工程 CMake 与（必要时）代码中完成：`target_link_libraries(本模块 PRIVATE <第三方_target>)`、`target_include_directories`、以及所需的宏定义（如 `STB_IMAGE_IMPLEMENTATION`）；与「引入方式」对应的 CMake 写法一致。 |

- **引入方式与 CMake 的对应**：  
  - **header-only**：下载 → 配置（可选）→ 无安装/编译 → 部署（include 路径）→ 配置实现（INTERFACE 或 include_directories + 单 TU 的 IMPLEMENTATION include）。  
  - **source**：下载 → 配置（Cache 变量）→ add_subdirectory/FetchContent_MakeAvailable 即「安装/部署」→ 编译（随主工程）→ 配置实现（target_link_libraries）。  
  - **sdk**：下载/安装到指定目录 → 配置（路径变量）→ 无需编译第三方 → 部署（路径已定）→ 配置实现（find_package 或 target_include_directories + target_link_libraries）。  
  - **system**：无下载；文档说明安装方式 → 配置（find_package）→ 配置实现（target_link_libraries）。  

---

## 五、CMake 配置小结（按引入方式）

- **header-only**  
  - 拉取后：`add_library(<id> INTERFACE)`，`target_include_directories(<id> INTERFACE ${<id>_SOURCE_DIR})`；本模块 `target_link_libraries(本模块 PRIVATE <id>)`。  
  - 若需在单 TU 中展开实现：在某一 .cpp 中 `#define STB_XXX_IMPLEMENTATION` 后 `#include "stb_xxx.h"`。  
- **source**  
  - `FetchContent_Declare` + `FetchContent_MakeAvailable(<id>)`，或 `add_subdirectory(third_party/<id>)`；本模块 `target_link_libraries(本模块 PRIVATE <id>::<id> 或 <id>)`。  
- **sdk**  
  - `find_package(<id>)` 或 `find_path`/`find_library`；`target_link_libraries(本模块 PRIVATE <id>::<id>)`。  
- **system**  
  - `find_package(Vulkan)` 等；`target_link_libraries(本模块 PRIVATE Vulkan::Vulkan)`。  

---

## 六、与 FirstEngine 的对应关系

FirstEngine 中第三方位于 `third_party/`，通过 **git submodule** 拉取；各模块在各自 `CMakeLists.txt` 中通过 `add_subdirectory` 或 include 路径引用：

- **header-only**：glm、stb（stb 仅 include 路径，在单 TU 中 define IMPLEMENTATION 后 include）。  
- **source**：assimp、glfw、glslang、spirv-cross、pybind11（均为 add_subdirectory）。  
- **sdk/system**：Vulkan（external/vulkan 或 find_package(Vulkan)）、Python（external/python 或系统路径）。  

TenEngine 可采用 **FetchContent 优先**（配置时自动下载），或沿用 submodule + add_subdirectory；无论哪种，**自动下载（步骤 2）为必须**，且需在文档与任务中写明具体方式。

---

## 七、引用与维护

- 第三方库一览与单库字段定义：`docs/third_party/README.md`。  
- 单库文档模板（含「引入方式」「CMake 集成」）：见 README §四。  
- Plan 模板：`.specify/templates/plan-template.md`（含「第三方依赖」表）。  
- Plan 指令：`.cursor/commands/speckit.plan.md`。  
- Task 指令与 7 步要求：`.cursor/commands/speckit.tasks.md`。
