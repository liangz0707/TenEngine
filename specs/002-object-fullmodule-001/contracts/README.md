# Contracts (002-object-fullmodule-001)

本 feature 的 **ABI 与对外 API** 以以下文件为单一事实来源：

- **全量 ABI 表**：见 [../plan.md](../plan.md) 中「全量 ABI 内容（实现参考）」小节；tasks 与 implement 基于该表实现全部符号。
- **仓库契约**：`specs/_contracts/002-object-ABI.md`、`specs/_contracts/002-object-public-api.md`。

本目录不单独存放 ABI 副本；plan 中已包含实现所需的完整 ABI 条目。若 plan 产出「契约更新」条目，写回时仅将**新增/修改**部分合并到 `specs/_contracts/002-object-ABI.md`。

**上游依赖**：实现仅使用 `specs/_contracts/001-core-public-api.md` 中声明的类型与 API。
