# 007-Subsystems 模块 ABI/API/代码一致性验证报告

**生成时间**: 2026-02-06  
**验证范围**: ABI 文件、Public API 文件、实际代码实现

## 执行摘要

发现 **多个不一致问题**，主要集中在：
1. 类名和方法名不匹配（ABI 使用旧名称）
2. 生命周期方法的位置不同（ABI 中在 Registry，代码中在 Lifecycle）
3. 头文件路径不匹配

## 详细对比

### 1. ISubsystem 接口

| 项目 | ABI 文件 | 实际代码 | 状态 |
|------|----------|----------|------|
| 类名 | `ISubsystem` | `ISubsystem` | ✅ 一致 |
| 头文件 | `te/subsystems/Subsystem.h` | `te/subsystems/subsystem.hpp` | ❌ 不一致 |
| 初始化方法 | `Init()` | `Initialize()` | ❌ 不一致 |
| 关闭方法 | `Shutdown()` | `Shutdown()` | ✅ 一致 |
| 其他方法 | - | `Start()`, `Stop()`, `GetDescriptor()`, `GetName()` | ⚠️ 代码有扩展 |

**问题**: 
- ABI 中使用 `Init()`，但代码实现为 `Initialize()`
- ABI 中头文件路径为 `.h`，实际为 `.hpp`
- 代码中有额外的生命周期方法（Start/Stop）和查询方法（GetDescriptor/GetName）

### 2. Registry/SubsystemRegistry

| 项目 | ABI 文件 | 实际代码 | 状态 |
|------|----------|----------|------|
| 类名 | `SubsystemRegistry` | `Registry` | ❌ 不一致 |
| 头文件 | `te/subsystems/SubsystemRegistry.h` | `te/subsystems/registry.hpp` | ❌ 不一致 |
| 单例方法 | `GetInstance()` | `GetInstance()` | ✅ 一致 |
| 注册方法 | `RegisterSubsystem(ISubsystem*)` | `Register(SubsystemDescriptor, unique_ptr<ISubsystem>)` | ❌ 不一致 |
| 注销方法 | `UnregisterSubsystem(ISubsystem*)` | `Unregister(void const* typeInfo)` | ❌ 不一致 |
| 查询方法 | `GetSubsystem<T>()` | `GetSubsystem<T>()` | ✅ 一致（签名） |
| 生命周期方法 | `InitAll()`, `ShutdownAll()`, `StartAll()`, `StopAll()` | 不在 Registry 中 | ❌ 不一致 |

**问题**:
- 类名完全不同（`SubsystemRegistry` vs `Registry`）
- 注册方法签名完全不同（ABI 使用原始指针，代码使用 unique_ptr 和描述符）
- 生命周期方法在 ABI 中是 Registry 的方法，但在代码中属于 Lifecycle 类

### 3. Lifecycle 类

| 项目 | ABI 文件 | 实际代码 | 状态 |
|------|----------|----------|------|
| 类名 | 未单独列出 | `Lifecycle` | ⚠️ ABI 中缺失 |
| 头文件 | - | `te/subsystems/lifecycle.hpp` | ⚠️ ABI 中缺失 |
| 初始化方法 | `InitAll()` (在 Registry) | `InitializeAll(Registry&)` | ❌ 位置和名称不一致 |
| 启动方法 | `StartAll()` (在 Registry) | `StartAll(Registry&)` | ❌ 位置不一致 |
| 停止方法 | `StopAll()` (在 Registry) | `StopAll(Registry&)` | ❌ 位置不一致 |
| 关闭方法 | `ShutdownAll()` (在 Registry) | `ShutdownAll(Registry&)` | ❌ 位置不一致 |

**问题**:
- ABI 中没有单独的 Lifecycle 类，生命周期方法被列在 SubsystemRegistry 下
- 方法名称不同（`InitAll` vs `InitializeAll`）
- 代码中方法需要 Registry 引用参数，ABI 中没有说明

### 4. SubsystemDescriptor

| 项目 | ABI 文件 | 实际代码 | 状态 |
|------|----------|----------|------|
| 类型 | `struct SubsystemDescriptor` | `struct SubsystemDescriptor` | ✅ 一致 |
| 头文件 | `te/subsystems/SubsystemDescriptor.h` | `te/subsystems/descriptor.hpp` | ❌ 不一致 |
| 字段 | 元数据、依赖列表、优先级、平台过滤 | `typeInfo`, `name`, `version`, `dependencies`, `dependencyCount`, `priority`, `platformFilter`, `configData` | ✅ 基本一致 |

**问题**:
- 头文件路径不匹配（`.h` vs `.hpp`，文件名不同）

### 5. Discovery 功能

| 项目 | Public API | 实际代码 | 状态 |
|------|------------|----------|------|
| 类名 | 未明确 | `Discovery` | ✅ 存在 |
| 扫描方法 | `ScanPlugins` | `ScanPlugins(Registry&, char const* const*, size_t)` | ✅ 一致 |
| 注册方法 | `RegisterFromPlugin` | `RegisterFromPlugin(Registry&, void*)` | ✅ 一致 |
| 注销方法 | - | `UnregisterFromPlugin(Registry&, void*)` | ⚠️ 代码有扩展 |

**问题**:
- Public API 中提到了 Discovery 功能，但 ABI 文件中没有列出

### 6. 新增功能（代码中有但 ABI/API 中未声明）

| 功能 | 说明 | 状态 |
|------|------|------|
| `RegisterResult` | 注册结果结构 | ⚠️ 未在 ABI 中声明 |
| `LifecycleResult` | 生命周期结果结构 | ⚠️ 未在 ABI 中声明 |
| `DiscoveryResult` | 发现结果结构 | ⚠️ 未在 ABI 中声明 |
| `SubsystemState` | 子系统状态枚举 | ⚠️ 未在 ABI 中声明 |
| `Registry::GetSubsystemByName()` | 按名称查询 | ⚠️ 未在 ABI 中声明 |
| `Registry::GetAllSubsystems()` | 获取所有子系统 | ⚠️ 未在 ABI 中声明 |
| `Registry::GetSubsystemState()` | 获取子系统状态 | ⚠️ 未在 ABI 中声明 |
| `Lifecycle::InitializeSubsystem()` | 单个子系统初始化 | ⚠️ 未在 ABI 中声明 |
| `Lifecycle::StartSubsystem()` | 单个子系统启动 | ⚠️ 未在 ABI 中声明 |
| `Lifecycle::StopSubsystem()` | 单个子系统停止 | ⚠️ 未在 ABI 中声明 |
| `Lifecycle::ShutdownSubsystem()` | 单个子系统关闭 | ⚠️ 未在 ABI 中声明 |
| `Lifecycle::CheckDependencies()` | 检查依赖 | ⚠️ 未在 ABI 中声明 |
| `Discovery::UnregisterFromPlugin()` | 从插件注销 | ⚠️ 未在 ABI 中声明 |

## 问题总结

### 严重不一致（需要修复）

1. **类名不匹配**: ABI 中使用 `SubsystemRegistry`，代码中使用 `Registry`
2. **方法名不匹配**: ABI 中使用 `Init()` 和 `InitAll()`，代码中使用 `Initialize()` 和 `InitializeAll()`
3. **生命周期方法位置**: ABI 中在 Registry，代码中在独立的 Lifecycle 类
4. **注册方法签名**: ABI 中使用原始指针，代码使用 unique_ptr 和描述符

### 中等不一致（建议修复）

1. **头文件路径**: ABI 中使用 `.h` 扩展名，代码使用 `.hpp`
2. **头文件命名**: ABI 中使用 `Subsystem.h`，代码使用 `subsystem.hpp`

### 缺失声明（建议补充）

1. `Lifecycle` 类未在 ABI 中单独列出
2. 多个结果结构体未在 ABI 中声明
3. 多个查询和状态管理方法未在 ABI 中声明
4. 单个子系统生命周期方法未在 ABI 中声明

## 建议

### 选项 1: 更新 ABI 文件以匹配代码（推荐）

更新 `specs/_contracts/007-subsystems-ABI.md` 以反映实际的代码实现：
- 将 `SubsystemRegistry` 改为 `Registry`
- 将 `Init()` 改为 `Initialize()`
- 将 `InitAll()` 改为 `InitializeAll()`
- 添加独立的 `Lifecycle` 类
- 更新方法签名以匹配实际实现
- 添加所有新增的功能和类型

### 选项 2: 更新代码以匹配 ABI

修改代码实现以匹配 ABI 文件：
- 重命名 `Registry` 为 `SubsystemRegistry`
- 重命名 `Initialize()` 为 `Init()`
- 将生命周期方法移回 Registry 类
- 修改注册方法签名

### 选项 3: 混合方案

保留代码的改进，但更新 ABI 以反映实际状态，并添加迁移说明。

## 结论

**当前状态**: ❌ **不一致**

代码实现比 ABI 文件描述的功能更丰富和完善，但两者之间存在显著差异。建议采用**选项 1**，更新 ABI 文件以匹配实际代码实现，这样可以：
1. 保持代码的改进（更好的错误处理、状态管理等）
2. 使 ABI 文件反映实际可用的 API
3. 为下游消费者提供准确的接口文档
