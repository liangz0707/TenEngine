# 001-Core 模块 ABI

- **契约**：[001-core-public-api.md](./001-core-public-api.md)（能力与类型描述）
- **本文件**：001-Core 对外 ABI 显式表，供实现与下游统一命名空间、头文件与符号。
- **CMake Target 名称**：**`te_core`**（project name `te_core`，不是 `TenEngine_Core`）。下游在 `target_link_libraries` 中应使用 **`te_core`**，不是 `TenEngine_Core`。
- **统一分配接口**：**所有内容分配**均通过 **IAllocator/Allocator** 进行；**现成分配**（默认分配器、池、预分配等）也通过**同一套** **Allocator** 暴露（如 GetDefaultAllocator() 返回 Allocator*），调用方仅依赖 Allocator。
- **命名**：成员方法与自由函数采用**首字母大写的驼峰**（PascalCase）；所有方法在说明列给出**完整函数签名**。
- **命名空间**：TenEngine::core（实现可用 te::core，头文件路径 te/core/ = TenEngine/core/）。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| 001-Core | TenEngine::core | — | 全局堆分配 | TenEngine/core/alloc.h | Alloc | `void* Alloc(size_t size, size_t alignment);` 失败返回 nullptr，size==0 或非法 alignment 返回 nullptr |
| 001-Core | TenEngine::core | — | 全局堆释放 | TenEngine/core/alloc.h | Free | `void Free(void* ptr);` ptr 可为 nullptr；double-free 为 no-op |
| 001-Core | TenEngine::core | Allocator | 抽象分配器接口 | TenEngine/core/alloc.h | Allocator::Alloc, Allocator::Free | `void* Alloc(size_t size, size_t alignment);` `void Free(void* ptr);` 虚接口，由 DefaultAllocator 等实现；Free(nullptr) 为 no-op |
| 001-Core | TenEngine::core | DefaultAllocator | 默认堆分配器 | TenEngine/core/alloc.h | DefaultAllocator | 实现 Allocator，用于默认堆 |
| 001-Core | TenEngine::core | — | 获取默认分配器 | TenEngine/core/alloc.h | GetDefaultAllocator | `Allocator* GetDefaultAllocator();` 调用方不拥有指针 |
| 001-Core | TenEngine::core | — | 进程级初始化 | TenEngine/core/engine.h | Init | `bool Init(InitParams const* params);` 失败返回 false，可重复调用时幂等 |
| 001-Core | TenEngine::core | — | 进程级关闭 | TenEngine/core/engine.h | Shutdown | `void Shutdown();` 进程退出前调用，Init 之后仅调用一次 |
| 001-Core | TenEngine::core | — | 初始化参数 | TenEngine/core/engine.h | InitParams | struct，可选：log_path, allocator_policy；下游按需填充 |
| 001-Core | TenEngine::core | Thread | 线程 | TenEngine/core/thread.h | Thread | 默认构造、`explicit Thread(std::function<void()> fn)`、析构、Join、Detach、Joinable；不可拷贝，可移动 |
| 001-Core | TenEngine::core | TLS&lt;T&gt; | 线程局部存储 | TenEngine/core/thread.h | TLS | 类模板；Get/Set |
| 001-Core | TenEngine::core | Atomic&lt;T&gt; | 原子类型 | TenEngine/core/thread.h | Atomic | 类模板；Load, Store, Exchange, CompareExchangeStrong |
| 001-Core | TenEngine::core | Mutex | 互斥 | TenEngine/core/thread.h | Mutex | Lock, Unlock, TryLock；不可拷贝 |
| 001-Core | TenEngine::core | LockGuard | RAII 锁 | TenEngine/core/thread.h | LockGuard | `explicit LockGuard(Mutex& m);` |
| 001-Core | TenEngine::core | ConditionVariable | 条件变量 | TenEngine/core/thread.h | ConditionVariable | Wait(Mutex& m), NotifyOne, NotifyAll；不可拷贝 |
| 001-Core | TenEngine::core | TaskQueue | 任务队列 | TenEngine/core/thread.h | TaskQueue | Submit(std::function&lt;void()&gt; task), RunOne(), Shutdown() |
| 001-Core | TenEngine::core | IThreadPool | 抽象线程池接口 | TenEngine/core/thread.h | IThreadPool::SubmitTask | `void SubmitTask(TaskCallback callback, void* user_data);` 线程安全 |
| 001-Core | TenEngine::core | — | 任务回调类型 | TenEngine/core/thread.h | TaskCallback | `void (*TaskCallback)(void* user_data);` 在工作线程中执行 |
| 001-Core | TenEngine::core | — | 获取全局线程池 | TenEngine/core/thread.h | GetThreadPool | `IThreadPool* GetThreadPool();` 调用方不拥有指针 |
| 001-Core | TenEngine::core | — | 平台宏 | TenEngine/core/platform.h | TE_PLATFORM_WINDOWS/LINUX/MACOS/ANDROID/IOS | 1 表示当前平台，0 表示非当前平台 |
| 001-Core | TenEngine::core | — | 读文件 | TenEngine/core/platform.h | FileRead | `std::optional<std::vector<std::uint8_t>> FileRead(std::string const& path);` 失败返回 empty |
| 001-Core | TenEngine::core | — | 写文件（字节） | TenEngine/core/platform.h | FileWrite | `bool FileWrite(std::string const& path, std::vector<std::uint8_t> const& data);` |
| 001-Core | TenEngine::core | — | 写文件（字符串） | TenEngine/core/platform.h | FileWrite | `bool FileWrite(std::string const& path, std::string const& data);` |
| 001-Core | TenEngine::core | — | 目录项类型 | TenEngine/core/platform.h | DirEntry | std::string（目录项名） |
| 001-Core | TenEngine::core | — | 枚举目录 | TenEngine/core/platform.h | DirectoryEnumerate | `std::vector<DirEntry> DirectoryEnumerate(std::string const& path);` 失败返回空 vector |
| 001-Core | TenEngine::core | — | 墙钟时间 | TenEngine/core/platform.h | Time | `double Time();` 自 epoch 的秒数 |
| 001-Core | TenEngine::core | — | 高精度计时 | TenEngine/core/platform.h | HighResolutionTimer | `double HighResolutionTimer();` 单调高精度秒数 |
| 001-Core | TenEngine::core | — | 环境变量 | TenEngine/core/platform.h | GetEnv | `std::optional<std::string> GetEnv(std::string const& name);` 未设置返回 empty |
| 001-Core | TenEngine::core | — | 路径规范化 | TenEngine/core/platform.h | PathNormalize | `std::string PathNormalize(std::string const& path);` 解析 . / .. 与分隔符 |
| 001-Core | TenEngine::core | — | 日志级别 | TenEngine/core/log.h | LogLevel | `enum class LogLevel { Debug, Info, Warn, Error };` |
| 001-Core | TenEngine::core | LogSink | 日志输出通道 | TenEngine/core/log.h | LogSink::Write | `virtual void Write(LogLevel level, char const* message) = 0;` 可重定向与过滤 |
| 001-Core | TenEngine::core | — | 写日志 | TenEngine/core/log.h | Log | `void Log(LogLevel level, char const* message);` 线程安全 |
| 001-Core | TenEngine::core | — | 设置日志级别过滤 | TenEngine/core/log.h | LogSetLevelFilter | `void LogSetLevelFilter(LogLevel min_level);` |
| 001-Core | TenEngine::core | — | 设置 stderr 阈值 | TenEngine/core/log.h | LogSetStderrThreshold | `void LogSetStderrThreshold(LogLevel stderr_level);` |
| 001-Core | TenEngine::core | — | 设置自定义 Sink | TenEngine/core/log.h | LogSetSink | `void LogSetSink(LogSink* sink);` nullptr 恢复默认 |
| 001-Core | TenEngine::core | — | 断言 | TenEngine/core/log.h | Assert | `void Assert(bool condition);` 条件为假时调用 CrashHandler 后 abort |
| 001-Core | TenEngine::core | — | 崩溃回调类型 | TenEngine/core/log.h | CrashHandlerFn | `void (*CrashHandlerFn)(char const* message);` |
| 001-Core | TenEngine::core | — | 设置崩溃钩子 | TenEngine/core/log.h | SetCrashHandler | `void SetCrashHandler(CrashHandlerFn fn);` |
| 001-Core | TenEngine::core | — | 宏 | 统一校验（Warning） | TenEngine/core/check.h | CheckWarning(condition[, message]) | 条件为假时记录 Warning；可编译选项启用 |
| 001-Core | TenEngine::core | — | 宏 | 统一校验（Error） | TenEngine/core/check.h | CheckError(condition[, message]) | 条件为假时记录 Error 并可选中止/返回 |
| 001-Core | TenEngine::core | — | 标量类型 | TenEngine/core/math.h | Scalar | float |
| 001-Core | TenEngine::core | Vector2/3/4 | 向量 | TenEngine/core/math.h | Vector2, Vector3, Vector4 | struct；x,y[,z][,w]；operator[] |
| 001-Core | TenEngine::core | Matrix3/4 | 矩阵 | TenEngine/core/math.h | Matrix3, Matrix4 | struct；m[3][3]/m[4][4]；operator[](row) |
| 001-Core | TenEngine::core | Quaternion | 四元数 | TenEngine/core/math.h | Quaternion | struct；x, y, z, w |
| 001-Core | TenEngine::core | AABB | 轴对齐包围盒 | TenEngine/core/math.h | AABB | struct；Vector3 min, max |
| 001-Core | TenEngine::core | Ray | 射线 | TenEngine/core/math.h | Ray | struct；Vector3 origin, direction |
| 001-Core | TenEngine::core | — | 线性插值 | TenEngine/core/math.h | Lerp | Scalar/Vector2/3/4 Lerp(...); |
| 001-Core | TenEngine::core | — | 点积 | TenEngine/core/math.h | Dot | Scalar Dot(Vector2/3/4 const& a, Vector2/3/4 const& b); |
| 001-Core | TenEngine::core | — | 叉积 | TenEngine/core/math.h | Cross | Vector3 Cross(Vector3 const& a, Vector3 const& b); |
| 001-Core | TenEngine::core | — | 长度 | TenEngine/core/math.h | Length | Scalar Length(Vector2/3/4 const& v); |
| 001-Core | TenEngine::core | — | 归一化 | TenEngine/core/math.h | Normalize | Vector2/3/4 Normalize(Vector2/3/4 const& v); 零向量返回零向量 |
| 001-Core | TenEngine::core | — | 动态数组类型 | TenEngine/core/containers.h | Array&lt;T, Allocator&gt; | std::vector&lt;T, Allocator&gt; 等价 |
| 001-Core | TenEngine::core | — | 哈希表类型 | TenEngine/core/containers.h | Map&lt;K,V,...&gt; | std::unordered_map 等价；支持自定义分配器 |
| 001-Core | TenEngine::core | — | 字符串类型 | TenEngine/core/containers.h | String | std::string |
| 001-Core | TenEngine::core | — | 独占指针类型 | TenEngine/core/containers.h | UniquePtr&lt;T&gt; | std::unique_ptr&lt;T&gt; |
| 001-Core | TenEngine::core | — | 共享指针类型 | TenEngine/core/containers.h | SharedPtr&lt;T&gt; | std::shared_ptr&lt;T&gt; |
| 001-Core | TenEngine::core | — | 模块句柄类型 | TenEngine/core/module_load.h | ModuleHandle | void*（LoadLibrary 返回） |
| 001-Core | TenEngine::core | — | 加载动态库 | TenEngine/core/module_load.h | LoadLibrary | `ModuleHandle LoadLibrary(char const* path);` 失败返回 nullptr |
| 001-Core | TenEngine::core | — | 卸载动态库 | TenEngine/core/module_load.h | UnloadLibrary | `void UnloadLibrary(ModuleHandle handle);` nullptr 为 no-op |
| 001-Core | TenEngine::core | — | 取符号地址 | TenEngine/core/module_load.h | GetSymbol | `void* GetSymbol(ModuleHandle handle, char const* name);` 未找到返回 nullptr |
| 001-Core | TenEngine::core | — | 模块初始化回调类型 | TenEngine/core/module_load.h | ModuleInitFn | `void (*ModuleInitFn)();` |
| 001-Core | TenEngine::core | — | 模块关闭回调类型 | TenEngine/core/module_load.h | ModuleShutdownFn | `void (*ModuleShutdownFn)();` |
| 001-Core | TenEngine::core | — | 注册模块初始化 | TenEngine/core/module_load.h | RegisterModuleInit | `void RegisterModuleInit(ModuleInitFn fn);` 注册顺序调用 |
| 001-Core | TenEngine::core | — | 注册模块关闭 | TenEngine/core/module_load.h | RegisterModuleShutdown | `void RegisterModuleShutdown(ModuleShutdownFn fn);` 逆序调用 |
| 001-Core | TenEngine::core | — | 执行模块初始化 | TenEngine/core/module_load.h | RunModuleInit | `void RunModuleInit();` 加载模块后调用 |
| 001-Core | TenEngine::core | — | 执行模块关闭 | TenEngine/core/module_load.h | RunModuleShutdown | `void RunModuleShutdown();` 卸载前调用 |

**平台与宏**：引擎支持 **Android、iOS** 等平台；**可以通过宏来判断执行哪一段代码**（如 TE_PLATFORM_ANDROID、TE_PLATFORM_IOS、TE_PLATFORM_WIN、TE_PLATFORM_LINUX、TE_PLATFORM_MACOS），编译时选择平台相关实现路径。平台检测与宏由 Platform 子模块或公共头提供。

**说明**：命名空间与头文件为下游 include 与 link 的唯一依据。新增条目采用 TenEngine::core 与 PascalCase 命名。本表由 plan 001-core-fullversion-002 契约更新同步（2026-01-30）。

---

## TODO（010-Shader 依赖）

010-Shader 模块**必须**使用本模块下列接口，001-Core 实现需确保可用：

| 用途 | 符号 | 头文件 | 说明 |
|------|------|--------|------|
| 加载 shader 源码 | FileRead | te/core/platform.h | LoadSource 读文件；返回 UTF-8 字节流可直接转 string |
| 错误与调试日志 | Log | te/core/log.h | 编译失败、缓存错误等使用 Log(LogLevel::Error/Warn, msg) |
| 内存分配 | Alloc, Free 或 GetDefaultAllocator | te/core/alloc.h | 工厂 Create/Destroy、handle 与 buffer 分配 |
| 路径规范化 | PathNormalize | te/core/platform.h | 缓存路径、源码路径规范化 |
| 枚举目录 | DirectoryEnumerate | te/core/platform.h | 热重载监听、批量加载 shader |
| 写缓存文件 | FileWrite | te/core/platform.h | SaveCache 写入字节 |
| 校验与断言 | CheckError, Assert | te/core/check.h, te/core/log.h | 内部校验 |

- [ ] 确认以上接口已实现且通过 010-Shader 集成测试
