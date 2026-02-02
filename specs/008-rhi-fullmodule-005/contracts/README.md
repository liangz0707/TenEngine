# Contracts: 008-rhi-fullmodule-005

本 feature **无**对现有 ABI 的新增或修改。

- **全量 ABI**：实现以 `specs/_contracts/008-rhi-ABI.md` 为准，实现该文件中列出的**全部**符号与能力。
- **契约**：对外 API 以 `specs/_contracts/008-rhi-public-api.md` 为准；上游仅依赖 `specs/_contracts/001-core-public-api.md`。

实现阶段基于**全量 ABI 内容**进行开发，禁止 stub 与长期 no-op；四后端（Vulkan、D3D12、Metal、D3D11）均为真实实现。
