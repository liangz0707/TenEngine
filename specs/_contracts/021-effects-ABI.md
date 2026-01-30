# 021-Effects 模块 ABI

- **契约**：[021-effects-public-api.md](./021-effects-public-api.md)（能力与类型描述）
- **本文件**：021-Effects 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 021-Effects | TenEngine::effects | — | 配置来源 | 后处理与抗锯齿按 RenderingConfig 生效 | TenEngine/pipeline/RenderingConfig.h | 见 Pipeline RenderingConfig | DOF、抗锯齿等由 Pipeline 的 `void IRenderPipeline::SetRenderingConfig(RenderingConfig const&);` 下发；Effects 按配置启用/禁用 |

*来源：用户故事 US-editor-001（编辑器内配置渲染设置并保存）；具体接口可由 Pipeline 转发或本模块提供 applyConfig，以实现为准。*
