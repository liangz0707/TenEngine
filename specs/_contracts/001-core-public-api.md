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
| Allocator / 内存块 | 抽象分配器 Allocator、DefaultAllocator；Alloc(size, alignment)、AllocAligned(size, alignment)、Free(ptr)、Realloc(ptr, newSize)（可选）；GetDefaultAllocator()、GetMemoryStats()（可选） | 分配后直至显式释放；Free(nullptr) 为 no-op |
| Task/Job、Thread、TLS、Atomic | Thread、TLS\<T\>、Atomic\<T\>、Mutex、LockGuard、ConditionVariable、TaskQueue、IThreadPool、ITaskExecutor、ExecutorType、TaskCallback、TaskId、TaskStatus、GetThreadPool；IThreadPool::SubmitTask、SetCallbackThread、ProcessMainThreadCallbacks、GetWorkerExecutor、GetIOExecutor、GetExecutor、RegisterExecutor、SpawnTask；ITaskExecutor::SubmitTask、SubmitTaskWithPriority、CancelTask、GetTaskStatus | 按 C++ 或实现约定 |
| 平台句柄与宏 | TE_PLATFORM_WINDOWS/LINUX/MACOS/ANDROID/IOS；FileRead/FileWrite、FileReadBinary/FileWriteBinary、FileGetSize/FileExists、DirectoryEnumerate、DirEntry、Time、HighResolutionTimer、GetEnv、PathNormalize、PathJoin、PathGetDirectory/PathGetFileName/PathGetExtension、PathResolveRelative | 按具体 API 约定 |
| 数学类型 | Scalar、Vector2/3/4、Matrix3/4、Quaternion、AABB、Ray；Lerp、Dot、Cross、Length、Normalize | 值类型或调用方管理 |
| 容器 | Array\<T\>、Map\<K,V\>、String、UniquePtr\<T\>、SharedPtr\<T\>（可指定分配器） | 调用方管理 |
| 日志与校验 | LogLevel、LogSink、Log、LogSetLevelFilter/LogSetStderrThreshold/LogSetSink、Assert、CrashHandlerFn、SetCrashHandler；CheckWarning、CheckError 宏 | 进程级 |
| 引擎与模块 | Init、Shutdown、InitParams；ModuleHandle、LoadLibrary、UnloadLibrary、GetSymbol；ModuleInitFn/ModuleShutdownFn、RegisterModuleInit/RegisterModuleShutdown、RunModuleInit/RunModuleShutdown | Init 后直至 Shutdown；Load 后直至 Unload |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 内存管理 | Alloc/Free、AllocAligned、Allocator 接口、GetDefaultAllocator；分配失败返回 nullptr；可选 Realloc、内存统计、池化与统计 |
| 2 | 线程管理 | Thread、TLS、Atomic、Mutex、LockGuard、ConditionVariable、TaskQueue、IThreadPool（SubmitTask、SetCallbackThread、ProcessMainThreadCallbacks、GetWorkerExecutor、GetIOExecutor、GetExecutor、RegisterExecutor、SpawnTask）、ITaskExecutor（SubmitTask、SubmitTaskWithPriority、CancelTask、GetTaskStatus）、ExecutorType、GetThreadPool；主线程回调、专用 IO/Worker Executor、一次性 SpawnTask |
| 3 | 平台抽象 | 文件 FileRead/FileWrite、FileReadBinary/FileWriteBinary（支持大文件和指定偏移）、FileGetSize/FileExists、目录 DirectoryEnumerate、时间 Time/HighResolutionTimer、GetEnv、路径 PathNormalize/PathJoin/PathGetDirectory/PathGetFileName/PathGetExtension/PathResolveRelative；平台宏 TE_PLATFORM_* 编译时选择 |
| 4 | 日志 | LogLevel、Log、LogSetLevelFilter/LogSetStderrThreshold/LogSetSink、Assert、SetCrashHandler；可重定向与过滤 |
| 5 | 数学 | Scalar、Vector2/3/4、Matrix3/4、Quaternion、AABB、Ray、Lerp、Dot、Cross、Length、Normalize；无 GPU 依赖 |
| 6 | 容器 | Array、Map、String、UniquePtr、SharedPtr；无反射/ECS，可与自定义分配器配合 |
| 7 | 模块加载 | LoadLibrary、UnloadLibrary、GetSymbol；RegisterModuleInit/RegisterModuleShutdown、RunModuleInit/RunModuleShutdown；与构建/插件配合 |

命名空间 `te::core`；头文件 alloc.h、engine.h、thread.h、platform.h、log.h、check.h、math.h、containers.h、module_load.h。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化（MAJOR.MINOR.PATCH）；破坏性变更递增 MAJOR。

## 约束

- 主工程或上层须先完成 Core 初始化（Init）后再调用各子能力；卸载前释放由 Core 分配的资源并停止使用句柄。

## TODO 列表

（以下任务来自原 001-core-ABI.md「010-Shader 依赖」TODO。）

- [ ] **010-Shader 依赖**：确保下列接口已实现且可供 010 使用——FileRead（LoadSource 读文件）、Log（编译失败/缓存错误）、Alloc/Free 或 GetDefaultAllocator、PathNormalize、DirectoryEnumerate、FileWrite（SaveCache）、CheckError/Assert；确认通过 010 集成测试。

## 增强功能说明（2026-02-06）

### 文件 I/O 增强

为支持资源模块的大文件加载需求，新增以下接口：

- **FileReadBinary**：读取文件的指定范围（支持偏移和大小），用于大文件分块读取
- **FileWriteBinary**：写入文件的指定位置，支持追加和覆盖模式
- **FileGetSize**：获取文件大小，用于预分配内存
- **FileExists**：检查文件是否存在，用于资源路径验证

### Executor 架构（2026-02）

- **ITaskExecutor**：单线程任务执行器；SubmitTask、SubmitTaskWithPriority、CancelTask、GetTaskStatus
- **ExecutorType**：Worker、IO、Audio、Physics、Network（扩展用）
- **IThreadPool**：SubmitTask（完成回调路由至主线程或 Worker）、SetCallbackThread、ProcessMainThreadCallbacks（主线程每帧调用）、GetWorkerExecutor、GetIOExecutor、GetExecutor、RegisterExecutor、SpawnTask（一次性线程）
- 任务提交与取消通过 GetWorkerExecutor()/GetIOExecutor() 等获取 Executor 后调用

### 内存管理增强

为支持资源模块的大块内存分配需求：

- **AllocAligned**：显式对齐分配接口，确保资源数据满足对齐要求
- **Realloc**（可选）：重新分配内存，用于动态调整资源缓冲区大小
- **GetMemoryStats**（可选）：获取内存统计信息，用于调试和性能分析

### 路径操作增强

为支持资源模块的路径管理需求，新增以下接口：

- **PathJoin**：拼接路径组件，用于构建资源路径
- **PathGetDirectory**：获取目录部分，用于资源路径解析
- **PathGetFileName**：获取文件名，用于资源标识
- **PathGetExtension**：获取扩展名，用于资源类型识别
- **PathResolveRelative**：解析相对路径为绝对路径，用于资源路径规范化

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| （初始） | 从 001-engine-core-module spec 提炼 |
| T0 更新 | 对齐 T0 架构；移除 ECS/序列化（归 002-Object）；消费者按依赖图 |
| 2026-02-05 | 统一目录；能力列表用表格描述类型与句柄、能力；不引用 ABI 文件 |
| 2026-02-06 | 增强功能：文件 I/O（FileReadBinary/FileWriteBinary/FileGetSize/FileExists）、异步操作（任务优先级/取消/状态查询/回调线程控制）、内存管理（AllocAligned/Realloc/GetMemoryStats）、路径操作（PathJoin/PathGetDirectory/PathGetFileName/PathGetExtension/PathResolveRelative）；支持资源模块重构后的需求 |
| 2026-02-12 | Executor 架构：ITaskExecutor、ExecutorType；IThreadPool 重构为 GetWorkerExecutor/GetIOExecutor/ProcessMainThreadCallbacks/SpawnTask；移除 IThreadPool 上的 SubmitTaskWithPriority/CancelTask/GetTaskStatus |
| 2026-02-22 | Verified alignment with code: all types and functions in headers match contract; ITaskExecutor has SubmitTask and SubmitTaskWithPriority; CallbackThreadType enum confirmed; GetThreadPool returns IThreadPool* |
