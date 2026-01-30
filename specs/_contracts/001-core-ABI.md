# 001-Core 模块 ABI

- **契约**：[001-core-public-api.md](./001-core-public-api.md)（能力与类型描述）
- **本文件**：001-Core 对外 ABI 显式表，供实现与下游统一命名空间、头文件与符号。
- **统一分配接口**：**所有内容分配**均通过 **IAllocator** 进行；**现成分配**（默认分配器、池、预分配等）也通过**同一套** **IAllocator** 暴露（如 GetDefaultAllocator() 返回 IAllocator*），调用方仅依赖 IAllocator。
- **命名**：成员方法采用**首字母大写的驼峰**（PascalCase）；所有方法在说明列给出**完整函数签名**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| 001-Core | TenEngine::core | — | 全局堆分配 | TenEngine/core/Alloc.h | Alloc | `void* Alloc(size_t size, size_t alignment);` 失败返回 nullptr |
| 001-Core | TenEngine::core | — | 全局堆释放 | TenEngine/core/Alloc.h | Free | `void Free(void* ptr);` ptr 可为 nullptr；double-free 为 no-op |
| 001-Core | TenEngine::core | IAllocator | 抽象分配器接口 | TenEngine/core/Allocator.h | IAllocator::Alloc, IAllocator::Free | `void* Alloc(size_t size, size_t alignment);` `void Free(void* ptr);` 虚接口，由 DefaultAllocator 等实现；Free(nullptr) 为 no-op |
| 001-Core | TenEngine::core | DefaultAllocator | 默认堆分配器 | TenEngine/core/Allocator.h | DefaultAllocator | 实现 IAllocator，用于默认堆 |
| 001-Core | TenEngine::core | — | 日志级别枚举 | TenEngine/core/Log.h | LogLevel | `enum class LogLevel { Debug, Info, Warn, Error };` |
| 001-Core | TenEngine::core | — | 写一条日志 | TenEngine/core/Log.h | Log | `void Log(LogLevel level, char const* message);` 线程安全 |
| 001-Core | TenEngine::core | LogSink | 日志输出通道抽象 | TenEngine/core/Log.h | LogSink::Write | `void Write(LogLevel level, char const* message);` 可重定向与过滤 |
| 001-Core | TenEngine::core | — | 断言 | TenEngine/core/Check.h | CheckError | 宏：条件为假时 abort；与 CheckWarning/CheckError 宏约定一致 |
| 001-Core | TenEngine::core | — | 宏 | 统一校验（Warning） | TenEngine/core/Check.h | CheckWarning(condition[, message]) | 条件为假时记录 Warning；可通过编译选项启用（TENENGINE_CHECK_WARNING / TENENGINE_CHECK_LEVEL） |
| 001-Core | TenEngine::core | — | 宏 | 统一校验（Error） | TenEngine/core/Check.h | CheckError(condition[, message]) | 条件为假时记录 Error 并可选中止/返回；可通过编译选项启用 |
| 001-Core | TenEngine::core | — | 自由函数 | 进程级初始化 | TenEngine/core/Engine.h | Init | `bool Init(InitParams const* params);` 失败返回 false，可重复调用时幂等 |
| 001-Core | TenEngine::core | — | 自由函数 | 进程级关闭 | TenEngine/core/Engine.h | Shutdown | `void Shutdown();` 在进程退出前调用，Init 之后仅调用一次 |
| 001-Core | TenEngine::core | — | struct | 初始化参数 | TenEngine/core/Engine.h | InitParams | 可选：log_path, allocator_policy；下游按需填充 |
| 001-Core | TenEngine::core | IThreadPool | 抽象接口 | 线程池 | TenEngine/core/ThreadPool.h | IThreadPool::SubmitTask | `void SubmitTask(TaskCallback callback, void* user_data);` 将任务提交到工作线程；线程安全 |
| 001-Core | TenEngine::core | — | 回调类型 | 任务回调 | TenEngine/core/ThreadPool.h | TaskCallback | `void (*TaskCallback)(void* user_data);` 在工作线程中执行 |
| 001-Core | TenEngine::core | — | 自由函数/单例 | 获取全局线程池 | TenEngine/core/ThreadPool.h | GetThreadPool | `IThreadPool* GetThreadPool();` 调用方不拥有指针；Resource 等用于后台加载 |
| 001-Core | TenEngine::core | IAllocator | 抽象接口 | **统一**分配接口 | TenEngine/core/Allocator.h | IAllocator::Alloc, IAllocator::Free | `void* Alloc(size_t size, size_t alignment);` `void Free(void* ptr);` 失败返回 nullptr；Free(nullptr) 为 no-op |
| 001-Core | TenEngine::core | — | 自由函数/单例 | **现成分配**：获取默认分配器 | TenEngine/core/Allocator.h | GetDefaultAllocator | `IAllocator* GetDefaultAllocator();` 返回默认堆分配器；调用方不拥有指针 |
| 001-Core | TenEngine::core | — | 自由函数 | 全局分配（委托默认分配器） | TenEngine/core/Allocator.h | Alloc, Free | `void* Alloc(size_t size, size_t alignment);` `void Free(void* ptr);` 与 GetDefaultAllocator()->Alloc/Free 语义一致 |
| 001-Core | TenEngine::core | — | 约定 | 现成分配均实现 IAllocator | TenEngine/core/Allocator.h | — | 池、线程局部分配器等现成分配均实现 IAllocator，并通过 GetXxxAllocator() 返回 IAllocator* |

**平台与宏**：引擎支持 **Android、iOS** 等平台；**可以通过宏来判断执行哪一段代码**（如 TE_PLATFORM_ANDROID、TE_PLATFORM_IOS、TE_PLATFORM_WIN、TE_PLATFORM_LINUX、TE_PLATFORM_MACOS），编译时选择平台相关实现路径。平台检测与宏由 Platform 子模块或公共头提供。

**说明**：上表为示例；完整 7 子模块（线程、平台、数学、容器、模块加载等）符号可后续按同一格式追加。命名空间与头文件为下游 include 与 link 的唯一依据。新增条目采用 TenEngine::core 与 PascalCase（首字母大写的驼峰）命名。
