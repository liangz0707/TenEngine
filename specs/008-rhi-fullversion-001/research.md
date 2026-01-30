# Research: 008-RHI 完整功能

**Feature**: 008-rhi-fullversion-001 | **Date**: 2026-01-29

## Decisions

| 主题 | Decision | Rationale |
|------|----------|-----------|
| CreateDevice 与窗口/上下文 | 不涉及；窗口/上下文仅用于 Present、XR 交换链等后续路径 | Spec 澄清 Q1；与 Device 切片一致。 |
| SelectBackend 与 CreateDevice | 两种均支持：先 SelectBackend 再 CreateDevice()，或 CreateDevice(Backend) 直接指定 | Spec 澄清 Q2。 |
| 后端不可用 | 仅明确失败；不自动回退；回退与重试由上层负责 | Spec 澄清 Q3。 |
| 线程安全 | 不要求默认并发安全；多线程行为由实现定义并文档化 | Spec 澄清 Q4。 |
| GetFeatures 结构 | 由 plan 定义最小集；spec 仅要求可读、可验证 | Spec 澄清 Q5。 |
| 错误报告 | CreateDevice / Create* 失败返回 nullptr；Submit 等失败以实现约定（错误码/返回值）报告 | 满足契约「明确失败」；与 Device 切片一致。 |
| 命令列表所有权 | IDevice 分配 ICommandList；Begin/End 录制；Submit 提交到 IQueue；池化/复用由实现约定 | 契约「单次录制周期内有效」；pipeline-to-rci 约定 RHI 接收命令缓冲并执行。 |
| 提交接口 | Submit(ICommandList, IQueue) 或等价；Pipeline 产出的抽象命令缓冲通过该接口交给 RHI | pipeline-to-rci：RHI 提供「提交命令缓冲」接口；格式见 pipeline-to-rci。 |
| 资源 / PSO / 同步 | Create* / Destroy* 在 IDevice；描述符（BufferDesc、TextureDesc 等）字段由实现定义；视图句柄与资源生命周期一致 | 契约能力 3–5；与 RenderCore/Shader 对接。 |

## Alternatives Considered

- **CreateDevice 接受窗口/上下文**：已澄清不涉及；Present/XR 路径另行处理。
- **自动后端回退**：已澄清仅失败，回退由上层负责。
- **强制线程安全**：已澄清不要求；由实现定义并文档化。
- **GetFeatures 在 spec 约定最小集**：已澄清由 plan 定义；spec 仅要求可读、可验证。

## References

- `docs/module-specs/008-rhi.md`
- `specs/_contracts/008-rhi-public-api.md`
- `specs/_contracts/001-core-public-api.md`
- `specs/_contracts/pipeline-to-rci.md`
- `specs/008-rhi-fullversion-001/spec.md`（含 Clarifications）
