# Research: 001-Core 完整功能集

**Branch**: `001-core-fullversion-001` | **Phase**: 0  
**Purpose**: 技术选型与实现模式，满足规约 7 个子模块且不引入第三方库；**内存与线程需封装与抽象以便未来切换 Android、iOS 等平台**。

## 1. 技术栈与依赖约束

**Decision**: C++17、CMake；仅使用 C++ 标准库与平台 API（Win32、POSIX、macOS dyld）；不引入 jemalloc、spdlog、EASTL 等（per spec 澄清）。

**Rationale**: Spec 明确不引入可选外部库；跨平台用 std + 平台抽象统一接口；规约 7 依赖「平台 API」与「技术/标准」已覆盖。

---

## 2. Memory：分配器、池、调试与泄漏（含平台抽象）

**Decision**: Allocator 抽象基类提供 Alloc/Free；DefaultAllocator 通过**平台抽象层**调用底层分配（当前：aligned_alloc/_aligned_malloc；未来 Android/iOS：可替换为平台 API 如 android_malloc、iOS 分配接口）；PoolAllocator 在默认分配器之上做块池；DebugAllocator/LeakTracker 可选包装。**公共 API 与底层实现解耦**，便于未来切换平台时仅替换实现文件、不改头文件与契约。

**Rationale**: 契约能力 1 要求分配器抽象；用户要求「内存需封装与抽象以便未来切换 Android、iOS 等平台」；通过 Allocator 接口 + 平台抽象层实现，当前 Win/Linux/macOS 用 std/platform API，未来扩展时新增平台实现即可。

---

## 3. Thread：线程、TLS、原子、同步、TaskQueue（含平台抽象）

**Decision**: **线程与同步通过平台抽象层封装**：公共 API（Thread 创建/销毁/join、Mutex、ConditionVariable、TLS、Atomic、TaskQueue）在头文件中稳定声明；当前实现使用 std::thread、std::mutex、std::condition_variable、std::atomic、thread_local；未来 Android/iOS 可替换为平台原生 API（如 pthread、Android JNI 线程、iOS GCD 等）的封装，**仅替换实现层、不改变契约与公共头文件**。TaskQueue 骨架为固定容量或无界队列 + worker 线程或提交即执行，语义见契约。

**Rationale**: 契约能力 2；用户要求「线程需封装与抽象以便未来切换 Android、iOS 等平台」；通过抽象接口 + 平台实现分支（当前 std，未来可加 android/ios 实现）满足可扩展性。

---

## 4. Platform：文件、目录、时间、环境、路径

**Decision**: std::filesystem 用于路径与目录枚举；文件读写用平台 API 封装（CreateFile/ReadFile、open/read、fopen）；时间与高精度计时用 std::chrono + platform（QueryPerformanceCounter、clock_gettime）；GetEnv 用 getenv/_wgetenv；平台宏 Win32/POSIX/dyld 条件编译。

**Rationale**: 规约要求与具体 OS 解耦、行为可重复验证；std::filesystem 与 std::chrono 加薄封装即可统一接口。

---

## 5. Log：级别、通道、Assert、CrashHandler

**Decision**: LogLevel 枚举；LogSink 抽象（可 stdout/stderr、文件、自定义）；Log() 用 std::mutex 保证线程安全、单条原子；Assert 为条件失败时可选调用 CrashHandler；CrashHandler 可注册回调（如写 dump、abort）。

**Rationale**: 契约能力 4；无 spdlog 故自实现；与 001-core-minimal 的 Log 语义兼容并扩展通道与 Assert/CrashHandler。

---

## 6. Math 与 Containers

**Decision**: Math：Vector2/3/4、Matrix3/4、Quaternion、AABB、Ray 为值类型；Lerp 等为自由函数；无 GPU 依赖。Containers：Array、Map、String、UniquePtr、SharedPtr 可基于 std 或自实现；支持传入 Allocator 以配合契约「可与自定义分配器配合」。

**Rationale**: 规约 5.2 与契约能力 5、6；不引入 EASTL 则用 std 或自实现；若自实现需保证 ABI 与契约一致。

---

## 7. ModuleLoad

**Decision**: LoadLibrary/UnloadLibrary/GetSymbol 封装 dlopen/dlsym、LoadLibrary/GetProcAddress、dlopen/dlsym（macOS）；ModuleInit/Shutdown 为加载/卸载时调用的回调注册表；依赖顺序由调用方按契约约定顺序 Load。

**Rationale**: 契约能力 7；与构建/插件系统配合；平台 API 已覆盖。

---

## 8. 性能与规模目标（plan 产出）

**Decision**: 可衡量目标由本 plan 产出并纳入验收；示例（具体数值由实现与主工程约定）：Alloc/Free 热点路径无多余锁；Log 写入线程安全且单条原子；TaskQueue 支持 N 并发任务；平台 API 延迟与契约「可重复验证」一致。在 quickstart 或 tasks 中列出验收指标。

**Rationale**: Spec 澄清「可衡量性能/规模目标由 plan 阶段产出」；Constitution 要求域内目标在 spec/plan 中说明。

---

## Summary

| 主题 | Decision | 状态 |
|------|----------|------|
| 依赖 | 仅 std + 平台 API，无第三方 | Resolved |
| Memory | Allocator + 平台抽象层（当前 Win/Linux/macOS，未来 Android/iOS 可替换实现） | Resolved |
| Thread | 平台抽象层（当前 std；未来 Android/iOS 可替换实现）+ TaskQueue 骨架 | Resolved |
| 平台可扩展性 | Memory/Thread 封装与抽象，便于未来切换 Android、iOS | Resolved |
| Platform | std::filesystem + 平台 API 封装 | Resolved |
| Log | 自实现 LogSink/Assert/CrashHandler | Resolved |
| Math/Containers | std 或自实现，可指定分配器 | Resolved |
| ModuleLoad | dlopen/LoadLibrary/dyld 封装 | Resolved |
| 性能目标 | 由 plan 产出并纳入验收 | Resolved |
