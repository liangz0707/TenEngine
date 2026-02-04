# Research: 001-Core 最小切片（Alloc/Free、Log）

**Branch**: `001-core-minimal` | **Phase**: 0  
**Purpose**: 解决 Technical Context 中的技术选型与实现模式，供 Phase 1 设计与实现使用。

## 1. Alloc/Free 实现方式

**Decision**: 使用 C++17 标准库与平台对齐分配封装为统一 `Alloc(size, alignment)` / `Free(ptr)`；本切片不暴露 Allocator 接口、不实现池化或泄漏追踪。

**Rationale**:
- 契约要求「分配（指定大小与对齐）、释放、语义明确」；spec 澄清 Alloc(0/非法 alignment)→nullptr，Free(nullptr)/double-free 为 no-op。
- C11 `aligned_alloc`（Linux/macOS）与 Windows `_aligned_malloc` / `_aligned_free` 行为一致，可抽象为同一 API。
- 不引入 jemalloc/tcmalloc 等外部分配器，避免本切片依赖膨胀；后续切片可扩展 DefaultAllocator 或池化。

**Alternatives considered**:
- 直接暴露 Allocator 抽象：规约 Memory 有 Allocator 接口，但本切片仅「最小子集」，延后到后续切片。
- 使用 std::allocator：与契约「显式 Alloc/Free」语义一致，但标准 allocator 不保证对齐参数化，故采用 aligned_alloc/_aligned_malloc 封装。

---

## 2. Log 输出与线程安全

**Decision**: Log 输出仅 stdout/stderr；写入时用 `std::mutex` 保护，单条消息原子写入（一条消息内不交错）；级别与 stderr 阈值可配置（全局或线程局部由实现定）。

**Rationale**:
- Spec 澄清：本切片 Log 仅输出到 stdout/stderr；至少 Debug/Info/Warn/Error 四级；可配置「某级别及以上→stderr、其余→stdout」及过滤；写入要求线程安全、单条消息原子。
- C++17 下 `std::mutex` + `std::lock_guard` 即可；不引入 spdlog 等外部库，保持本切片零依赖（仅 std + stdio）。

**Alternatives considered**:
- 无锁队列 + 后台线程：复杂度高，本切片不采纳；后续若需性能可扩展。
- 调用方加锁：契约要求「Log 写入线程安全」，故由实现保证，不交给调用方。

---

## 3. 构建与测试

**Decision**: CMake 3.16+；单元测试可选用 Google Test 或手写 main；仅测试本切片暴露的 Alloc/Free、Log 行为及契约边界（Alloc(0)、Free(nullptr)、double-free、级别过滤）。

**Rationale**:
- Constitution 与用户要求技术栈 C++17、CMake。
- 测试需覆盖契约规定的成功路径与边界；不依赖其他引擎模块。

**Alternatives considered**:
- Meson：与规约「CMake/Meson」一致，本计划采用 CMake 以与 Constitution 示例一致。
- Catch2 / doctest：可选；若项目已有 GTest 则沿用，否则手写 main 亦可满足本切片范围。

---

## 4. 依赖与头文件布局

**Decision**: 本切片不依赖其他引擎模块；头文件仅使用 C++17 标准库与 C 标准库（如 `<cstdlib>`, `<cstddef>`, `<cstdio>`, `<mutex>`）。公共 API 放在 `include/te/core/`，命名空间可选（如 `te::core` 或全局），以契约与现有代码风格为准。

**Rationale**:
- 001-Core 为根模块；spec 明确无上游契约依赖。
- 保持 include 边界清晰，便于下游仅依赖 `alloc.h`、`log.h` 而不拖入未声明类型。

---

## Summary

| 主题 | Decision | 状态 |
|------|----------|------|
| Alloc/Free | aligned_alloc/_aligned_malloc 封装，Alloc(0/非法)→nullptr，Free(nullptr)/double-free no-op | Resolved |
| Log | stdout/stderr only，mutex 保护，单条原子，四级+可配置通道/过滤 | Resolved |
| 构建/测试 | CMake 3.16+，单元测试 GTest 或手写 | Resolved |
| 依赖 | 仅 std + stdio，无引擎/第三方依赖 | Resolved |
