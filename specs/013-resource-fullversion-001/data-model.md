# Data Model: 013-Resource 最小切片（ResourceId / LoadSync / Release）

**Feature**: 013-resource-fullversion-001 | **Date**: 2026-01-29

## 1. 实体与类型（本切片）

### ResourceId

| 属性 | 类型 | 说明 |
|------|------|------|
| kind | Path \| Guid | 区分可寻址路径与 GUID |
| value | char const*（或与 Core 契约对齐的字符串类型） | 路径字符串或 GUID 字符串 |

- **唯一性/身份**：由 value + kind 共同标识；与 Object 契约中的 GUID/路径解析一致。
- **生命周期**：由调用方管理；LoadSync 仅读取，不取得所有权。
- **校验**：无效（如 nullptr、空串）在 LoadSync 时通过 LoadResult.success==false 与 error_code 表示。

### LoadHandle

| 语义 | 说明 |
|------|------|
| 不透明句柄 | 实现定义为 void* 或 struct { void* impl; }；调用方不解读内容 |
| 生命周期 | 自 LoadSync 成功返回至 Release 调用（或进程结束）；每句柄对应一次显式引用 |
| 唯一性 | 每次 LoadSync 成功返回新的 LoadHandle（显式引用）；同一 ResourceId 多次 LoadSync 得到多个句柄 |

- **状态**：有效（未 Release）/ 已释放。已释放后再次 Release 为幂等。
- **不变量**：有效句柄至多被 Release 一次即失效；多次 Release 为幂等，不崩溃。

### LoadResult

| 属性 | 类型 | 说明 |
|------|------|------|
| success | bool | true = 加载成功，此时 handle 有效；false = 失败，此时 error_code 有效 |
| handle | LoadHandle | success 时有效；失败时为 nullptr 或未定义 |
| error_code | int | 失败时错误码；0 表示无错误 |

- **生命周期**：值类型，栈或调用方管理；不持有 LoadHandle 所有权，仅传递 handle 值。
- **校验**：调用方必须先检查 success，再使用 handle 或 error_code。

## 2. 状态与生命周期

```
ResourceId (调用方持有)
    │
    ▼ LoadSync
LoadResult ── success? ── yes ──► LoadHandle (调用方持有)
    │                                    │
    └── no ──► error_code                 ▼ Release (幂等，可多次)
                                                    │
                                                    ▼ 句柄失效，资源可回收
```

## 3. 与契约类型表的对应

| 本 data-model 实体 | 契约 013-resource-public-api 类型 |
|--------------------|-----------------------------------|
| ResourceId         | ResourceId（可寻址路径、GUID）    |
| LoadHandle         | LoadHandle（加载请求句柄）        |
| LoadResult         | 契约「结果类型」的具体化；未单独列类型名时作为 LoadSync 返回类型 |
| Release            | Unload 能力中的 Release           |

## 4. 不实现的类型（本切片外）

- AsyncResult、StreamingHandle、Metadata：不在此切片。
- LoadAsync、RequestStreaming、GC、UnloadPolicy 等：不在此切片。
