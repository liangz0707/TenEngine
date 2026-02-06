# 001-Core 模块描述

## 1. 模块简要说明

Core 是 TenEngine 的**基础层根模块**，不依赖任何其他引擎模块。提供内存、线程、平台抽象、日志、数学、容器与模块加载等基础设施，对应 Unreal 的 **Core**、Unity 的 **Engine Core (Native)**，不包含反射与 ECS。

## 2. 详细功能描述

- **内存**：分配器抽象、默认堆分配、对齐分配（AllocAligned）、可选重新分配（Realloc）、可选内存统计（GetMemoryStats）、内存池、调试分配与泄漏追踪。
- **线程**：线程创建与管理、TLS、原子类型、Mutex/ConditionVariable、任务队列骨架、线程池（支持任务优先级、取消、状态查询、回调线程控制）。
- **平台**：文件 I/O（FileRead/FileWrite、FileReadBinary/FileWriteBinary 支持大文件和指定偏移、FileGetSize/FileExists）、目录枚举、时间与高精度计时、环境变量、路径操作（PathNormalize、PathJoin、PathGetDirectory/PathGetFileName/PathGetExtension、PathResolveRelative）、平台检测（Windows/Linux/macOS 等）。
- **日志**：分级日志、输出通道、断言、崩溃报告钩子。
- **数学**：标量/向量/矩阵/四元数、AABB、射线、插值及常用数学函数（与渲染无关的纯数学）。
- **容器**：动态数组、哈希表、字符串、智能指针（无反射、无 ECS）。
- **模块加载**：动态库加载/卸载/符号解析、模块依赖顺序、初始化与关闭回调。

## 3. 实现难度

**中**。平台抽象与多后端（Win/Linux/macOS）需统一接口；内存与线程需兼顾性能与调试能力；不引入反射/ECS 可降低复杂度。

## 4. 操作的资源类型

- **内存**：堆、池、对齐块。
- **文件句柄**：文件读写、目录枚举。
- **系统资源**：线程句柄、TLS 槽、互斥/条件变量、动态库句柄。
- **无 GPU 资源**：本模块不直接操作显存或图形 API。

## 5. 是否有子模块

有。Core 在实现上可拆分为以下逻辑子模块。

### 5.1 子模块说明

| 子模块 | 职责 |
|--------|------|
| Memory | 分配器抽象、堆/池/对齐、调试与泄漏追踪 |
| Thread | 线程、TLS、原子、同步原语、任务队列骨架 |
| Platform | 文件/目录/时间/环境/路径、平台宏与检测 |
| Log | 日志级别、输出通道、断言、崩溃钩子 |
| Math | 向量/矩阵/四元数、AABB、射线、插值 |
| Containers | 动态数组、哈希表、字符串、智能指针 |
| ModuleLoad | 动态库加载、符号解析、模块生命周期 |

### 5.2 具体功能

- **Memory**：Allocator 接口、DefaultAllocator、AllocAligned、可选 Realloc、可选 GetMemoryStats、PoolAllocator、DebugAllocator/LeakTracker。
- **Thread**：Thread、TLS、Atomic、Mutex、ConditionVariable、TaskQueue 骨架、IThreadPool（SubmitTask、SubmitTaskWithPriority、CancelTask、GetTaskStatus、SetCallbackThread）。
- **Platform**：FileRead/Write、FileReadBinary/FileWriteBinary（支持大文件和指定偏移）、FileGetSize/FileExists、DirectoryEnumerate、Time/HighResolutionTimer、GetEnv、PathNormalize、PathJoin、PathGetDirectory/PathGetFileName/PathGetExtension、PathResolveRelative、平台宏。
- **Log**：LogLevel、LogSink、Assert、CrashHandler。
- **Math**：Vector2/3/4、Matrix3/4、Quaternion、AABB、Ray、Lerp 等。
- **Containers**：Array、Map、String、UniquePtr、SharedPtr。
- **ModuleLoad**：LoadLibrary、UnloadLibrary、GetSymbol、ModuleInit/Shutdown 回调。

### 5.3 子模块依赖图

```mermaid
flowchart LR
  Memory[Memory]
  Thread[Thread]
  Platform[Platform]
  Log[Log]
  Math[Math]
  Containers[Containers]
  ModuleLoad[ModuleLoad]
  Platform --> Memory
  Thread --> Memory
  Log --> Platform
  Containers --> Memory
  ModuleLoad --> Platform
  ModuleLoad --> Containers
```

## 6. 模块上下游

### 6.1 和上下游交互、传递的数据类型

- **上游**：无。Core 为根模块。
- **下游**：所有模块均直接或间接依赖 Core。向下游提供：
  - **类型**：基础类型（uint8/16/32/64、float、double）、容器（Array、Map、String）、数学类型（Vector、Matrix、Quaternion）、智能指针、原子类型。
  - **接口**：分配器（含对齐分配、可选重新分配、可选内存统计）、文件 I/O（含大文件二进制操作、文件大小查询、文件存在检查）、时间、日志、线程/同步（含任务优先级、取消、状态查询、回调线程控制）、路径操作（含路径拼接、分解、相对路径解析）、模块加载 API。

### 6.2 上下游依赖图

```mermaid
flowchart TB
  Core[001-Core]
  Object[002-Object]
  App[003-Application]
  Scene[004-Scene]
  RHI[008-RHI]
  Core --> Object
  Core --> App
  Core --> Scene
  Core --> RHI
  style Core fill:#e0f0e0
```

（仅示意直接下游；Object、Application、Scene、Entity、Input、Subsystems、RHI、RenderCore、Shader、Mesh、Resource、Physics、Animation、Audio、UICore、Pipeline、2D、Terrain、Editor、Networking、XR 等均依赖 Core。）

## 7. 依赖的外部内容

| 类别 | 内容 |
|------|------|
| **技术/标准** | C++17 或以上（std::filesystem、std::atomic、std::thread 等） |
| **平台 API** | Win32（文件、线程、动态库、高精度计时）；POSIX（pthread、dlopen、clock_gettime）；macOS（Mach 线程、dyld） |
| **外部库（可选）** | EASTL、jemalloc/tcmalloc、spdlog 等；需通过 Core 抽象对外统一接口，避免上层直接依赖 |
| **构建工具** | CMake/Meson 等，与 ModuleLoad 配合用于按模块编译与加载 |
| **协议** | 无业务协议依赖 |

## 8. 增强功能（2026-02-06）

为支持资源模块重构后的需求，Core 模块新增以下增强功能：

### 8.1 文件 I/O 增强

- **FileReadBinary**：读取文件的指定范围（支持偏移和大小），用于大文件分块读取，避免一次性加载整个文件到内存
- **FileWriteBinary**：写入文件的指定位置，支持追加和覆盖模式，用于大文件写入
- **FileGetSize**：获取文件大小，用于预分配内存和资源大小验证
- **FileExists**：检查文件是否存在，用于资源路径验证

### 8.2 异步操作增强

扩展 IThreadPool 接口，支持资源模块的异步加载需求：

- **SubmitTaskWithPriority**：提交带优先级的任务，资源加载任务可设置优先级
- **CancelTask**：取消正在执行的任务，支持资源加载取消
- **GetTaskStatus**：查询任务执行状态，用于资源加载状态跟踪
- **SetCallbackThread**：设置回调执行线程（主线程或工作线程），确保资源加载回调在正确线程执行

### 8.3 内存管理增强

为支持资源模块的大块内存分配需求：

- **AllocAligned**：显式对齐分配接口，确保资源数据满足对齐要求（如纹理数据需要特定对齐）
- **Realloc**（可选）：重新分配内存，用于动态调整资源缓冲区大小
- **GetMemoryStats**（可选）：获取内存统计信息，用于调试和性能分析

### 8.4 路径操作增强

为支持资源模块的路径管理需求，新增以下接口：

- **PathJoin**：拼接路径组件，用于构建资源路径
- **PathGetDirectory**：获取目录部分，用于资源路径解析
- **PathGetFileName**：获取文件名，用于资源标识
- **PathGetExtension**：获取扩展名，用于资源类型识别
- **PathResolveRelative**：解析相对路径为绝对路径，用于资源路径规范化

这些增强功能均向后兼容，不影响现有 API 的使用。
