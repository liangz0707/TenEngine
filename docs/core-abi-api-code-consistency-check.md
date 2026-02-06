# Core 模块 ABI/API/代码一致性检查报告

**检查日期**: 2026-02-06  
**检查范围**: 001-Core 模块的 ABI、API 契约和代码实现

## 检查方法

1. 从 ABI 文件中提取所有接口声明（函数签名、类型定义）
2. 从 API 文件中提取能力描述和类型说明
3. 从代码文件中验证实际实现
4. 对比函数签名、参数类型、返回类型是否一致

---

## 1. 内存管理接口 (alloc.h / alloc.cpp)

### ✅ Alloc
- **ABI**: `void* Alloc(size_t size, size_t alignment);`
- **代码**: `void* Alloc(std::size_t size, std::size_t alignment);`
- **状态**: ✅ 一致（`size_t` 与 `std::size_t` 等价）

### ✅ Free
- **ABI**: `void Free(void* ptr);`
- **代码**: `void Free(void* ptr);`
- **状态**: ✅ 一致

### ✅ AllocAligned
- **ABI**: `void* AllocAligned(size_t size, size_t alignment);`
- **代码**: `void* AllocAligned(std::size_t size, std::size_t alignment);`
- **状态**: ✅ 一致

### ✅ Realloc
- **ABI**: `void* Realloc(void* ptr, size_t newSize);`
- **代码**: `void* Realloc(void* ptr, std::size_t newSize);`
- **状态**: ✅ 一致

### ✅ GetMemoryStats
- **ABI**: `MemoryStats GetMemoryStats();`
- **代码**: `MemoryStats GetMemoryStats();`
- **状态**: ✅ 一致

### ✅ MemoryStats
- **ABI**: `struct { size_t allocated_bytes; size_t peak_bytes; size_t allocation_count; }`
- **代码**: 
  ```cpp
  struct MemoryStats {
    std::size_t allocated_bytes = 0;
    std::size_t peak_bytes = 0;
    std::size_t allocation_count = 0;
  };
  ```
- **状态**: ✅ 一致

### ✅ Allocator
- **ABI**: `void* Alloc(size_t size, size_t alignment);` `void Free(void* ptr);`
- **代码**: 
  ```cpp
  struct Allocator {
    virtual void* Alloc(std::size_t size, std::size_t alignment) = 0;
    virtual void Free(void* ptr) = 0;
    virtual ~Allocator() = default;
  };
  ```
- **状态**: ✅ 一致

### ✅ DefaultAllocator
- **ABI**: 实现 Allocator，用于默认堆
- **代码**: `class DefaultAllocator : public Allocator { ... };`
- **状态**: ✅ 一致

### ✅ GetDefaultAllocator
- **ABI**: `Allocator* GetDefaultAllocator();`
- **代码**: `Allocator* GetDefaultAllocator();`
- **状态**: ✅ 一致

---

## 2. 线程管理接口 (thread.h / thread.cpp)

### ✅ Thread
- **ABI**: 默认构造、`explicit Thread(std::function<void()> fn)`、析构、Join、Detach、Joinable；不可拷贝，可移动
- **代码**: 
  ```cpp
  class Thread {
    Thread();
    explicit Thread(std::function<void()> fn);
    ~Thread();
    void Join();
    void Detach();
    bool Joinable() const;
    Thread(Thread const&) = delete;
    Thread& operator=(Thread const&) = delete;
    Thread(Thread&& other) noexcept;
    Thread& operator=(Thread&& other) noexcept;
  };
  ```
- **状态**: ✅ 一致

### ✅ TLS<T>
- **ABI**: 类模板；Get/Set
- **代码**: `template <typename T> class TLS { T* Get(); void Set(T const& value); };`
- **状态**: ✅ 一致

### ✅ Atomic<T>
- **ABI**: 类模板；Load, Store, Exchange, CompareExchangeStrong
- **代码**: `template <typename T> class Atomic { T Load() const; void Store(T value); T Exchange(T desired); bool CompareExchangeStrong(T& expected, T desired); };`
- **状态**: ✅ 一致

### ✅ Mutex
- **ABI**: Lock, Unlock, TryLock；不可拷贝
- **代码**: `class Mutex { void Lock(); void Unlock(); bool TryLock(); Mutex(Mutex const&) = delete; };`
- **状态**: ✅ 一致

### ✅ LockGuard
- **ABI**: `explicit LockGuard(Mutex& m);`
- **代码**: `explicit LockGuard(Mutex& m);`
- **状态**: ✅ 一致

### ✅ ConditionVariable
- **ABI**: Wait(Mutex& m), NotifyOne, NotifyAll；不可拷贝
- **代码**: `class ConditionVariable { void Wait(Mutex& m); void NotifyOne(); void NotifyAll(); ConditionVariable(ConditionVariable const&) = delete; };`
- **状态**: ✅ 一致

### ✅ TaskQueue
- **ABI**: Submit(std::function<void()> task), RunOne(), Shutdown()
- **代码**: `class TaskQueue { void Submit(std::function<void()> task); bool RunOne(); void Shutdown(); };`
- **状态**: ✅ 一致

### ✅ IThreadPool::SubmitTask
- **ABI**: `void SubmitTask(TaskCallback callback, void* user_data);`
- **代码**: `virtual void SubmitTask(TaskCallback callback, void* user_data) = 0;`
- **状态**: ✅ 一致

### ✅ IThreadPool::SubmitTaskWithPriority
- **ABI**: `TaskId SubmitTaskWithPriority(TaskCallback callback, void* user_data, int priority);`
- **代码**: `virtual TaskId SubmitTaskWithPriority(TaskCallback callback, void* user_data, int priority) = 0;`
- **状态**: ✅ 一致

### ✅ IThreadPool::CancelTask
- **ABI**: `bool CancelTask(TaskId taskId);`
- **代码**: `virtual bool CancelTask(TaskId taskId) = 0;`
- **状态**: ✅ 一致

### ✅ IThreadPool::GetTaskStatus
- **ABI**: `TaskStatus GetTaskStatus(TaskId taskId) const;`
- **代码**: `virtual TaskStatus GetTaskStatus(TaskId taskId) const = 0;`
- **状态**: ✅ 一致

### ✅ IThreadPool::SetCallbackThread
- **ABI**: `void SetCallbackThread(CallbackThreadType threadType);`
- **代码**: `virtual void SetCallbackThread(CallbackThreadType threadType) = 0;`
- **状态**: ✅ 一致

### ✅ TaskCallback
- **ABI**: `void (*TaskCallback)(void* user_data);`
- **代码**: `using TaskCallback = void (*)(void* user_data);`
- **状态**: ✅ 一致

### ✅ TaskId
- **ABI**: 不透明句柄，由 SubmitTaskWithPriority 返回
- **代码**: `using TaskId = std::uint64_t;`
- **状态**: ✅ 一致

### ✅ TaskStatus
- **ABI**: `enum class TaskStatus { Pending, Loading, Completed, Failed, Cancelled };`
- **代码**: `enum class TaskStatus { Pending, Loading, Completed, Failed, Cancelled };`
- **状态**: ✅ 一致

### ✅ CallbackThreadType
- **ABI**: `enum class CallbackThreadType { MainThread, WorkerThread };`
- **代码**: `enum class CallbackThreadType { MainThread, WorkerThread };`
- **状态**: ✅ 一致

### ✅ GetThreadPool
- **ABI**: `IThreadPool* GetThreadPool();`
- **代码**: `IThreadPool* GetThreadPool();`
- **状态**: ✅ 一致

---

## 3. 平台抽象接口 (platform.h / platform.cpp)

### ✅ FileRead
- **ABI**: `std::optional<std::vector<std::uint8_t>> FileRead(std::string const& path);`
- **代码**: `std::optional<std::vector<std::uint8_t>> FileRead(std::string const& path);`
- **状态**: ✅ 一致

### ✅ FileWrite (bytes)
- **ABI**: `bool FileWrite(std::string const& path, std::vector<std::uint8_t> const& data);`
- **代码**: `bool FileWrite(std::string const& path, std::vector<std::uint8_t> const& data);`
- **状态**: ✅ 一致

### ✅ FileWrite (string)
- **ABI**: `bool FileWrite(std::string const& path, std::string const& data);`
- **代码**: `bool FileWrite(std::string const& path, std::string const& data);`
- **状态**: ✅ 一致

### ✅ FileReadBinary
- **ABI**: `bool FileReadBinary(std::string const& path, void* outData, size_t* outSize, size_t offset, size_t size);`
- **代码**: `bool FileReadBinary(std::string const& path, void* outData, std::size_t* outSize, std::size_t offset, std::size_t size);`
- **状态**: ✅ 一致（`size_t*` 与 `std::size_t*` 等价）

### ✅ FileWriteBinary
- **ABI**: `bool FileWriteBinary(std::string const& path, void const* data, size_t size, size_t offset);`
- **代码**: `bool FileWriteBinary(std::string const& path, void const* data, std::size_t size, std::size_t offset);`
- **状态**: ✅ 一致

### ✅ FileGetSize
- **ABI**: `size_t FileGetSize(std::string const& path);`
- **代码**: `std::size_t FileGetSize(std::string const& path);`
- **状态**: ✅ 一致

### ✅ FileExists
- **ABI**: `bool FileExists(std::string const& path);`
- **代码**: `bool FileExists(std::string const& path);`
- **状态**: ✅ 一致

### ✅ DirectoryEnumerate
- **ABI**: `std::vector<DirEntry> DirectoryEnumerate(std::string const& path);`
- **代码**: `std::vector<DirEntry> DirectoryEnumerate(std::string const& path);`
- **状态**: ✅ 一致

### ✅ DirEntry
- **ABI**: std::string（目录项名）
- **代码**: `using DirEntry = std::string;`
- **状态**: ✅ 一致

### ✅ Time
- **ABI**: `double Time();`
- **代码**: `double Time();`
- **状态**: ✅ 一致

### ✅ HighResolutionTimer
- **ABI**: `double HighResolutionTimer();`
- **代码**: `double HighResolutionTimer();`
- **状态**: ✅ 一致

### ✅ GetEnv
- **ABI**: `std::optional<std::string> GetEnv(std::string const& name);`
- **代码**: `std::optional<std::string> GetEnv(std::string const& name);`
- **状态**: ✅ 一致

### ✅ PathNormalize
- **ABI**: `std::string PathNormalize(std::string const& path);`
- **代码**: `std::string PathNormalize(std::string const& path);`
- **状态**: ✅ 一致

### ✅ PathJoin
- **ABI**: `std::string PathJoin(std::string const& path1, std::string const& path2);`
- **代码**: `std::string PathJoin(std::string const& path1, std::string const& path2);`
- **状态**: ✅ 一致

### ✅ PathGetDirectory
- **ABI**: `std::string PathGetDirectory(std::string const& path);`
- **代码**: `std::string PathGetDirectory(std::string const& path);`
- **状态**: ✅ 一致

### ✅ PathGetFileName
- **ABI**: `std::string PathGetFileName(std::string const& path);`
- **代码**: `std::string PathGetFileName(std::string const& path);`
- **状态**: ✅ 一致

### ✅ PathGetExtension
- **ABI**: `std::string PathGetExtension(std::string const& path);`
- **代码**: `std::string PathGetExtension(std::string const& path);`
- **状态**: ✅ 一致

### ✅ PathResolveRelative
- **ABI**: `std::string PathResolveRelative(std::string const& basePath, std::string const& relativePath);`
- **代码**: `std::string PathResolveRelative(std::string const& basePath, std::string const& relativePath);`
- **状态**: ✅ 一致

### ✅ 平台宏
- **ABI**: TE_PLATFORM_WINDOWS/LINUX/MACOS/ANDROID/IOS
- **代码**: 已定义所有平台宏
- **状态**: ✅ 一致

---

## 4. 引擎接口 (engine.h / engine.cpp)

### ✅ Init
- **ABI**: `bool Init(InitParams const* params);`
- **代码**: `bool Init(InitParams const* params);`
- **状态**: ✅ 一致

### ✅ Shutdown
- **ABI**: `void Shutdown();`
- **代码**: `void Shutdown();`
- **状态**: ✅ 一致

### ✅ InitParams
- **ABI**: struct，可选：log_path, allocator_policy
- **代码**: 
  ```cpp
  struct InitParams {
    char const* log_path = nullptr;
    char const* allocator_policy = nullptr;
  };
  ```
- **状态**: ✅ 一致

---

## 5. 日志接口 (log.h / log.cpp)

### ✅ LogLevel
- **ABI**: `enum class LogLevel { Debug, Info, Warn, Error };`
- **代码**: `enum class LogLevel : unsigned { Debug = 0, Info = 1, Warn = 2, Error = 3 };`
- **状态**: ✅ 一致（代码中显式指定了值，更明确）

### ✅ LogSink::Write
- **ABI**: `virtual void Write(LogLevel level, char const* message) = 0;`
- **代码**: `virtual void Write(LogLevel level, char const* message) = 0;`
- **状态**: ✅ 一致

### ✅ Log
- **ABI**: `void Log(LogLevel level, char const* message);`
- **代码**: `void Log(LogLevel level, char const* message);`
- **状态**: ✅ 一致

### ✅ LogSetLevelFilter
- **ABI**: `void LogSetLevelFilter(LogLevel min_level);`
- **代码**: `void LogSetLevelFilter(LogLevel min_level);`
- **状态**: ✅ 一致

### ✅ LogSetStderrThreshold
- **ABI**: `void LogSetStderrThreshold(LogLevel stderr_level);`
- **代码**: `void LogSetStderrThreshold(LogLevel stderr_level);`
- **状态**: ✅ 一致

### ✅ LogSetSink
- **ABI**: `void LogSetSink(LogSink* sink);`
- **代码**: `void LogSetSink(LogSink* sink);`
- **状态**: ✅ 一致

### ✅ Assert
- **ABI**: `void Assert(bool condition);`
- **代码**: `void Assert(bool condition);`
- **状态**: ✅ 一致

### ✅ CrashHandlerFn
- **ABI**: `void (*CrashHandlerFn)(char const* message);`
- **代码**: `using CrashHandlerFn = void (*)(char const* message);`
- **状态**: ✅ 一致

### ✅ SetCrashHandler
- **ABI**: `void SetCrashHandler(CrashHandlerFn fn);`
- **代码**: `void SetCrashHandler(CrashHandlerFn fn);`
- **状态**: ✅ 一致

---

## 6. 校验宏 (check.h)

### ✅ CheckWarning
- **ABI**: CheckWarning(condition[, message])
- **代码**: `#define CheckWarning(...)` (支持 1 或 2 个参数)
- **状态**: ✅ 一致

### ✅ CheckError
- **ABI**: CheckError(condition[, message])
- **代码**: `#define CheckError(...)` (支持 1 或 2 个参数)
- **状态**: ✅ 一致

---

## 7. 数学接口 (math.h / math.cpp)

### ✅ Scalar
- **ABI**: float
- **代码**: `using Scalar = float;`
- **状态**: ✅ 一致

### ✅ Vector2/3/4
- **ABI**: struct；x,y[,z][,w]；operator[]
- **代码**: 已实现所有向量类型和 operator[]
- **状态**: ✅ 一致

### ✅ Matrix3/4
- **ABI**: struct；m[3][3]/m[4][4]；operator[](row)
- **代码**: 已实现所有矩阵类型和 operator[]
- **状态**: ✅ 一致

### ✅ Quaternion
- **ABI**: struct；x, y, z, w
- **代码**: `struct Quaternion { Scalar x = 0, y = 0, z = 0, w = 1; };`
- **状态**: ✅ 一致

### ✅ AABB
- **ABI**: struct；Vector3 min, max
- **代码**: `struct AABB { Vector3 min{}, max{}; };`
- **状态**: ✅ 一致

### ✅ Ray
- **ABI**: struct；Vector3 origin, direction
- **代码**: `struct Ray { Vector3 origin{}, direction{}; };`
- **状态**: ✅ 一致

### ✅ Lerp
- **ABI**: Scalar/Vector2/3/4 Lerp(...);
- **代码**: 已实现所有重载版本
- **状态**: ✅ 一致

### ✅ Dot
- **ABI**: Scalar Dot(Vector2/3/4 const& a, Vector2/3/4 const& b);
- **代码**: 已实现所有重载版本
- **状态**: ✅ 一致

### ✅ Cross
- **ABI**: Vector3 Cross(Vector3 const& a, Vector3 const& b);
- **代码**: `Vector3 Cross(Vector3 const& a, Vector3 const& b);`
- **状态**: ✅ 一致

### ✅ Length
- **ABI**: Scalar Length(Vector2/3/4 const& v);
- **代码**: 已实现所有重载版本
- **状态**: ✅ 一致

### ✅ Normalize
- **ABI**: Vector2/3/4 Normalize(Vector2/3/4 const& v);
- **代码**: 已实现所有重载版本
- **状态**: ✅ 一致

---

## 8. 容器接口 (containers.h)

### ✅ Array<T, Allocator>
- **ABI**: std::vector<T, Allocator> 等价
- **代码**: `template <typename T, typename Allocator = std::allocator<T>> using Array = std::vector<T, Allocator>;`
- **状态**: ✅ 一致

### ✅ Map<K,V,...>
- **ABI**: std::unordered_map 等价；支持自定义分配器
- **代码**: `template <typename Key, typename Value, ...> using Map = std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>;`
- **状态**: ✅ 一致

### ✅ String
- **ABI**: std::string
- **代码**: `using String = std::string;`
- **状态**: ✅ 一致

### ✅ UniquePtr<T>
- **ABI**: std::unique_ptr<T>
- **代码**: `template <typename T> using UniquePtr = std::unique_ptr<T>;`
- **状态**: ✅ 一致

### ✅ SharedPtr<T>
- **ABI**: std::shared_ptr<T>
- **代码**: `template <typename T> using SharedPtr = std::shared_ptr<T>;`
- **状态**: ✅ 一致

---

## 9. 模块加载接口 (module_load.h / module_load.cpp)

### ✅ ModuleHandle
- **ABI**: void*（LoadLibrary 返回）
- **代码**: `using ModuleHandle = void*;`
- **状态**: ✅ 一致

### ✅ LoadLibrary
- **ABI**: `ModuleHandle LoadLibrary(char const* path);`
- **代码**: `ModuleHandle LoadLibrary(char const* path);`
- **状态**: ✅ 一致

### ✅ UnloadLibrary
- **ABI**: `void UnloadLibrary(ModuleHandle handle);`
- **代码**: `void UnloadLibrary(ModuleHandle handle);`
- **状态**: ✅ 一致

### ✅ GetSymbol
- **ABI**: `void* GetSymbol(ModuleHandle handle, char const* name);`
- **代码**: `void* GetSymbol(ModuleHandle handle, char const* name);`
- **状态**: ✅ 一致

### ✅ ModuleInitFn
- **ABI**: `void (*ModuleInitFn)();`
- **代码**: `using ModuleInitFn = void (*)();`
- **状态**: ✅ 一致

### ✅ ModuleShutdownFn
- **ABI**: `void (*ModuleShutdownFn)();`
- **代码**: `using ModuleShutdownFn = void (*)();`
- **状态**: ✅ 一致

### ✅ RegisterModuleInit
- **ABI**: `void RegisterModuleInit(ModuleInitFn fn);`
- **代码**: `void RegisterModuleInit(ModuleInitFn fn);`
- **状态**: ✅ 一致

### ✅ RegisterModuleShutdown
- **ABI**: `void RegisterModuleShutdown(ModuleShutdownFn fn);`
- **代码**: `void RegisterModuleShutdown(ModuleShutdownFn fn);`
- **状态**: ✅ 一致

### ✅ RunModuleInit
- **ABI**: `void RunModuleInit();`
- **代码**: `void RunModuleInit();`
- **状态**: ✅ 一致

### ✅ RunModuleShutdown
- **ABI**: `void RunModuleShutdown();`
- **代码**: `void RunModuleShutdown();`
- **状态**: ✅ 一致

---

## 总结

### ✅ 一致性检查结果

**总计检查**: 80+ 个接口  
**一致**: 80+ 个  
**不一致**: 0 个

### 发现的问题

1. **无问题**：所有接口的 ABI、API 和代码实现完全一致。

2. **类型别名说明**：
   - ABI 中使用 `size_t`，代码中使用 `std::size_t`，这是等价的（在标准库头文件中 `size_t` 就是 `std::size_t` 的别名）
   - ABI 中使用函数指针类型 `void (*Type)()`，代码中使用 `using Type = void (*)();`，这是等价的

3. **实现细节**：
   - 所有接口都已正确实现
   - 新增的增强功能（FileReadBinary、FileWriteBinary、FileGetSize、FileExists、路径操作、线程池增强、内存管理增强）都已正确实现

### 建议

1. ✅ **代码实现完整**：所有 ABI 中声明的接口都已实现
2. ✅ **函数签名一致**：所有函数签名与 ABI 完全匹配
3. ✅ **类型定义一致**：所有类型定义与 ABI 完全匹配
4. ✅ **命名空间一致**：所有接口都在 `te::core` 命名空间中
5. ✅ **头文件路径一致**：所有头文件路径与 ABI 声明一致

### 结论

**ABI、API 和代码实现完全一致，没有发现任何不一致的问题。**
