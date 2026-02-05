# Research: 001-Core 完整模块实现

**Feature**: 001-core-fullmodule | **Phase**: 0

## 1. Unity / Unreal Core 参考

**Decision**: 001-Core 对齐「基础层根模块」职责：内存、线程、平台、日志、数学、容器、模块加载；不包含反射与 ECS。

**Rationale**:
- **Unity**：Engine Core (Native) 提供内存分配、线程、文件 I/O、数学、容器、日志与断言；插件/模块通过动态库加载与初始化回调集成。
- **Unreal**：Core 模块提供 FMemory、FPlatformProcess、FThread、FFileManager、FMath、TArray/TMap、DLL 加载与模块生命周期；对外仅暴露声明在 Public 头中的类型与 API。

**Alternatives considered**: 将反射或序列化放入 Core — 已排除，规约明确由 002-Object 提供。

## 2. C++17 与分配器/线程/平台

**Decision**: 使用 C++17；分配器用 `std::aligned_alloc`（POSIX）或 `_aligned_malloc`（Windows）；线程用 `std::thread`、`std::mutex`、`std::condition_variable`；平台文件与路径用 `std::filesystem`；时间用 `std::chrono`。

**Rationale**: 契约与 ABI 不指定实现细节；C++17 满足跨平台与可维护性。

**Alternatives considered**: 自研内存池/线程池 — 保留为可选扩展，ABI 仅要求 DefaultAllocator 与 GetThreadPool() 的契约行为。

## 3. 命名空间与头文件路径

**Decision**: 对外命名空间 TenEngine::core；实现可用 te::core；头文件路径 te/core/（与 TenEngine/core/ 等价）。与 `001-core-ABI.md` 一致。

**Rationale**: 契约与 ABI 已规定；下游 include 与 link 以 ABI 为准。

## 4. 构建与测试

**Decision**: CMake 单库 te_core；单元测试独立可执行文件（test_alloc, test_engine, test_thread, test_platform, test_log, test_check, test_math, test_containers, test_module_load）；无外部依赖，构建不引入上游模块。

**Rationale**: 001-Core 为根模块；Constitution §VI 要求构建使用真实源码（本库即真实实现），无 stub。
