# 001-Core 模块 ABI

- **契约**：[001-core-public-api.md](./001-core-public-api.md)（能力与类型描述）
- **本文件**：001-Core 对外 ABI 显式表，供实现与下游统一命名空间、头文件与符号。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| 001-Core | te::core | — | 全局堆分配 | te/core/alloc.h | Alloc | void* Alloc(size_t size, size_t alignment); 失败 nullptr |
| 001-Core | te::core | — | 全局堆释放 | te/core/alloc.h | Free | void Free(void* ptr); nullptr 与 double-free 为 no-op |
| 001-Core | te::core | Allocator | 抽象分配器接口 | te/core/alloc.h | Allocator::Alloc, Allocator::Free | 虚接口，由 DefaultAllocator 等实现 |
| 001-Core | te::core | DefaultAllocator | 默认堆分配器 | te/core/alloc.h | DefaultAllocator | 实现 Allocator，用于默认堆 |
| 001-Core | te::core | — | 日志级别枚举 | te/core/log.h | LogLevel | enum class LogLevel { Debug, Info, Warn, Error }; |
| 001-Core | te::core | — | 写一条日志 | te/core/log.h | Log | void Log(LogLevel level, char const* message); 线程安全 |
| 001-Core | te::core | LogSink | 日志输出通道抽象 | te/core/log.h | LogSink::Write | 可重定向与过滤 |
| 001-Core | te::core | — | 断言 | te/core/log.h | Assert | void Assert(bool condition); 失败调用 CrashHandler 后 abort |

**说明**：上表为示例；完整 7 子模块（线程、平台、数学、容器、模块加载等）符号可后续按同一格式追加。命名空间与头文件为下游 include 与 link 的唯一依据。
