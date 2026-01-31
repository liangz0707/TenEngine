# 第三方工具集成（Third-Party Integrations）

本目录描述 TenEngine 所依赖的**外部工程**及其集成方式。每个集成对应一个独立的 `.md` 文档；**需要该依赖的模块通过引用对应文档即可由构建系统自动完成集成**（如 CMake FetchContent / find_package 或统一脚本拉取）。

**集成工作流**（Plan 自动纳入、Task 7 步、引入方式与 CMake 对应）：见 [third_party-integration-workflow.md](../third_party-integration-workflow.md)。

---

## 一、引入方式分类（必须明确）

每个第三方库在对应 md 中**必须**标明 **引入方式**，且仅能属于以下四类之一；**自动识别**与 CMake 写法见 [third_party-integration-workflow.md](../third_party-integration-workflow.md) §一、§五。

| 引入方式 | 含义 | CMake 典型写法 |
|----------|------|----------------|
| **header-only** | 仅头文件，无独立编译产物；可能需在某一 .cpp 中 `#define XXX_IMPLEMENTATION` 后 include | `add_library(xxx INTERFACE)` + `target_include_directories(xxx INTERFACE <path>)` |
| **source** | 源码纳入工程，随主工程一起编译 | `add_subdirectory(third_party/xxx)` 或 `FetchContent_Declare` + `FetchContent_MakeAvailable(xxx)` |
| **sdk** | 预编译库或官方 SDK，不随工程编译 | `find_package(xxx)` 或 `find_path`/`find_library` + `target_link_libraries` |
| **system** | 系统/平台提供的库（如 Vulkan SDK、D3D12） | `find_package(Vulkan)` 等；文档说明安装方式 |

---

## 二、用途与约定

| 项目 | 说明 |
|------|------|
| **每个集成一个 md** | 每个第三方库/工具对应 `docs/third_party/<id>-<name>.md`，内容含：仓库/来源、版本建议、CMake 集成方式、引用方式、可选配置。 |
| **工程引用即自动集成** | 模块的 CMake 或依赖清单中**声明**所需第三方（如 `TENENGINE_USE_SPDLOG=ON` 或引用 `third_party/spdlog`）；构建时由 `cmake/` 或主仓统一脚本根据文档拉取/配置，无需手写 URL 与选项。 |
| **与 T0 契约一致** | 第三方仅用于实现层；对外 API 仍以 `specs/_contracts/` 契约为准，不直接暴露第三方类型到契约。 |

---

## 三、如何引用并自动集成

1. **在模块或根 CMake 中声明依赖**  
   使用统一变量或清单列出本模块需要的第三方 ID（见下表「ID」列），例如：  
   `set(TENENGINE_THIRD_PARTY "gtest;spdlog;glm;stb")` 或在 `cmake/ThirdPartyDependencies.cmake` 中按模块启用。

2. **构建时自动拉取与配置**  
   根目录或各 worktree 的 CMake 在配置阶段：  
   - 读取所需第三方 ID；  
   - 根据 `docs/third_party/<id>-<name>.md` 中的 **CMake 集成** 小节执行（如 `FetchContent_Declare` + `FetchContent_MakeAvailable`，或 `find_package`）；  
   - 暴露 `target_link_libraries(本模块 PRIVATE <第三方_target>)` 所需的 target 与 include。

3. **可选：集中清单**  
   可在主仓或 `cmake/` 下维护一份 `third_party_manifest.cmake`，按模块 ID 映射到第三方 ID 列表，实现「模块引用 → 自动集成」。

---

## 四、外部工程一览（初版）

下表列出当前规划需要依赖的**所有外部工程**；具体版本、许可证与 CMake 用法见对应 md。

| ID | 名称 | 用途 | 文档 | 使用模块/场景 |
|----|------|------|------|----------------|
| **gtest** | Google Test | 单元测试 | [gtest.md](./gtest.md) | 各模块 tests/ |
| **spdlog** | spdlog | 日志 | [spdlog.md](./spdlog.md) | 001-Core、通用 |
| **fmt** | fmt | 格式化（spdlog 可选依赖） | [fmt.md](./fmt.md) | 与 spdlog 配套 |
| **glm** | OpenGL Mathematics | 数学库（向量/矩阵/四元数） | [glm.md](./glm.md) | Core、RHI、RenderCore、Scene、Physics 等 |
| **stb** | stb 单头文件库 | 图像加载/写入、其他工具 | [stb.md](./stb.md) | Resource、Editor、工具 |
| **cgltf** | cgltf | glTF 2.0 模型解析（单头文件，轻量） | [cgltf.md](./cgltf.md) | 012-Mesh、013-Resource、glTF 资源 |
| **tinygltf** | tinygltf | glTF 2.0 加载（含 JSON/二进制、贴图引用） | [tinygltf.md](./tinygltf.md) | 012-Mesh、013-Resource（可选与 cgltf 二选一） |
| **libpng** | libpng | PNG 图像编解码 | [libpng.md](./libpng.md) | 013-Resource、贴图管线、截图/导出 |
| **libjpeg-turbo** | libjpeg-turbo | JPEG 图像编解码（高性能） | [libjpeg-turbo.md](./libjpeg-turbo.md) | 013-Resource、贴图管线 |
| **libwebp** | libwebp | WebP 图像编解码 | [libwebp.md](./libwebp.md) | 013-Resource、贴图管线、Web 资源 |
| **basis-universal** | Basis Universal | 纹理压缩（KTX2/Basis、GPU 友好） | [basis-universal.md](./basis-universal.md) | 013-Resource、008-RHI、贴图上传 |
| **volk** | Volk | Vulkan 加载器 | [volk.md](./volk.md) | 008-RHI（Vulkan 后端） |
| **glslang** | glslang | GLSL/HLSL 编译为 SPIR-V | [glslang.md](./glslang.md) | 010-Shader、离线编译 |
| **spirv-cross** | SPIRV-Cross | SPIR-V 转 MSL/HLSL 等 | [spirv-cross.md](./spirv-cross.md) | 010-Shader、跨后端 |
| **imgui** | Dear ImGui | 即时模式 GUI（编辑器/工具） | [imgui.md](./imgui.md) | 024-Editor、025-Tools、调试 UI |
| **miniaudio** | miniaudio | 跨平台音频采集与播放 | [miniaudio.md](./miniaudio.md) | 016-Audio（可选后端） |
| **openal** | OpenAL Soft | 3D 音频 API | [openal.md](./openal.md) | 016-Audio（可选后端） |
| **nlohmann-json** | nlohmann/json | JSON 解析与序列化 | [nlohmann-json.md](./nlohmann-json.md) | Resource、Tools、配置与元数据 |
| **zstd** | Zstandard | 压缩/解压 | [zstd.md](./zstd.md) | Resource、网络、资产管线 |
| **jolt** | Jolt Physics | 物理引擎（3D） | [jolt.md](./jolt.md) | 014-Physics（可选实现） |
| **bullet** | Bullet | 物理引擎（3D/2D） | [bullet.md](./bullet.md) | 014-Physics（可选实现） |
| **box2d** | Box2D | 2D 物理 | [box2d.md](./box2d.md) | 014-Physics、022-2D（2D 部分） |
| **assimp** | Assimp | 模型/场景导入（FBX、OBJ、glTF 等多格式） | [assimp.md](./assimp.md) | 012-Mesh、013-Resource、024-Editor（可选） |
| **fast_obj** | fast_obj | OBJ 模型快速解析（单文件、轻量） | [fast_obj.md](./fast_obj.md) | 012-Mesh、013-Resource（可选，OBJ 专用） |
| **vulkan-headers** | Vulkan Headers | Vulkan 头文件与注册表 | [vulkan-headers.md](./vulkan-headers.md) | 008-RHI、volk/glslang 依赖 |

**平台/系统级（不单独建 md，仅在模块「依赖的外部内容」中说明）**  
- **Vulkan SDK**：开发机安装；运行时可选。  
- **D3D12 / DXGI**：Windows SDK。  
- **Metal**：Apple 平台系统框架。  
- **WASAPI / ALSA / Core Audio**：各平台音频 API，由 016-Audio 抽象。

---

## 五、文档模板（每个集成 md 应含内容）

每个 `docs/third_party/<id>-<name>.md` **必须**包含以下字段，以便 Plan/Task 自动纳入与**自动识别引入方式**：

1. **引入方式**（**必须**）：`header-only` | `source` | `sdk` | `system` 四选一；用于 Task 生成 7 步与 CMake 配置。  
2. **名称与简介**：库名、一句话用途。  
3. **仓库/来源**：官方仓库 URL、推荐版本/标签（用于版本选择与自动下载）。  
4. **许可证**：简短说明（如 MIT、BSD、Apache-2.0）。  
5. **CMake 集成**：`FetchContent` 或 `find_package` / `add_subdirectory` 的推荐写法；如何暴露 target、include；对应「配置实现」步骤。  
6. **引用方式**：本工程如何声明依赖该第三方（变量名或清单字段），以实现「引用即自动集成」。  
7. **可选配置**：如编译选项、开关（如只头文件、静态库等）；对应「配置」步骤。  
8. **使用模块**：TenEngine 中哪些模块会用到（与上表一致即可）。

**Task 7 步与 CMake 对应**：版本选择 → 自动下载（必须）→ 配置 → 安装 → 编译测试 → 部署进工程 → 配置实现。详见 [third_party-integration-workflow.md](../third_party-integration-workflow.md) §四、§五。

---

## 六、与构建规约的衔接

- **依赖子模块**（T0 模块）仍按 `docs/engine-build-module-convention.md` 与 `cmake/TenEngineModuleDependencies.cmake` 解析；**第三方**与本目录描述一致，由统一脚本或根 CMake 在配置阶段拉取，不改变「单模块 worktree、依赖源码优先」等原则。  
- **测试**：build 测试时不引入子分支的测试工程（见 `docs/agent-build-guide.md`）；第三方测试工程（如 gtest）仅用于本模块 tests/，不向上游传递。
