# 007-Subsystems 模块 ABI

- **契约**：[007-subsystems-public-api.md](./007-subsystems-public-api.md)（能力与类型描述）
- **本文件**：007-Subsystems 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 007-Subsystems | te::subsystems | — | enum class | 子系统状态 | te/subsystems/registry.hpp | SubsystemState | `enum class SubsystemState { Uninitialized, Initialized, Started, Stopped, Shutdown };` 子系统生命周期状态 |
| 007-Subsystems | te::subsystems | — | struct | 注册结果 | te/subsystems/registry.hpp | RegisterResult | `struct RegisterResult { bool success; char const* errorMessage; };` 注册操作结果 |
| 007-Subsystems | te::subsystems | — | struct | 子系统描述符 | te/subsystems/descriptor.hpp | SubsystemDescriptor | `struct SubsystemDescriptor { void const* typeInfo; char const* name; char const* version; char const* const* dependencies; std::size_t dependencyCount; int priority; std::uint32_t platformFilter; void const* configData; };` 元数据、依赖列表、优先级、平台过滤 |
| 007-Subsystems | te::subsystems | ISubsystem | 抽象接口 | 子系统生命周期接口 | te/subsystems/subsystem.hpp | ISubsystem::Initialize, ISubsystem::Start, ISubsystem::Stop, ISubsystem::Shutdown, ISubsystem::GetDescriptor, ISubsystem::GetName | `bool Initialize();` `void Start();` `void Stop();` `void Shutdown();` `SubsystemDescriptor const& GetDescriptor() const;` `char const* GetName() const;` 由 Lifecycle 按依赖顺序调用 |
| 007-Subsystems | te::subsystems | Registry | 单例 | 子系统注册表 | te/subsystems/registry.hpp | Registry::GetInstance | `static Registry& GetInstance();` 获取单例实例 |
| 007-Subsystems | te::subsystems | Registry | 实例方法 | 注册子系统 | te/subsystems/registry.hpp | Registry::RegisterInstance | `RegisterResult RegisterInstance(SubsystemDescriptor const& desc, std::unique_ptr<ISubsystem> instance);` 注册子系统，所有权转移给注册表 |
| 007-Subsystems | te::subsystems | Registry | 静态方法 | 注册子系统（便捷方法） | te/subsystems/registry.hpp | Registry::Register | `static bool Register(SubsystemDescriptor const& desc, std::unique_ptr<ISubsystem> instance);` 返回 false 表示失败（向后兼容） |
| 007-Subsystems | te::subsystems | Registry | 模板方法 | 按类型获取子系统 | te/subsystems/registry.hpp | Registry::GetSubsystem\<T\> | `template<typename T> static T* GetSubsystem();` 返回已注册的 T*；未注册或 ShutdownAll 后返回 nullptr |
| 007-Subsystems | te::subsystems | Registry | 实例方法 | 按名称获取子系统 | te/subsystems/registry.hpp | Registry::GetSubsystemByName | `ISubsystem* GetSubsystemByName(char const* name);` 按名称查询子系统；未找到或 ShutdownAll 后返回 nullptr |
| 007-Subsystems | te::subsystems | Registry | 实例方法 | 获取所有子系统 | te/subsystems/registry.hpp | Registry::GetAllSubsystems | `std::vector<ISubsystem*> GetAllSubsystems();` 返回所有已注册的子系统列表 |
| 007-Subsystems | te::subsystems | Registry | 实例方法 | 获取子系统状态 | te/subsystems/registry.hpp | Registry::GetSubsystemState | `SubsystemState GetSubsystemState(void const* typeInfo) const;` 按 typeInfo 查询子系统状态 |
| 007-Subsystems | te::subsystems | Registry | 实例方法 | 按名称获取子系统状态 | te/subsystems/registry.hpp | Registry::GetSubsystemStateByName | `SubsystemState GetSubsystemStateByName(char const* name) const;` 按名称查询子系统状态 |
| 007-Subsystems | te::subsystems | Registry | 静态方法 | 注销子系统 | te/subsystems/registry.hpp | Registry::Unregister | `static void Unregister(void const* typeInfo);` 按 typeInfo 注销子系统 |
| 007-Subsystems | te::subsystems | Registry | 实例方法 | 锁定注册表 | te/subsystems/registry.hpp | Registry::Lock | `void Lock();` 锁定注册表互斥锁（用于外部同步） |
| 007-Subsystems | te::subsystems | Registry | 实例方法 | 解锁注册表 | te/subsystems/registry.hpp | Registry::Unlock | `void Unlock();` 解锁注册表互斥锁 |
| 007-Subsystems | te::subsystems | Registry | 实例方法 | 检查关闭状态 | te/subsystems/registry.hpp | Registry::IsShutdown | `bool IsShutdown() const;` 检查注册表是否处于关闭状态 |
| 007-Subsystems | te::subsystems | — | struct | 生命周期结果 | te/subsystems/lifecycle.hpp | LifecycleResult | `struct LifecycleResult { bool success; std::vector<char const*> failedSubsystems; char const* errorMessage; };` 生命周期操作结果 |
| 007-Subsystems | te::subsystems | Lifecycle | 静态方法 | 初始化所有子系统 | te/subsystems/lifecycle.hpp | Lifecycle::InitializeAll | `static LifecycleResult InitializeAll(Registry& reg);` 按依赖顺序初始化所有子系统；返回结果包含失败列表 |
| 007-Subsystems | te::subsystems | Lifecycle | 静态方法 | 启动所有子系统 | te/subsystems/lifecycle.hpp | Lifecycle::StartAll | `static LifecycleResult StartAll(Registry& reg);` 按优先级顺序启动所有子系统；返回结果包含失败列表 |
| 007-Subsystems | te::subsystems | Lifecycle | 静态方法 | 停止所有子系统 | te/subsystems/lifecycle.hpp | Lifecycle::StopAll | `static LifecycleResult StopAll(Registry& reg);` 按反向优先级顺序停止所有子系统；返回结果包含失败列表 |
| 007-Subsystems | te::subsystems | Lifecycle | 静态方法 | 关闭所有子系统 | te/subsystems/lifecycle.hpp | Lifecycle::ShutdownAll | `static void ShutdownAll(Registry& reg);` 按反向依赖顺序关闭所有子系统 |
| 007-Subsystems | te::subsystems | Lifecycle | 静态方法 | 初始化单个子系统 | te/subsystems/lifecycle.hpp | Lifecycle::InitializeSubsystem | `static LifecycleResult InitializeSubsystem(Registry& reg, char const* name);` 按名称初始化单个子系统 |
| 007-Subsystems | te::subsystems | Lifecycle | 静态方法 | 启动单个子系统 | te/subsystems/lifecycle.hpp | Lifecycle::StartSubsystem | `static LifecycleResult StartSubsystem(Registry& reg, char const* name);` 按名称启动单个子系统 |
| 007-Subsystems | te::subsystems | Lifecycle | 静态方法 | 停止单个子系统 | te/subsystems/lifecycle.hpp | Lifecycle::StopSubsystem | `static LifecycleResult StopSubsystem(Registry& reg, char const* name);` 按名称停止单个子系统 |
| 007-Subsystems | te::subsystems | Lifecycle | 静态方法 | 关闭单个子系统 | te/subsystems/lifecycle.hpp | Lifecycle::ShutdownSubsystem | `static void ShutdownSubsystem(Registry& reg, char const* name);` 按名称关闭单个子系统 |
| 007-Subsystems | te::subsystems | Lifecycle | 静态方法 | 检查依赖 | te/subsystems/lifecycle.hpp | Lifecycle::CheckDependencies | `static bool CheckDependencies(Registry& reg, char const* subsystemName);` 检查子系统的所有依赖是否已注册并初始化 |
| 007-Subsystems | te::subsystems | — | struct | 发现结果 | te/subsystems/discovery.hpp | DiscoveryResult | `struct DiscoveryResult { bool success; std::vector<char const*> discoveredSubsystems; std::vector<char const*> failedPlugins; char const* errorMessage; };` 插件发现操作结果 |
| 007-Subsystems | te::subsystems | Discovery | 静态方法 | 扫描插件 | te/subsystems/discovery.hpp | Discovery::ScanPlugins | `static DiscoveryResult ScanPlugins(Registry& reg, char const* const* pluginPaths, size_t pathCount);` 从多个路径扫描插件并注册子系统；返回发现的子系统和失败的插件列表 |
| 007-Subsystems | te::subsystems | Discovery | 静态方法 | 从插件注册 | te/subsystems/discovery.hpp | Discovery::RegisterFromPlugin | `static bool RegisterFromPlugin(Registry& reg, void* moduleHandle);` 从插件模块注册子系统；moduleHandle 为 Core 模块句柄 |
| 007-Subsystems | te::subsystems | Discovery | 静态方法 | 从插件注销 | te/subsystems/discovery.hpp | Discovery::UnregisterFromPlugin | `static bool UnregisterFromPlugin(Registry& reg, void* moduleHandle);` 从插件模块注销子系统；应在卸载插件模块前调用 |

*来源：用户故事 US-001（应用启动并进入主循环）。契约能力：描述符、注册表、生命周期（Initialize/Start/Stop/Shutdown）、发现。*

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 007-Subsystems 初始 ABI |
| 2026-02-06 | 更新以匹配实际代码实现：Registry（原 SubsystemRegistry）、Initialize（原 Init）、独立的 Lifecycle 类、新增状态管理和错误处理功能 |
