# 021-Effects 模块 ABI

- **契约**：[021-effects-public-api.md](./021-effects-public-api.md)（能力与类型描述）
- **本文件**：021-Effects 对外 ABI 显式表。
- **参考**：Unity PostProcessing、粒子系统；UE Niagara、后处理；后处理栈、粒子、VFX。
- **命名**：成员方法采用 **PascalCase**；说明列给出**完整函数签名**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 后处理（PostProcess）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 021-Effects | te::effects | IPostProcessStack | 抽象接口 | 添加 Pass | te/effects/PostProcess.h | IPostProcessStack::AddPass | `void AddPass(PostProcessPassType type, EffectParams const* params);` Bloom、TAA、ToneMapping、DOF 等；与 PipelineCore Fullscreen Pass 对接 |
| 021-Effects | te::effects | IPostProcessStack | 抽象接口 | 设置强度 | te/effects/PostProcess.h | IPostProcessStack::SetIntensity | `void SetIntensity(char const* passName, float intensity);` |
| 021-Effects | te::effects | — | struct | 效果参数 | te/effects/EffectParams.h | EffectParams | Bloom、TAA、ToneMapping、DOF、Intensity 等；与 Pass 或系统绑定 |
| 021-Effects | te::effects | — | 枚举 | 后处理 Pass 类型 | te/effects/PostProcess.h | PostProcessPassType | Bloom、TAA、ToneMapping、DOF、Custom 等 |
| 021-Effects | te::effects | — | 配置来源 | 按 RenderingConfig 生效 | te/pipeline/RenderingConfig.h | 见 Pipeline RenderingConfig | DOF、抗锯齿等由 IRenderPipeline::SetRenderingConfig 下发；Effects 按配置启用/禁用 |

### 粒子（Particles）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 021-Effects | te::effects | IParticleSystem | 抽象接口 | 发射 | te/effects/ParticleSystem.h | IParticleSystem::Emit | `void Emit(uint32_t count, void const* params);` |
| 021-Effects | te::effects | IParticleSystem | 抽象接口 | 更新 | te/effects/ParticleSystem.h | IParticleSystem::Update | `void Update(float deltaTime);` |
| 021-Effects | te::effects | IParticleSystem | 抽象接口 | 粒子缓冲 | te/effects/ParticleSystem.h | IParticleSystem::GetParticleBuffer | `IBuffer* GetParticleBuffer() const;` 与 RenderCore/Shader 粒子绘制对接 |
| 021-Effects | te::effects | IParticleSystem | 抽象接口 | 图集 | te/effects/ParticleSystem.h | IParticleSystem::GetAtlas | `ITextureResource* GetAtlas() const;` 与 RenderCore 对接 |
| 021-Effects | te::effects | — | 自由函数/工厂 | 创建粒子系统 | te/effects/ParticleSystem.h | CreateParticleSystem | `IParticleSystem* CreateParticleSystem(ParticleSystemDesc const& desc);` 失败返回 nullptr |

### VFX（可选）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 021-Effects | te::effects | IVFXGraph | 抽象接口（可选） | VFX 图 | te/effects/VFX.h | IVFXGraph | 高级 VFX；VFXGraph、CustomPass、与 Shader 对接 |
| 021-Effects | te::effects | — | 自由函数（可选） | 自定义 Pass | te/effects/VFX.h | RegisterCustomPass | `void RegisterCustomPass(char const* name, CustomPassCallback cb);` 与 PipelineCore 对接 |

*来源：契约能力 PostProcess、Particles、VFX、Lighting；参考 Unity PostProcessing/粒子、UE Niagara。*
