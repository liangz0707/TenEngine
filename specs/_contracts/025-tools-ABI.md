# 025-Tools 模块 ABI

- **契约**：[025-tools-public-api.md](./025-tools-public-api.md)（能力与类型描述）
- **本文件**：025-Tools 对外 ABI 显式表。
- **参考**：Unity Build/Asset Pipeline、UE Build/Editor 工具；构建、批处理、CLI、插件/包管理。本模块**不向引擎内运行时模块**提供跨边界类型；向用户/CI/脚本暴露抽象。
- **命名**：成员方法采用 **PascalCase**；说明列给出**完整函数签名**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 构建（Build）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 025-Tools | te::tools | IBuildSystem | 抽象接口 | 配置 | te/tools/Build.h | IBuildSystem::Configure | `bool Configure(BuildConfig const& config);` 项目编译、依赖、多目标；与 CMake/MSBuild 对接 |
| 025-Tools | te::tools | IBuildSystem | 抽象接口 | 编译 | te/tools/Build.h | IBuildSystem::Compile | `bool Compile();` 单次构建 |
| 025-Tools | te::tools | — | struct | 构建配置 | te/tools/Build.h | BuildConfig | 目标平台、依赖、选项；单次构建 |
| 025-Tools | te::tools | — | 自由函数/工厂 | 获取构建系统 | te/tools/Build.h | GetBuildSystem | `IBuildSystem* GetBuildSystem();` 可选；由 Tools 或主程序提供 |

### 批处理（Batch）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 025-Tools | te::tools | IBatchProcessor | 抽象接口 | 批量导入 | te/tools/Batch.h | IBatchProcessor::BatchImport | `bool BatchImport(char const* inputDir, char const* outputDir, BatchJob const& job);` 与 Resource 对接；单次批处理 |
| 025-Tools | te::tools | IBatchProcessor | 抽象接口 | 批量处理 | te/tools/Batch.h | IBatchProcessor::BatchProcess | `bool BatchProcess(BatchJob const& job);` 单次批处理 |
| 025-Tools | te::tools | — | struct | 批处理任务 | te/tools/Batch.h | BatchJob | 输入/输出、选项；单次批处理 |

### CLI

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 025-Tools | te::tools | — | 自由函数 | 解析参数 | te/tools/CLI.h | ParseArgs | `CLIResult ParseArgs(int argc, char const** argv);` 命令行、子命令 |
| 025-Tools | te::tools | — | 自由函数 | 执行命令 | te/tools/CLI.h | RunCommand | `int RunCommand(char const* name, CLIResult const& args);` 离线 API；单次调用 |
| 025-Tools | te::tools | — | struct | CLI 结果 | te/tools/CLI.h | CLIResult | 子命令、选项；单次调用 |
| 025-Tools | te::tools | — | 自由函数 | 离线 API | te/tools/CLI.h | InvokeOfflineAPI | `int InvokeOfflineAPI(char const* apiName, void const* input, void* output);` 可选；单次调用 |

### 包管理（PackageManager）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 025-Tools | te::tools | IPluginManager | 抽象接口 | 发现插件 | te/tools/PluginManager.h | IPluginManager::DiscoverPlugins | `void DiscoverPlugins(char const* directory);` 与 ModuleLoad 对接 |
| 025-Tools | te::tools | IPluginManager | 抽象接口 | 解析依赖 | te/tools/PluginManager.h | IPluginManager::ResolveDeps | `bool ResolveDeps(PluginDescriptor const* plugin);` |
| 025-Tools | te::tools | IPluginManager | 抽象接口 | 加载插件 | te/tools/PluginManager.h | IPluginManager::LoadPlugin | `void* LoadPlugin(char const* path);` 与 Core.ModuleLoad 对接 |
| 025-Tools | te::tools | — | struct | 插件描述符 | te/tools/PluginManager.h | PluginDescriptor | 版本、依赖、由 Tools 或包管理管理 |

*来源：契约能力 Build、Batch、CLI、PackageManager；参考 Unity Build、UE 构建/工具。*
