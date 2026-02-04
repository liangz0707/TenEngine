# Data Model: 001-Core

**Feature**: 001-core-fullmodule | **Phase**: 1

## 实体与类型摘要（与 ABI 对齐）

以下仅列出契约/ABI 中声明的对外类型与句柄；实现内部结构不在此列出。

| 实体/类型 | 语义 | 生命周期/所有权 |
|-----------|------|------------------|
| Allocator | 抽象分配器接口；Alloc(size, alignment)、Free(ptr) | 由 GetDefaultAllocator() 返回的指针调用方不拥有 |
| DefaultAllocator | 默认堆分配器，实现 Allocator | 单例或进程静态，调用方不拥有 |
| InitParams | 初始化参数结构；可选 log_path, allocator_policy | 调用方栈或静态，Init 只读 |
| Thread | 线程对象；构造/析构/Join/Detach/Joinable | 调用方拥有，不可拷贝可移动 |
| TLS\<T\> | 线程局部存储 | 按线程生命周期 |
| Atomic\<T\> | 原子类型 | 调用方拥有 |
| Mutex | 互斥 | 调用方拥有，不可拷贝 |
| LockGuard | RAII 锁 | 栈上，作用域内 |
| ConditionVariable | 条件变量 | 调用方拥有，不可拷贝 |
| TaskQueue | 任务队列；Submit/RunOne/Shutdown | 调用方拥有 |
| IThreadPool | 线程池抽象；SubmitTask(callback, user_data) | GetThreadPool() 返回的指针调用方不拥有 |
| TaskCallback | 函数指针 `void (*)(void* user_data)` | 由调用方提供，线程池在工作线程调用 |
| DirEntry | 目录项名，即 std::string | 值类型 |
| LogLevel | 枚举 Debug/Info/Warn/Error | 值类型 |
| LogSink | 日志输出通道抽象；Write(level, message) | 调用方可实现并通过 LogSetSink 注入 |
| CrashHandlerFn | 函数指针 `void (*)(char const* message)` | 由 SetCrashHandler 设置 |
| Scalar | float | 值类型 |
| Vector2/3/4, Matrix3/4, Quaternion, AABB, Ray | 数学值类型 | 值类型，调用方管理 |
| Array\<T, Allocator\>, Map\<K,V,...\>, String, UniquePtr\<T\>, SharedPtr\<T\> | 容器与智能指针 | 与 std 等价语义，调用方管理 |
| ModuleHandle | void*（LoadLibrary 返回） | Load 后直至 Unload，调用方负责 Unload |
| ModuleInitFn, ModuleShutdownFn | 函数指针 `void (*)()` | 由 Register* 注册，RunModuleInit/RunModuleShutdown 调用 |

## 状态与顺序约束

- **Engine**：Init 前不可使用其他 Core API（或约定未 Init 时行为）；Shutdown 后不再使用 Core 分配或持有的句柄。
- **内存**：Alloc 返回的指针必须通过同一 Allocator/全局 Free 释放；Free(nullptr) 与 double-free 为 no-op。
- **模块加载**：RunModuleInit 按注册顺序调用；RunModuleShutdown 按注册逆序调用。
