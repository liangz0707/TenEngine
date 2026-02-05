# Research: 013-Resource 最小切片（ResourceId / LoadSync / Release）

**Feature**: 013-resource-fullversion-001 | **Date**: 2026-01-29

## 1. LoadSync 结果类型（C++17 无异常）

**Decision**: 使用 `struct LoadResult { bool success; LoadHandle handle; int error_code; }` 表示同步加载结果；失败不抛异常。

**Rationale**: Spec 澄清已定「返回结果类型：成功时带 LoadHandle，失败时带错误码或原因，不抛异常」。C++17 下可选方案：`std::optional<LoadHandle>` + 单独错误码、`std::variant<LoadHandle, Error>`、或聚合结构体。聚合结构体与现有 001-Core/002-Object 契约风格一致（简单、无 STL 强依赖），且调用方必须显式检查 `success`，避免误用。

**Alternatives considered**: `std::optional` + 全局/线程局部 `LastError()`：增加全局状态，不利于多线程与可测试性。异常：spec 明确不抛异常。

## 2. ResourceId 的路径与 GUID 表示

**Decision**: `ResourceId` 为 `struct { enum Kind { Path, Guid }; Kind kind; char const* value; }`；value 为路径或 GUID 字符串，与 002-Object 契约中的 GUID/引用解析对接。

**Rationale**: 契约与 spec 要求「可寻址路径与 GUID 均支持」；双形式用 kind 区分，value 由 Object 契约（GUID、路径约定）解析。使用 `char const*` 与 Core 契约中的字符串/分配器约定对齐；若 Core 暴露 `String` 类型可后续替换为 `String const&` 等。

**Alternatives considered**: 单一字符串 + 启发式判断 Path/GUID：易歧义。两套 API（LoadByPath / LoadByGuid）：增加表面面积；本切片统一为 ResourceId 一种入参。

## 3. LoadHandle 类型与每次 LoadSync 新句柄

**Decision**: `LoadHandle` 为不透明句柄（如 `void*` 或 `struct { void* impl; }`）；对同一 ResourceId 多次 LoadSync 每次返回新的 LoadHandle（显式引用），由实现内部管理资源与引用。

**Rationale**: Spec 澄清已定「每次 LoadSync 返回新的 LoadHandle」。不透明句柄与契约「加载请求句柄」语义一致；实现可内部做引用计数或每句柄对应一次加载，对外仅暴露「每句柄须对应一次 Release、Release 幂等」。

**Alternatives considered**: 同一 ResourceId 返回同一句柄 + 内部引用计数：与 spec 澄清「每次返回新句柄」不符，已否决。

## 4. Release 幂等与错误码约定

**Decision**: Release 对同一句柄多次调用为幂等（第二次及以后无操作/成功）；不返回错误码，`void Release(LoadHandle)`。无效或已释放句柄传入时仍为幂等，不崩溃。

**Rationale**: Spec 澄清已定 Release 幂等。C++ 侧常见资源 API（如智能指针 reset、句柄 close）采用幂等；减少调用方负担，与「不抛异常」一致。

**Alternatives considered**: Release 返回 bool 表示是否真正释放：增加调用方检查负担；spec 已定幂等，无需区分。

## 5. 错误码（LoadResult.error_code）约定

**Decision**: 本切片仅约定「0 表示无错误；非 0 为失败」；具体错误码枚举或数值在实现或后续契约小节中定义（如 NOT_FOUND、IO_ERROR、INVALID_ID）。

**Rationale**: 契约「失败时带错误码或原因」已满足；具体枚举可随实现补齐，避免 plan 阶段过度承诺。

**Alternatives considered**: 本 plan 即定义完整 enum class LoadError：可做；留实现阶段可减少 plan 变更。

## 6. 与 001-Core、002-Object 契约对接

**Decision**: 仅使用两契约已声明的类型与 API。Core：Alloc/Free、文件/路径（若契约有）、字符串/容器（若契约有）。Object：GUID、引用解析、类型/序列化（本切片最小用到 GUID 或路径解析即可）。具体调用点在 data-model 与实现中落实。

**Rationale**: Constitution 与 spec FR-004 要求仅使用上游契约已声明接口；避免依赖未声明内部 API。

**Alternatives considered**: 无；合规为硬性约束。
