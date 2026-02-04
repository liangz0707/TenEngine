# 007-Subsystems 模块 ABI

- **契约**：[007-subsystems-public-api.md](./007-subsystems-public-api.md)（能力与类型描述）
- **本文件**：007-Subsystems 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 007-Subsystems | te::subsystems | ISubsystem | 抽象接口 | 子系统生命周期 | te/subsystems/Subsystem.h | ISubsystem::Init, ISubsystem::Shutdown | `bool Init();` `void Shutdown();` 由 SubsystemRegistry 按序调用 |
| 007-Subsystems | te::subsystems | SubsystemRegistry | 单例 | 子系统注册与批量初始化 | te/subsystems/SubsystemRegistry.h | SubsystemRegistry::GetInstance, RegisterSubsystem, UnregisterSubsystem, InitAll, ShutdownAll, StartAll, StopAll | `static SubsystemRegistry& GetInstance();` `void RegisterSubsystem(ISubsystem* ptr);` `void UnregisterSubsystem(ISubsystem* ptr);` `bool InitAll();` `void ShutdownAll();` `void StartAll();` `void StopAll();` 按依赖序/逆序调用 |
| 007-Subsystems | te::subsystems | SubsystemRegistry | 模板方法 | 按类型获取子系统 | te/subsystems/SubsystemRegistry.h | SubsystemRegistry::GetSubsystem\<T\> | `template<typename T> T* GetSubsystem();` 返回已注册的 T*；未注册返回 nullptr |
| 007-Subsystems | te::subsystems | — | struct | 子系统描述符 | te/subsystems/SubsystemDescriptor.h | SubsystemDescriptor | 元数据、依赖列表、优先级、平台过滤；可序列化、反射 |

*来源：用户故事 US-001（应用启动并进入主循环）。契约能力：描述符、注册表、生命周期（Initialize/Start/Stop/Shutdown）。*
