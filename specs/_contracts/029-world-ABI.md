# 029-World 模块 ABI

- **契约**：[029-world-public-api.md](./029-world-public-api.md)（能力与类型描述）
- **本文件**：029-World 对外 ABI 显式表。

## ABI 表

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| （待补充） | 见本模块契约 | — | — | — | — | 由本模块契约与实现填入 |

## 与 004-Scene、013-Resource 的调用关系

- World 调用 004 **CreateSceneFromDesc**、**UnloadScene**；调用 013 **Load**、ResourceId/句柄解析。
- 具体符号与签名以 004-scene-public-api、013-resource-public-api 及本模块实现为准。
