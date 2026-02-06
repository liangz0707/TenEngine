# 契约：007-Subsystems 模块对外 API

## 适用模块

- **实现方**：007-Subsystems（L1；可插拔子系统：描述符、注册、生命周期管理；Display、XR 等）
- **对应规格**：`docs/module-specs/007-subsystems.md`
- **依赖**：001-Core、002-Object

## 消费者

- 027-XR
- 其他需要子系统功能的模块

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SubsystemDescriptor | 子系统元数据、依赖列表、优先级、平台过滤 | 注册后直至卸载 |
| ISubsystem | 子系统实例接口，提供生命周期钩子 | 与 Start/Stop 一致 |
| Registry | 子系统注册表单例，管理子系统注册和查询 | 应用生命周期 |
| Lifecycle | 子系统生命周期管理器，负责批量生命周期操作 | 静态类，无实例 |
| Discovery | 插件发现和注册管理器 | 静态类，无实例 |
| SubsystemState | 子系统状态枚举（Uninitialized/Initialized/Started/Stopped/Shutdown） | 枚举类型 |
| RegisterResult | 注册操作结果，包含成功状态和错误信息 | 临时值 |
| LifecycleResult | 生命周期操作结果，包含成功状态、失败子系统列表和错误信息 | 临时值 |
| DiscoveryResult | 插件发现操作结果，包含发现的子系统和失败的插件列表 | 临时值 |
| GetSubsystem\<T\> | 按类型查询子系统实例 | 与子系统生命周期一致 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 描述符 | SubsystemDescriptor 包含 typeInfo、name、version、dependencies、dependencyCount、priority、platformFilter、configData；支持依赖声明、优先级排序、平台过滤；可序列化、反射（Object） |
| 2 | 注册表 | Registry 单例提供 Register、GetSubsystem\<T\>、GetSubsystemByName、GetAllSubsystems、Unregister；支持按类型和名称查询；线程安全；支持状态查询（GetSubsystemState、GetSubsystemStateByName） |
| 3 | 生命周期管理 | Lifecycle 类提供 InitializeAll、StartAll、StopAll、ShutdownAll；支持依赖拓扑排序；支持单个子系统生命周期操作（InitializeSubsystem、StartSubsystem、StopSubsystem、ShutdownSubsystem）；支持依赖检查（CheckDependencies）；返回详细的操作结果（LifecycleResult） |
| 4 | 插件发现 | Discovery 类提供 ScanPlugins、RegisterFromPlugin、UnregisterFromPlugin；支持从多个路径扫描插件；与 Core.ModuleLoad 配合；返回详细的发现结果（DiscoveryResult） |
| 5 | 状态管理 | 支持查询子系统状态（SubsystemState）；支持状态跟踪（Uninitialized → Initialized → Started → Stopped → Shutdown） |
| 6 | 错误处理 | RegisterResult、LifecycleResult、DiscoveryResult 提供详细的错误信息和失败列表；支持部分失败场景 |

## API 详细说明

### Registry（注册表）

**单例模式**：通过 `Registry::GetInstance()` 获取实例。

**核心功能**：
- **注册**：`Register(SubsystemDescriptor, unique_ptr<ISubsystem>)` - 注册子系统，所有权转移给注册表
- **查询**：`GetSubsystem<T>()` - 按类型查询；`GetSubsystemByName(name)` - 按名称查询；`GetAllSubsystems()` - 获取所有子系统
- **状态**：`GetSubsystemState(typeInfo)` / `GetSubsystemStateByName(name)` - 查询子系统状态
- **注销**：`Unregister(typeInfo)` - 注销子系统
- **同步**：`Lock()` / `Unlock()` - 外部同步支持
- **状态检查**：`IsShutdown()` - 检查注册表是否已关闭

**线程安全**：所有操作都受互斥锁保护。

### Lifecycle（生命周期管理）

**静态类**：所有方法都是静态方法，需要传入 Registry 引用。

**批量操作**：
- `InitializeAll(Registry&)` - 按依赖顺序初始化所有子系统
- `StartAll(Registry&)` - 按优先级顺序启动所有子系统
- `StopAll(Registry&)` - 按反向优先级顺序停止所有子系统
- `ShutdownAll(Registry&)` - 按反向依赖顺序关闭所有子系统

**单个操作**：
- `InitializeSubsystem(Registry&, name)` - 初始化单个子系统
- `StartSubsystem(Registry&, name)` - 启动单个子系统
- `StopSubsystem(Registry&, name)` - 停止单个子系统
- `ShutdownSubsystem(Registry&, name)` - 关闭单个子系统

**辅助功能**：
- `CheckDependencies(Registry&, name)` - 检查依赖是否满足

**错误处理**：所有批量操作和单个操作（除 ShutdownAll/ShutdownSubsystem）都返回 `LifecycleResult`，包含成功状态、失败子系统列表和错误信息。

### Discovery（插件发现）

**静态类**：所有方法都是静态方法，需要传入 Registry 引用。

**核心功能**：
- `ScanPlugins(Registry&, pluginPaths, pathCount)` - 从多个路径扫描插件并注册子系统
- `RegisterFromPlugin(Registry&, moduleHandle)` - 从插件模块注册子系统
- `UnregisterFromPlugin(Registry&, moduleHandle)` - 从插件模块注销子系统

**错误处理**：`ScanPlugins` 返回 `DiscoveryResult`，包含发现的子系统列表和失败的插件列表。

### ISubsystem（子系统接口）

**生命周期钩子**：
- `Initialize()` - 初始化子系统，返回成功/失败
- `Start()` - 启动子系统
- `Stop()` - 停止子系统
- `Shutdown()` - 关闭子系统

**元数据查询**：
- `GetDescriptor()` - 获取子系统描述符
- `GetName()` - 获取子系统名称

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- **当前版本**：v1.0（2026-02-06 更新）

## 约束

- 须在 Core、Object 可用之后使用
- 子系统启动/停止顺序按依赖图保证
- 生命周期操作应在主循环单线程环境中调用
- Registry 操作是线程安全的，但 Lifecycle 操作建议在单线程环境中使用
- XR、Display 等作为实现挂接于本模块

## 使用示例

```cpp
// 注册子系统
te::subsystems::SubsystemDescriptor desc{};
desc.typeInfo = &typeid(MySubsystem);
desc.name = "MySubsystem";
desc.dependencies = nullptr;
desc.dependencyCount = 0;
desc.priority = 0;
desc.platformFilter = 0;

auto subsystem = std::make_unique<MySubsystem>();
te::subsystems::Registry::Register(desc, std::move(subsystem));

// 批量生命周期操作
auto& reg = te::subsystems::Registry::GetInstance();
auto initResult = te::subsystems::Lifecycle::InitializeAll(reg);
if (initResult.success) {
    auto startResult = te::subsystems::Lifecycle::StartAll(reg);
    // ...
}

// 查询子系统
MySubsystem* subsystem = te::subsystems::Registry::GetSubsystem<MySubsystem>();
if (subsystem) {
    // 使用子系统
}

// 停止和关闭
te::subsystems::Lifecycle::StopAll(reg);
te::subsystems::Lifecycle::ShutdownAll(reg);
```

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 007-Subsystems 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
| 2026-02-06 | 更新以匹配实际代码实现：Registry（原 SubsystemRegistry）、独立的 Lifecycle 和 Discovery 类、新增状态管理和错误处理功能、详细 API 说明 |
