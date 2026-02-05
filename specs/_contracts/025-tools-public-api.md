# 契约：025-Tools 模块对外 API

## 适用模块

- **实现方**：025-Tools（L4；构建、批处理、CLI、插件/包管理；按需依赖）
- **对应规格**：`docs/module-specs/025-tools.md`
- **依赖**：按需（通常 Core、Object、Resource 等）

## 消费者

- 无（L4 消费端；向用户/CI 提供构建产物、批处理、CLI、插件列表）

## 能力列表

### 类型与句柄（跨边界）

本模块不向引擎内运行时模块提供跨边界类型；以下为向用户/CI/脚本暴露的抽象。

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| BuildConfig | 项目编译、依赖、多目标、与 CMake/MSBuild 对接 | 单次构建 |
| BatchJob | 批量导入、批量处理、与 Resource 对接 | 单次批处理 |
| CLICommand | 命令行、子命令、离线 API | 单次调用 |
| PluginDescriptor | 插件/包发现、版本、依赖、与 ModuleLoad 对接 | 由 Tools 或包管理管理 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 构建 | BuildConfig、多目标、与 CMake/MSBuild 对接 |
| 2 | 批处理 | BatchJob、批量导入、与 Resource 对接 |
| 3 | CLI | CLICommand、子命令、离线 API |
| 4 | 插件/包管理 | PluginDescriptor、发现、版本、依赖、与 ModuleLoad 对接 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 按需依赖；运行时如需查询构建信息或插件列表可在本契约中补充只读接口。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 025-Tools 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
