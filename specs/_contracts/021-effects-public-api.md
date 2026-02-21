# Contract: 021-Effects Module Public API

## Status: **TO BE IMPLEMENTED**

## Applicable Module

- **Implementer**: 021-Effects (L3; post-processing, particles/VFX, lighting post-processing)
- **Specification**: `docs/module-specs/021-effects.md`
- **Dependencies**: 019-PipelineCore, 009-RenderCore, 010-Shader, 028-Texture

## Consumers

- 020-Pipeline, 024-Editor

## Capability List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| PostProcessStack | Post-processing stack, Pass order, PipelineCore Fullscreen Pass, parameters (intensity, etc.) | Managed by Pipeline or caller |
| ParticleSystemHandle | Particle system handle; Emit, Update, ParticleBuffer, Atlas, RenderCore integration | Created until explicitly released |
| VFXHandle | Advanced VFX handle (optional); VFXGraph, CustomPass, Shader integration | Created until explicitly released |
| EffectParams | Post-processing/particle parameters; Bloom, TAA, ToneMapping, DOF, Intensity, etc. | Bound to Pass or system |

### Capabilities (Provider Guarantees)

| ID | Capability | Description |
|----|------------|-------------|
| 1 | Post-Processing | PostProcessStack, Pass order, PipelineCore Fullscreen Pass integration; Bloom, TAA, ToneMapping, DOF, etc. |
| 2 | Particles | ParticleSystemHandle, Emit, Update, ParticleBuffer, Atlas |
| 3 | VFX (Optional) | VFXHandle, VFXGraph, CustomPass, Shader integration |
| 4 | Lighting Post-Process | Parameters and Passes for Pipeline lighting Pass integration |

## Version / ABI

- Follows Constitution: Public API versioned; breaking changes increment MAJOR.
- **Current Status**: Implementation pending. No public headers available.

## Constraints

- Must be used after PipelineCore, RenderCore, Shader, and Texture initialization.
- GPU resources must be used through PipelineCore/RHI RT, Buffer, and PSO.

## Implementation Notes

The module directory `Engine/TenEngine-021-effects/include/` currently contains no header files.
The following interfaces are planned but not yet implemented:

- `te/effects/PostProcess.h` - IPostProcessStack interface
- `te/effects/ParticleSystem.h` - IParticleSystem interface
- `te/effects/VFX.h` - IVFXGraph interface (optional)
- `te/effects/EffectParams.h` - EffectParams structure

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 021-Effects contract |
| 2026-02-05 | Unified directory; capability list in table format |
| 2026-02-22 | Updated to reflect actual implementation status (to be implemented) |
