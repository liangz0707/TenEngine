# 契约：003-Application 模块对外 API

## 适用模块

- **实现方**：003-Application（L0；应用生命周期、窗口、消息循环、主循环、DPI）
- **对应规格**：`docs/module-specs/003-application.md`
- **依赖**：001-Core

## 消费者

- 006-Input、017-UICore、024-Editor

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| WindowHandle | 窗口句柄、与平台原生句柄对接 | 创建后直至销毁 |
| DisplayInfo | 显示器信息、DPI、多显示器 | 查询时有效 |
| Event | 来自平台消息泵的事件；与 Input 解耦的事件源 | 单次泵取或队列内 |
| DeltaTime / TickCallback | 帧间隔、主循环 Tick 挂接点 | 每帧/按注册 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 应用生命周期 | Initialize、Run、Pause、Resume、RequestExit、GetExitCode |
| 2 | 窗口 | CreateWindow、DestroyWindow、SetSize、SetFullscreen、GetNativeHandle、GetDisplayInfo |
| 3 | 消息循环 | EventLoop、PumpEvents、与 Input 桥接的事件源 |
| 4 | 主循环 | Tick、GetDeltaTime、RegisterTickCallback；与引擎各子系统 Tick 挂接 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core 初始化之后使用；窗口创建与事件泵依赖平台抽象（Core）。主循环与 Input、UICore、Editor 的对接顺序由实现约定并文档化。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 003-Application 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
