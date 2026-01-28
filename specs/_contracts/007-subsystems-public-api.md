# 契约：007-Subsystems 模块对外 API

## 适用模块

- **实现方**：**007-Subsystems**（T0 可插拔子系统）
- **对应规格**：`docs/module-specs/007-subsystems.md`
- **依赖**：001-Core（001-core-public-api）, 002-Object（002-object-public-api）

## 消费者（T0 下游）

- 027-XR（作为子系统挂接）, 024-Editor（子系统列表与开关）。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SubsystemDescriptor | 子系统元数据、依赖列表、优先级、平台过滤 | 注册后直至卸载 |
| ISubsystem / GetSubsystem<T> | 子系统实例、按类型/接口查询 | 与生命周期 Start/Stop 一致 |
| 生命周期钩子 | Initialize、Start、Stop、Shutdown | 按依赖顺序调用 |

## 能力列表（提供方保证）

1. **描述符**：SubsystemDescriptor、Dependencies、Priority、PlatformFilter；可序列化、反射（Object）。
2. **注册表**：Register、GetSubsystem<T>、Unregister；按类型查询、单例或按实例管理。
3. **生命周期**：InitializeAll、StartAll、StopAll、ShutdownAll；依赖拓扑排序、与主循环或按需调用。
4. **可选发现**：ScanPlugins、RegisterFromPlugin；与 Core.ModuleLoad 配合。

## 调用顺序与约束

- 须在 Core、Object 可用之后使用；子系统启动/停止顺序按依赖图保证。
- XR、Display 等子系统作为实现挂接于本模块；契约仅约定注册、查询与生命周期接口。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 007-Subsystems 模块规格与依赖表新增契约 |
