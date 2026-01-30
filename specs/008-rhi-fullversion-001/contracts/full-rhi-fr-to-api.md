# 008-RHI 完整功能：FR → API 映射

**Feature**: 008-rhi-fullversion-001 | **Date**: 2026-01-29

| FR | 能力 | API / 类型 |
|----|------|------------|
| FR-001 | Device、Queue、GetFeatures、SelectBackend | CreateDevice、DestroyDevice、GetQueue、GetFeatures、SelectBackend、GetSelectedBackend；IDevice、IQueue、Backend、QueueType、DeviceFeatures |
| FR-002 | CommandList、Submit | Begin、End、Draw、Dispatch、Copy、ResourceBarrier、Submit；ICommandList、CreateCommandList、DestroyCommandList |
| FR-003 | Resources、View | CreateBuffer、CreateTexture、CreateSampler、CreateView、Destroy*；IBuffer、ITexture、ISampler、*Desc |
| FR-004 | PSO | CreateGraphicsPSO、CreateComputePSO、SetShader、Cache、DestroyPSO；IPSO、*PSODesc |
| FR-005 | Fence、Semaphore、ResourceBarrier | CreateFence、CreateSemaphore、Wait、Signal、Destroy*；IFence、ISemaphore；ResourceBarrier 在 ICommandList |
| FR-006 | 错误与恢复 | 失败 nullptr / 明确报告；设备丢失可上报，支持回退或重建 |
| FR-007 | 线程安全 | 由实现定义并文档化；不要求默认并发安全 |
| FR-008 | 仅用 001-Core 契约 | Alloc/Free、Log 等；见 001-core-public-api |

完整签名与语义见 `plan.md` 末尾「契约更新」及 `specs/_contracts/008-rhi-public-api.md` API 雏形。
