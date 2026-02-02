# 契约：025-Tools 模块对外 API

## 适用模块

- **实现方**：**025-Tools**（构建、批处理、CLI 与插件/包管理）
- **对应规格**：`docs/module-specs/025-tools.md`
- **依赖**：按需（通常 Core、Object、Resource 等；见 `000-module-dependency-map.md`）

## 消费者（T0 下游）

- **无**（Tools 为 L4 消费端；向用户/CI 提供构建产物、批处理结果、CLI 输出、插件列表）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

本模块**不向引擎内运行时模块**提供跨边界类型；以下为向**用户/CI/脚本**暴露的抽象（可选文档化）：

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| BuildConfig | 项目编译、依赖、多目标、与 CMake/MSBuild 对接 | 单次构建 |
| BatchJob | 批量导入、批量处理、与 Resource 对接 | 单次批处理 |
| CLICommand | 命令行、子命令、离线 API | 单次调用 |
| PluginDescriptor | 插件/包发现、版本、依赖、与 ModuleLoad 对接 | 由 Tools 或包管理管理 |

若运行时需查询构建信息或插件列表，可在本契约中补充只读接口。**ABI 显式表**：[025-tools-ABI.md](./025-tools-ABI.md)。

## 能力列表（提供方保证）

1. **Build**：IBuildSystem::Configure、Compile；BuildConfig；GetBuildSystem；与构建系统对接。
2. **Batch**：IBatchProcessor::BatchImport、BatchProcess；BatchJob；与 Resource 对接。
3. **CLI**：ParseArgs、RunCommand、InvokeOfflineAPI；CLIResult；单次调用。
4. **PackageManager**：IPluginManager::DiscoverPlugins、ResolveDeps、LoadPlugin；PluginDescriptor；与 ModuleLoad 对接。

## 调用顺序与约束

- 依赖按实际实现可增删；与 Resource 导入管线、Core 模块加载协同。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：025-Tools 对应本契约；无下游；与 docs/module-specs/025-tools.md 一致 |
| 2026-01-28 | 根据 025-tools-ABI 反向更新：IBuildSystem、IBatchProcessor、ParseArgs、RunCommand、IPluginManager、BuildConfig、BatchJob、PluginDescriptor；能力与类型与 ABI 表一致 |
