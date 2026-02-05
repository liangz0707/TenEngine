# 契约：021-Effects 模块对外 API

## 适用模块

- **实现方**：021-Effects（L3；后处理、粒子/VFX、光照后处理）
- **对应规格**：`docs/module-specs/021-effects.md`
- **依赖**：019-PipelineCore、009-RenderCore、010-Shader、028-Texture

## 消费者

- 020-Pipeline、024-Editor

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| PostProcessStack | 后处理栈、Pass 顺序、与 PipelineCore Fullscreen Pass、参数（强度等） | 由 Pipeline 或调用方管理 |
| ParticleSystemHandle | 粒子系统句柄；Emit、Update、ParticleBuffer、Atlas、与 RenderCore 对接 | 创建后直至显式释放 |
| VFXHandle | 高级 VFX 句柄（可选）；VFXGraph、CustomPass、与 Shader 对接 | 创建后直至显式释放 |
| EffectParams | 后处理/粒子参数；Bloom、TAA、ToneMapping、DOF、Intensity 等 | 与 Pass 或系统绑定 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 后处理 | PostProcessStack、Pass 顺序、与 PipelineCore Fullscreen Pass 对接；Bloom、TAA、ToneMapping、DOF 等 |
| 2 | 粒子 | ParticleSystemHandle、Emit、Update、ParticleBuffer、Atlas |
| 3 | VFX（可选） | VFXHandle、VFXGraph、CustomPass、与 Shader 对接 |
| 4 | 光照后处理 | 与 Pipeline 光照 Pass 对接的参数与 Pass |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 PipelineCore、RenderCore、Shader、Texture 初始化之后使用。GPU 资源通过 PipelineCore/RHI 的 RT、Buffer、PSO 使用。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 021-Effects 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
