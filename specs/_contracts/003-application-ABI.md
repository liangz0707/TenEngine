# 003-Application 模块 ABI

- **契约**：[003-application-public-api.md](./003-application-public-api.md)（能力与类型描述）
- **本文件**：003-Application 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 003-Application | TenEngine::application | IApplication | 抽象接口 | 应用生命周期与主循环 | TenEngine/application/Application.h | IApplication::Run, IApplication::RequestQuit | `void Run(RunParams const& args);` 进入主循环直至退出。`void RequestQuit();` 请求下一帧退出 |
| 003-Application | TenEngine::application | — | 自由函数 | 创建应用实例 | TenEngine/application/Application.h | CreateApplication | `IApplication* CreateApplication();` 失败返回 nullptr；调用方负责释放或由引擎管理 |
| 003-Application | TenEngine::application | — | struct | 运行参数 | TenEngine/application/Application.h | RunParams | 窗口标题、宽高、每帧回调 TickCallback、runMode；下游填充 |
| 003-Application | TenEngine::application | — | 回调类型 | 每帧回调 | TenEngine/application/Application.h | TickCallback | `void (*TickCallback)(float delta_time);` 主循环每帧调用一次 |
| 003-Application | TenEngine::application | — | 枚举 | 运行模式 | TenEngine/application/Application.h | RunMode | `enum class RunMode { Editor, Game };` 编辑器启动 vs 游戏启动 |
| 003-Application | TenEngine::application | IApplication | 抽象接口 | 获取当前运行模式 | TenEngine/application/Application.h | IApplication::GetRunMode | `RunMode GetRunMode() const;` 供子系统或脚本查询当前是编辑器还是游戏 |

*来源：用户故事 US-lifecycle-001（应用启动并进入主循环）、US-lifecycle-002（编辑器/游戏模式启动）。*
