# 021-Effects Module ABI

## Status: **TO BE IMPLEMENTED**

- **Contract**: [021-effects-public-api.md](./021-effects-public-api.md) (Capabilities and types description)
- **This Document**: 021-Effects external ABI explicit table.
- **Reference**: Unity PostProcessing, Particle System; UE Niagara, Post-Processing; post-processing stack, particles, VFX.
- **Naming**: Member methods use **PascalCase**; Description column provides **complete function signatures**.

## Implementation Status

The module directory `Engine/TenEngine-021-effects/include/` currently contains no header files.
All interfaces listed below are planned but not yet implemented.

## ABI Table (Planned)

Column Definition: **Module | Namespace | Class | Export | Interface | Header | Symbol | Description**

### Post-Processing (Planned)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 021-Effects | te::effects | IPostProcessStack | Abstract Interface | Add Pass | te/effects/PostProcess.h | IPostProcessStack::AddPass | `void AddPass(PostProcessPassType type, EffectParams const* params);` Bloom, TAA, ToneMapping, DOF, etc.; PipelineCore Fullscreen Pass integration |
| 021-Effects | te::effects | IPostProcessStack | Abstract Interface | Set Intensity | te/effects/PostProcess.h | IPostProcessStack::SetIntensity | `void SetIntensity(char const* passName, float intensity);` |
| 021-Effects | te::effects | — | struct | Effect Params | te/effects/EffectParams.h | EffectParams | Bloom, TAA, ToneMapping, DOF, Intensity, etc.; Bound to Pass or system |
| 021-Effects | te::effects | — | enum | Post-Process Pass Type | te/effects/PostProcess.h | PostProcessPassType | Bloom, TAA, ToneMapping, DOF, Custom, etc. |

### Particles (Planned)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 021-Effects | te::effects | IParticleSystem | Abstract Interface | Emit | te/effects/ParticleSystem.h | IParticleSystem::Emit | `void Emit(uint32_t count, void const* params);` |
| 021-Effects | te::effects | IParticleSystem | Abstract Interface | Update | te/effects/ParticleSystem.h | IParticleSystem::Update | `void Update(float deltaTime);` |
| 021-Effects | te::effects | IParticleSystem | Abstract Interface | Particle Buffer | te/effects/ParticleSystem.h | IParticleSystem::GetParticleBuffer | `IBuffer* GetParticleBuffer() const;` RenderCore/Shader particle rendering integration |
| 021-Effects | te::effects | IParticleSystem | Abstract Interface | Atlas | te/effects/ParticleSystem.h | IParticleSystem::GetAtlas | `ITextureResource* GetAtlas() const;` RenderCore integration |
| 021-Effects | te::effects | — | Free Function/Factory | Create Particle System | te/effects/ParticleSystem.h | CreateParticleSystem | `IParticleSystem* CreateParticleSystem(ParticleSystemDesc const& desc);` Returns nullptr on failure |

### VFX (Optional, Planned)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 021-Effects | te::effects | IVFXGraph | Abstract Interface (Optional) | VFX Graph | te/effects/VFX.h | IVFXGraph | Advanced VFX; VFXGraph, CustomPass, Shader integration |
| 021-Effects | te::effects | — | Free Function (Optional) | Custom Pass | te/effects/VFX.h | RegisterCustomPass | `void RegisterCustomPass(char const* name, CustomPassCallback cb);` PipelineCore integration |

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 021-Effects ABI |
| 2026-02-05 | Unified directory format |
| 2026-02-22 | Updated to reflect actual implementation status (to be implemented) |

*Source: Contract capabilities PostProcess, Particles, VFX, Lighting; Reference: Unity PostProcessing/Particles, UE Niagara.*
