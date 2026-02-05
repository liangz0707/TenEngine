# 契约：001-Core 模块对外 API

## 适用模块

- **实现方**：001-Core（L0 根模块：内存、线程、平台（文件/时间/环境）、日志、数学、容器、模块加载；无反射/ECS）
- **对应规格**：`docs/module-specs/001-core.md`
- **依赖**：无

## 消费者

- 002-Object、003-Application、004-Scene、005-Entity、006-Input、007-Subsystems、008-RHI、009-RenderCore、010-Shader、012-Mesh、013-Resource、014-Physics、015-Animation、016-Audio、017-UICore、020-Pipeline、022-2D、023-Terrain、024-Editor、026-Networking、027-XR、028-Texture

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| Allocator / 内存块 | 抽象分配器 Allocator、DefaultAllocator；Alloc(size, alignment)、Free(ptr)；GetDefaultAllocator() | 分配后直至显式释放；Free(nullptr) 为 no-op |
| Task/Job、Thread、TLS、Atomic | Thread、TLS\<T\>、Atomic\<T\>、Mutex、LockGuard、ConditionVariable、TaskQueue、IThreadPool、TaskCallback、GetThreadPool | 按 C++ 或实现约定 |
| 平台句柄与宏 | TE_PLATFORM_WINDOWS/LINUX/MACOS/ANDROID/IOS；FileRead/FileWrite、DirectoryEnumerate、DirEntry、Time、HighResolutionTimer、GetEnv、PathNormalize | 按具体 API 约定 |
| 数学类型 | Scalar、Vector2/3/4、Matrix3/4、Quaternion、AABB、Ray；Lerp、Dot、Cross、Length、Normalize | 值类型或调用方管理 |
| 容器 | Array\<T\>、Map\<K,V\>、String、UniquePtr\<T\>、SharedPtr\<T\>（可指定分配器） | 调用方管理 |
| 日志与校验 | LogLevel、LogSink、Log、LogSetLevelFilter/LogSetStderrThreshold/LogSetSink、Assert、CrashHandlerFn、SetCrashHandler；CheckWarning、CheckError 宏 | 进程级 |
| 引擎与模块 | Init、Shutdown、InitParams；ModuleHandle、LoadLibrary、UnloadLibrary、GetSymbol；ModuleInitFn/ModuleShutdownFn、RegisterModuleInit/RegisterModuleShutdown、RunModuleInit/RunModuleShutdown | Init 后直至 Shutdown；Load 后直至 Unload |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 内存管理 | Alloc/Free、Allocator 接口、GetDefaultAllocator；分配失败返回 nullptr；可选池化与统计 |
| 2 | 线程管理 | Thread、TLS、Atomic、Mutex、LockGuard、ConditionVariable、TaskQueue、IThreadPool::SubmitTask、GetThreadPool；语义明确 |
| 3 | 平台抽象 | 文件 FileRead/FileWrite、目录 DirectoryEnumerate、时间 Time/HighResolutionTimer、GetEnv、PathNormalize；平台宏 TE_PLATFORM_* 编译时选择 |
| 4 | 日志 | LogLevel、Log、LogSetLevelFilter/LogSetStderrThreshold/LogSetSink、Assert、SetCrashHandler；可重定向与过滤 |
| 5 | 数学 | Scalar、Vector2/3/4、Matrix3/4、Quaternion、AABB、Ray、Lerp、Dot、Cross、Length、Normalize；无 GPU 依赖 |
| 6 | 容器 | Array、Map、String、UniquePtr、SharedPtr；无反射/ECS，可与自定义分配器配合 |
| 7 | 模块加载 | LoadLibrary、UnloadLibrary、GetSymbol；RegisterModuleInit/RegisterModuleShutdown、RunModuleInit/RunModuleShutdown；与构建/插件配合 |

命名空间 `te::core`；头文件 alloc.h、engine.h、thread.h、platform.h、log.h、check.h、math.h、containers.h、module_load.h。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化（MAJOR.MINOR.PATCH）；破坏性变更递增 MAJOR。

## 约束

- 主工程或上层须先完成 Core 初始化（Init）后再调用各子能力；卸载前释放由 Core 分配的资源并停止使用句柄。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| （初始） | 从 001-engine-core-module spec 提炼 |
| T0 更新 | 对齐 T0 架构；移除 ECS/序列化（归 002-Object）；消费者按依赖图 |
| 2026-02-05 | 统一目录；能力列表用表格描述类型与句柄、能力；不引用 ABI 文件 |
