# 契约：020-Pipeline → 008-RHI（命令缓冲与提交）

## 适用边界

- **产出方**：**020-Pipeline**（T0 渲染管线实现）
- **消费方**：**008-RHI**（T0 图形 API 抽象层，RCI/RHI）

本契约描述从「场景收集 → DrawCall → 命令缓冲」到「RHI 执行」的接口边界，确保 Pipeline 与 RHI 由不同 Agent 实现时仍能正确对接。

## 数据流

```
Scene/Entity → 场景收集 → 剔除 → DrawCall 批次 → PipelineCore Pass 图 → 命令缓冲 → RHI 提交并执行 → Present/XR
```

## 抽象命令缓冲（Pipeline 产出 / RHI 消费）

- **含义**：与具体图形 API 解耦的一帧渲染命令序列；RHI 负责将其转换为底层 Vulkan/D3D12/Metal 调用。
- **内容**：至少包含与「绘制/计算调用」对应的信息（几何、材质/着色器、PSO、资源绑定、状态），以及资源屏障、同步点等；具体结构由 Pipeline 与 RHI 实现共同约定，并符合 `008-rhi-public-api.md` 的提交接口。
- **生命周期**：由 Pipeline 在每帧相应阶段创建/填充，提交给 RHI 后由 RHI 在本帧内消费；RHI 不假定命令缓冲在帧外有效。

## 资源创建与状态

- Pipeline 负责「何时创建 DrawCall 相关资源、何时分配/写入命令缓冲」；RHI 仅约定：**接收到的命令缓冲在提交时已就绪**，且格式符合 `008-rhi-public-api.md` 与本文约定。
- 资源状态机（未创建→就绪→使用中→可回收）由 Pipeline 与 PipelineCore 管理；跨边界时资源须处于 RHI 可安全使用的状态（屏障与过渡由 Pipeline 在命令缓冲中声明或由 RHI 推断）。

## 提交约定

- Pipeline 在每帧的适当时机（场景收集与 DrawCall/Pass 构建完成后）将**本帧的抽象命令缓冲**交给 RHI。
- RHI 提供「提交命令缓冲」的接口（名称与签名以实现为准），保证：
  - 同一帧内命令按提交顺序或约定顺序执行；
  - 命令缓冲引用到的资源在执行时仍有效且状态正确；
  - 支持 Present 与 XR 交换链提交（若由同一 RHI 管理）。

## 错误与降级

- 若 RHI 暂时不可用或拒绝接收（如设备丢失），Pipeline 应能安全降级或明确失败，并保持自身资源状态一致、可恢复。
- 若命令缓冲格式或引用不合法，RHI 可返回错误；Pipeline 不得依赖未定义行为。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| （初始） | 从 006-render-pipeline-system 与 002-rendering-rci-interface spec 提炼，定义 Pipeline↔RCI 边界 |
| T0 更新 | 产出方改为 020-Pipeline，消费方改为 008-RHI；数据流与 PipelineCore（019）一致 |
