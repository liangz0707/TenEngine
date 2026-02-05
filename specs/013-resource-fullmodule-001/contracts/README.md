# contracts/ 说明（013-resource-fullmodule-001）

本 feature 的 **ABI 全量内容** 写在 [../plan.md](../plan.md) 的「全量 ABI 内容（实现参考）」小节中，不在此目录下生成单独的 ABI 文件。

- **实现依据**：tasks 与 implement 阶段以 **plan.md 中的全量 ABI 表**（原始 + 新增 + 修改）为准。
- **写回契约**：plan 结束后可运行 **engine.contracts-writeback**，将 [../plan.md](../plan.md) 的「契约更新」小节（仅新增/修改部分）写回 `specs/_contracts/013-resource-ABI.md` 与 public-api。详见 `docs/agent-workflow-complete-guide.md`「2.0 写回契约」。

本目录保留为空说明目录，便于与「产出 REST/OpenAPI schema 至 contracts/」的项目结构一致；013 模块以 ABI 与 public-api 文件为唯一契约来源。
