# Research: 009-render-core full

**Branch**: `009-render-core-full` | **Date**: 2026-01-29  
**Purpose**: Phase 0 研究结论；澄清与技术选型。

## 技术选型

| 决策项 | 决策 | 理由 |
|--------|------|------|
| 语言/版本 | C++17 | Constitution 要求 C++17 或更高；与上游 001-Core、008-RHI 一致；无 C++20 强依赖。 |
| 构建系统 | CMake | Constitution 要求单一、可重现构建；与 TenEngine 主工程一致。 |
| 依赖引入 | 001-Core、008-RHI 源码 | 通过工作区或主工程引入上游源码；仅使用契约声明的类型与 API。 |
| 对外暴露 | 仅契约类型与能力 | UniformLayout、VertexFormat、IndexFormat、TextureDesc、BufferDesc、PassResourceDecl、UniformBufferHandle；不暴露 RHI 内部类型。 |

## 已澄清项（来自 spec /speckit.clarify）

- **RingBuffer 耗尽**：Block（阻塞或要求调用方等待/重试直到槽位释放）；不静默覆盖 in-flight 数据。
- **CreateLayout 语义**：根据 UniformLayout 创建 Uniform 缓冲并返回 UniformBufferHandle（非“仅定义 layout”）。
- **不支持的格式/尺寸**：在 API 调用时拒绝（返回错误或无效句柄；不创建资源）；调用方处理回退。
- **同资源同 pass 读写**：由 PipelineCore（及 RHI）定义；RenderCore 不额外限制。
- **Uniform 与 Shader 不一致**：可选校验（CreateLayout/Bind 时可校验；不一致则返回错误）；校验行为须文档化。

## 上游契约依赖摘要

- **001-Core**：Alloc/Free、数学类型（Vector/Matrix 等）、容器（Array、String 等）、日志；用于描述符与缓冲内存、数学字段。
- **008-RHI**：IDevice、IBuffer、ITexture、资源创建与绑定；本模块产出的描述符（VertexFormat、TextureDesc、BufferDesc 等）作为 RHI 创建参数传入，不直接持有 RHI 资源。

## Alternatives Considered

- **C++20**：未采用；当前工作区与上游以 C++17 为主，避免过早依赖 concepts/modules。
- **RingBuffer 自动扩展（Grow）**：未采用；spec 澄清为 Block，避免不可控内存增长。
- **同 pass 读写由 RenderCore 禁止**：未采用；spec 澄清为由 PipelineCore 定义，保持边界清晰。
