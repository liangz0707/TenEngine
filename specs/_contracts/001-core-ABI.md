# 001-Core 模块 ABI

- **契约**：[001-core-public-api.md](./001-core-public-api.md)（能力与类型描述）
- **本文件**：001-Core 对外 ABI 显式表，供实现与下游统一命名空间、头文件与符号。
- **CMake Target 名称**：**`te_core`**（project name `te_core`，不是 `TenEngine_Core`）。下游在 `target_link_libraries` 中应使用 **`te_core`**，不是 `TenEngine_Core`。
- **统一分配接口**：**所有内容分配**均通过 **IAllocator/Allocator** 进行；**现成分配**（默认分配器、池、预分配等）也通过**同一套** **Allocator** 暴露（如 GetDefaultAllocator() 返回 Allocator*），调用方仅依赖 Allocator。
- **命名**：成员方法与自由函数采用**首字母大写的驼峰**（PascalCase）；所有方法在说明列给出**完整函数签名**。
- **命名空间**：te::core（头文件路径 te/core/）。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| 001-Core | te::core | — | 全局堆分配 | te/core/alloc.h | Alloc | `void* Alloc(size_t size, size_t alignment);` 失败返回 nullptr，size==0 或非法 alignment 返回 nullptr |
| 001-Core | te::core | — | 全局堆释放 | te/core/alloc.h | Free | `void Free(void* ptr);` ptr 可为 nullptr；double-free 为 no-op |
| 001-Core | te::core | — | 对齐分配 | te/core/alloc.h | AllocAligned | `void* AllocAligned(size_t size, size_t alignment);` 显式对齐分配，失败返回 nullptr；与 Alloc 等价但语义更明确 |
| 001-Core | te::core | — | 重新分配内存（可选） | te/core/alloc.h | Realloc | `void* Realloc(void* ptr, size_t newSize);` 重新分配内存，失败返回 nullptr；ptr 为 nullptr 时等价于 Alloc；可选功能 |
| 001-Core | te::core | — | 内存统计（可选） | te/core/alloc.h | GetMemoryStats | `MemoryStats GetMemoryStats();` 返回内存使用统计信息（已分配、峰值等）；可选功能 |
| 001-Core | te::core | Allocator | 抽象分配器接口 | te/core/alloc.h | Allocator::Alloc, Allocator::Free | `void* Alloc(size_t size, size_t alignment);` `void Free(void* ptr);` 虚接口，由 DefaultAllocator 等实现；Free(nullptr) 为 no-op |
| 001-Core | te::core | DefaultAllocator | 默认堆分配器 | te/core/alloc.h | DefaultAllocator | 实现 Allocator，用于默认堆 |
| 001-Core | te::core | — | 获取默认分配器 | te/core/alloc.h | GetDefaultAllocator | `Allocator* GetDefaultAllocator();` 调用方不拥有指针 |
| 001-Core | te::core | — | 内存统计结构 | te/core/alloc.h | MemoryStats | struct { size_t allocated_bytes; size_t peak_bytes; size_t allocation_count; }；可选 |
| 001-Core | te::core | — | 进程级初始化 | te/core/engine.h | Init | `bool Init(InitParams const* params);` 失败返回 false，可重复调用时幂等 |
| 001-Core | te::core | — | 进程级关闭 | te/core/engine.h | Shutdown | `void Shutdown();` 进程退出前调用，Init 之后仅调用一次 |
| 001-Core | te::core | — | 初始化参数 | te/core/engine.h | InitParams | struct，可选：log_path, allocator_policy；下游按需填充 |
| 001-Core | te::core | Thread | 线程 | te/core/thread.h | Thread | 默认构造、`explicit Thread(std::function<void()> fn)`、析构、Join、Detach、Joinable；不可拷贝，可移动 |
| 001-Core | te::core | TLS&lt;T&gt; | 线程局部存储 | te/core/thread.h | TLS | 类模板；Get/Set |
| 001-Core | te::core | Atomic&lt;T&gt; | 原子类型 | te/core/thread.h | Atomic | 类模板；Load, Store, Exchange, CompareExchangeStrong |
| 001-Core | te::core | Mutex | 互斥 | te/core/thread.h | Mutex | Lock, Unlock, TryLock；不可拷贝 |
| 001-Core | te::core | LockGuard | RAII 锁 | te/core/thread.h | LockGuard | `explicit LockGuard(Mutex& m);` |
| 001-Core | te::core | ConditionVariable | 条件变量 | te/core/thread.h | ConditionVariable | Wait(Mutex& m), NotifyOne, NotifyAll；不可拷贝 |
| 001-Core | te::core | TaskQueue | 任务队列 | te/core/thread.h | TaskQueue | Submit(std::function&lt;void()&gt; task), RunOne(), Shutdown() |
| 001-Core | te::core | IThreadPool | 抽象线程池接口 | te/core/thread.h | IThreadPool::SubmitTask | `void SubmitTask(TaskCallback callback, void* user_data);` 线程安全 |
| 001-Core | te::core | IThreadPool | 抽象线程池接口 | te/core/thread.h | IThreadPool::SubmitTaskWithPriority | `TaskId SubmitTaskWithPriority(TaskCallback callback, void* user_data, int priority);` 提交带优先级的任务；priority 越大优先级越高；返回 TaskId |
| 001-Core | te::core | IThreadPool | 抽象线程池接口 | te/core/thread.h | IThreadPool::CancelTask | `bool CancelTask(TaskId taskId);` 取消任务；返回 true 表示成功取消，false 表示任务已完成或不存在 |
| 001-Core | te::core | IThreadPool | 抽象线程池接口 | te/core/thread.h | IThreadPool::GetTaskStatus | `TaskStatus GetTaskStatus(TaskId taskId) const;` 查询任务状态；返回 Pending/Loading/Completed/Failed/Cancelled |
| 001-Core | te::core | IThreadPool | 抽象线程池接口 | te/core/thread.h | IThreadPool::SetCallbackThread | `void SetCallbackThread(CallbackThreadType threadType);` 设置回调执行线程；MainThread 或 WorkerThread |
| 001-Core | te::core | — | 任务回调类型 | te/core/thread.h | TaskCallback | `void (*TaskCallback)(void* user_data);` 在工作线程中执行 |
| 001-Core | te::core | — | 任务 ID 类型 | te/core/thread.h | TaskId | 不透明句柄，由 SubmitTaskWithPriority 返回 |
| 001-Core | te::core | — | 任务状态枚举 | te/core/thread.h | TaskStatus | `enum class TaskStatus { Pending, Loading, Completed, Failed, Cancelled };` |
| 001-Core | te::core | — | 回调线程类型枚举 | te/core/thread.h | CallbackThreadType | `enum class CallbackThreadType { MainThread, WorkerThread };` |
| 001-Core | te::core | — | 获取全局线程池 | te/core/thread.h | GetThreadPool | `IThreadPool* GetThreadPool();` 调用方不拥有指针 |
| 001-Core | te::core | — | 平台宏 | te/core/platform.h | TE_PLATFORM_WINDOWS/LINUX/MACOS/ANDROID/IOS | 1 表示当前平台，0 表示非当前平台 |
| 001-Core | te::core | — | 读文件 | te/core/platform.h | FileRead | `std::optional<std::vector<std::uint8_t>> FileRead(std::string const& path);` 失败返回 empty |
| 001-Core | te::core | — | 写文件（字节） | te/core/platform.h | FileWrite | `bool FileWrite(std::string const& path, std::vector<std::uint8_t> const& data);` |
| 001-Core | te::core | — | 写文件（字符串） | te/core/platform.h | FileWrite | `bool FileWrite(std::string const& path, std::string const& data);` |
| 001-Core | te::core | — | 读文件（二进制，指定范围） | te/core/platform.h | FileReadBinary | `bool FileReadBinary(std::string const& path, void* outData, size_t* outSize, size_t offset, size_t size);` 读取文件的指定范围；outData 由调用方分配，outSize 返回实际读取大小；失败返回 false |
| 001-Core | te::core | — | 写文件（二进制，指定位置） | te/core/platform.h | FileWriteBinary | `bool FileWriteBinary(std::string const& path, void const* data, size_t size, size_t offset);` 写入文件的指定位置；offset 为 SIZE_MAX 时追加到文件末尾；失败返回 false |
| 001-Core | te::core | — | 获取文件大小 | te/core/platform.h | FileGetSize | `size_t FileGetSize(std::string const& path);` 返回文件大小（字节），失败返回 0 |
| 001-Core | te::core | — | 检查文件是否存在 | te/core/platform.h | FileExists | `bool FileExists(std::string const& path);` 返回文件是否存在 |
| 001-Core | te::core | — | 目录项类型 | te/core/platform.h | DirEntry | std::string（目录项名） |
| 001-Core | te::core | — | 枚举目录 | te/core/platform.h | DirectoryEnumerate | `std::vector<DirEntry> DirectoryEnumerate(std::string const& path);` 失败返回空 vector |
| 001-Core | te::core | — | 墙钟时间 | te/core/platform.h | Time | `double Time();` 自 epoch 的秒数 |
| 001-Core | te::core | — | 高精度计时 | te/core/platform.h | HighResolutionTimer | `double HighResolutionTimer();` 单调高精度秒数 |
| 001-Core | te::core | — | 环境变量 | te/core/platform.h | GetEnv | `std::optional<std::string> GetEnv(std::string const& name);` 未设置返回 empty |
| 001-Core | te::core | — | 路径规范化 | te/core/platform.h | PathNormalize | `std::string PathNormalize(std::string const& path);` 解析 . / .. 与分隔符 |
| 001-Core | te::core | — | 路径拼接 | te/core/platform.h | PathJoin | `std::string PathJoin(std::string const& path1, std::string const& path2);` 拼接路径组件，自动处理分隔符 |
| 001-Core | te::core | — | 获取目录部分 | te/core/platform.h | PathGetDirectory | `std::string PathGetDirectory(std::string const& path);` 返回路径的目录部分（不含文件名） |
| 001-Core | te::core | — | 获取文件名 | te/core/platform.h | PathGetFileName | `std::string PathGetFileName(std::string const& path);` 返回路径的文件名部分（含扩展名） |
| 001-Core | te::core | — | 获取扩展名 | te/core/platform.h | PathGetExtension | `std::string PathGetExtension(std::string const& path);` 返回路径的扩展名（含点号，如 ".txt"） |
| 001-Core | te::core | — | 解析相对路径 | te/core/platform.h | PathResolveRelative | `std::string PathResolveRelative(std::string const& basePath, std::string const& relativePath);` 将相对路径解析为基于 basePath 的绝对路径 |
| 001-Core | te::core | — | 日志级别 | te/core/log.h | LogLevel | `enum class LogLevel { Debug, Info, Warn, Error };` |
| 001-Core | te::core | LogSink | 日志输出通道 | te/core/log.h | LogSink::Write | `virtual void Write(LogLevel level, char const* message) = 0;` 可重定向与过滤 |
| 001-Core | te::core | — | 写日志 | te/core/log.h | Log | `void Log(LogLevel level, char const* message);` 线程安全 |
| 001-Core | te::core | — | 设置日志级别过滤 | te/core/log.h | LogSetLevelFilter | `void LogSetLevelFilter(LogLevel min_level);` |
| 001-Core | te::core | — | 设置 stderr 阈值 | te/core/log.h | LogSetStderrThreshold | `void LogSetStderrThreshold(LogLevel stderr_level);` |
| 001-Core | te::core | — | 设置自定义 Sink | te/core/log.h | LogSetSink | `void LogSetSink(LogSink* sink);` nullptr 恢复默认 |
| 001-Core | te::core | — | 断言 | te/core/log.h | Assert | `void Assert(bool condition);` 条件为假时调用 CrashHandler 后 abort |
| 001-Core | te::core | — | 崩溃回调类型 | te/core/log.h | CrashHandlerFn | `void (*CrashHandlerFn)(char const* message);` |
| 001-Core | te::core | — | 设置崩溃钩子 | te/core/log.h | SetCrashHandler | `void SetCrashHandler(CrashHandlerFn fn);` |
| 001-Core | te::core | — | 宏 | 统一校验（Warning） | te/core/check.h | CheckWarning(condition[, message]) | 条件为假时记录 Warning；可编译选项启用 |
| 001-Core | te::core | — | 宏 | 统一校验（Error） | te/core/check.h | CheckError(condition[, message]) | 条件为假时记录 Error 并可选中止/返回 |
| 001-Core | te::core | — | 标量类型 | te/core/math.h | Scalar | float |
| 001-Core | te::core | Vector2/3/4 | 向量 | te/core/math.h | Vector2, Vector3, Vector4 | struct；x,y[,z][,w]；operator[] |
| 001-Core | te::core | Matrix3/4 | 矩阵 | te/core/math.h | Matrix3, Matrix4 | struct；m[3][3]/m[4][4]；operator[](row) |
| 001-Core | te::core | Quaternion | 四元数 | te/core/math.h | Quaternion | struct；x, y, z, w |
| 001-Core | te::core | AABB | 轴对齐包围盒 | te/core/math.h | AABB | struct；Vector3 min, max |
| 001-Core | te::core | Ray | 射线 | te/core/math.h | Ray | struct；Vector3 origin, direction |
| 001-Core | te::core | — | 线性插值 | te/core/math.h | Lerp | Scalar/Vector2/3/4 Lerp(...); |
| 001-Core | te::core | — | 点积 | te/core/math.h | Dot | Scalar Dot(Vector2/3/4 const& a, Vector2/3/4 const& b); |
| 001-Core | te::core | — | 叉积 | te/core/math.h | Cross | Vector3 Cross(Vector3 const& a, Vector3 const& b); |
| 001-Core | te::core | — | 长度 | te/core/math.h | Length | Scalar Length(Vector2/3/4 const& v); |
| 001-Core | te::core | — | 归一化 | te/core/math.h | Normalize | Vector2/3/4 Normalize(Vector2/3/4 const& v); 零向量返回零向量 |
| 001-Core | te::core | — | 动态数组类型 | te/core/containers.h | Array&lt;T, Allocator&gt; | std::vector&lt;T, Allocator&gt; 等价 |
| 001-Core | te::core | — | 哈希表类型 | te/core/containers.h | Map&lt;K,V,...&gt; | std::unordered_map 等价；支持自定义分配器 |
| 001-Core | te::core | — | 字符串类型 | te/core/containers.h | String | std::string |
| 001-Core | te::core | — | 独占指针类型 | te/core/containers.h | UniquePtr&lt;T&gt; | std::unique_ptr&lt;T&gt; |
| 001-Core | te::core | — | 共享指针类型 | te/core/containers.h | SharedPtr&lt;T&gt; | std::shared_ptr&lt;T&gt; |
| 001-Core | te::core | — | 模块句柄类型 | te/core/module_load.h | ModuleHandle | void*（LoadLibrary 返回） |
| 001-Core | te::core | — | 加载动态库 | te/core/module_load.h | LoadLibrary | `ModuleHandle LoadLibrary(char const* path);` 失败返回 nullptr |
| 001-Core | te::core | — | 卸载动态库 | te/core/module_load.h | UnloadLibrary | `void UnloadLibrary(ModuleHandle handle);` nullptr 为 no-op |
| 001-Core | te::core | — | 取符号地址 | te/core/module_load.h | GetSymbol | `void* GetSymbol(ModuleHandle handle, char const* name);` 未找到返回 nullptr |
| 001-Core | te::core | — | 模块初始化回调类型 | te/core/module_load.h | ModuleInitFn | `void (*ModuleInitFn)();` |
| 001-Core | te::core | — | 模块关闭回调类型 | te/core/module_load.h | ModuleShutdownFn | `void (*ModuleShutdownFn)();` |
| 001-Core | te::core | — | 注册模块初始化 | te/core/module_load.h | RegisterModuleInit | `void RegisterModuleInit(ModuleInitFn fn);` 注册顺序调用 |
| 001-Core | te::core | — | 注册模块关闭 | te/core/module_load.h | RegisterModuleShutdown | `void RegisterModuleShutdown(ModuleShutdownFn fn);` 逆序调用 |
| 001-Core | te::core | — | 执行模块初始化 | te/core/module_load.h | RunModuleInit | `void RunModuleInit();` 加载模块后调用 |
| 001-Core | te::core | — | 执行模块关闭 | te/core/module_load.h | RunModuleShutdown | `void RunModuleShutdown();` 卸载前调用 |

**平台与宏**：引擎支持 **Android、iOS** 等平台；**可以通过宏来判断执行哪一段代码**（如 TE_PLATFORM_ANDROID、TE_PLATFORM_IOS、TE_PLATFORM_WIN、TE_PLATFORM_LINUX、TE_PLATFORM_MACOS），编译时选择平台相关实现路径。平台检测与宏由 Platform 子模块或公共头提供。

**说明**：命名空间与头文件为下游 include 与 link 的唯一依据。新增条目采用 te::core 与 PascalCase 命名。本表由 plan 001-core-fullversion-002 契约更新同步（2026-01-30）。

**增强更新（2026-02-06）**：为支持资源模块重构后的需求，新增文件 I/O 增强（FileReadBinary/FileWriteBinary/FileGetSize/FileExists）、异步操作增强（IThreadPool 扩展：SubmitTaskWithPriority/CancelTask/GetTaskStatus/SetCallbackThread）、内存管理增强（AllocAligned/Realloc/GetMemoryStats）、路径操作增强（PathJoin/PathGetDirectory/PathGetFileName/PathGetExtension/PathResolveRelative）。所有新增接口均向后兼容。

---

010-Shader 依赖的接口 TODO 已迁移至本模块契约 [001-core-public-api.md](./001-core-public-api.md) 的 TODO 列表；本文件仅保留 ABI 表与实现说明。
