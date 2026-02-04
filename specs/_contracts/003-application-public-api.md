# 契约：003-Application 模块对外 API

## 适用模块

- **实现方**：**003-Application**（T0 应用生命周期与主循环）
- **对应规格**：`docs/module-specs/003-application.md`
- **依赖**：001-Core（001-core-public-api）

## 消费者（T0 下游）

- 006-Input（窗口与事件源）, 017-UICore（窗口与消息循环）, 024-Editor（编辑器窗口与主循环）。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| WindowHandle | 窗口句柄、与平台原生句柄对接 | 创建后直至销毁 |
| DisplayInfo | 显示器信息、DPI、多显示器 | 查询时有效 |
| Event | 来自平台消息泵的事件；与 Input 解耦的事件源 | 单次泵取或队列内 |
| DeltaTime / TickCallback | 帧间隔、主循环 Tick 挂接点 | 每帧/按注册 |

## 能力列表（提供方保证）

1. **应用生命周期**：Initialize、Run、Pause、Resume、RequestExit、GetExitCode。
2. **窗口**：CreateWindow、DestroyWindow、SetSize、SetFullscreen、GetNativeHandle、GetDisplayInfo。
3. **消息循环**：EventLoop、PumpEvents、与 Input 桥接的事件源。
4. **主循环**：Tick、GetDeltaTime、RegisterTickCallback；与引擎各子系统 Tick 挂接。

## 调用顺序与约束

- 须在 Core 初始化之后使用；窗口创建与事件泵依赖平台抽象（Core）。
- 主循环与 Input、UICore、Editor 的对接顺序由实现约定并文档化。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 003-Application 模块规格与依赖表新增契约 |
| 2026-01-29 | 契约更新由 plan 003-application-fullversion-001 同步 |
