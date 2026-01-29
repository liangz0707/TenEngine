# API Sketch: 001-core-minimal 切片

本文件为 `specs/_contracts/001-core-public-api.md` 中「API 雏形」的本切片占位；正式内容以 plan.md 末尾「契约更新」为准，写回契约后与本文件保持一致。

## 内存

- `void* Alloc(size_t size, size_t alignment);`
- `void Free(void* ptr);`

语义见 plan.md 与契约「能力列表」「调用顺序与约束」。

## 日志

- 枚举 `LogLevel`: `Debug`, `Info`, `Warn`, `Error`。
- `void Log(LogLevel level, char const* message);`
- 配置：级别过滤阈值、stderr 阈值（≥ 该级别→stderr）；具体 API 由实现与契约定稿。
