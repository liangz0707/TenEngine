# 契约：021-Effects 模块对外 API

## 适用模块

- **实现方**：**021-Effects**（后处理、粒子与光照后处理）
- **对应规格**：`docs/module-specs/021-effects.md`
- **依赖**：019-PipelineCore（019-pipelinecore-public-api）、009-RenderCore（009-rendercore-public-api）、010-Shader（010-shader-public-api）

## 消费者（T0 下游）

- 020-Pipeline（将 Effects Pass 纳入管线、Fullscreen Pass、粒子绘制）
- 024-Editor（后处理/粒子预览，若存在）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| PostProcessStack | 后处理栈、Pass 顺序、与 PipelineCore Fullscreen Pass、参数（强度等） | 由 Pipeline 或调用方管理 |
| ParticleSystemHandle | 粒子系统句柄；Emit、Update、ParticleBuffer、Atlas、与 RenderCore 对接 | 创建后直至显式释放 |
| VFXHandle | 高级 VFX 句柄（可选）；VFXGraph、CustomPass、与 Shader 对接 | 创建后直至显式释放 |
| EffectParams | 后处理/粒子参数；Bloom、TAA、ToneMapping、DOF、Intensity 等 | 与 Pass 或系统绑定 |

下游仅通过上述类型与句柄访问；GPU 资源通过 PipelineCore/RHI 的 RT、Buffer、PSO 使用。

## 能力列表（提供方保证）

1. **PostProcess**：AddPass、Bloom、TAA、ToneMapping、DOF、Intensity；与 PipelineCore Fullscreen Pass 对接。
2. **Particles**：Emit、Update、ParticleBuffer、Atlas；与 Shader/RenderCore 粒子绘制对接。
3. **VFX（可选）**：VFXGraph、CustomPass；与 Shader 对接。
4. **Lighting（可选）**：VolumetricLight、Bloom；与 Pipeline 光照管线配合。

## 调用顺序与约束

- 须在 PipelineCore、RenderCore、Shader 初始化之后使用；Pass 与 PipelineCore 协议一致。
- Effects Pass 由 Pipeline 纳入管线并执行，资源声明与生命周期须符合 PipelineCore。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：021-Effects 对应本契约；与 docs/module-specs/021-effects.md 一致 |
