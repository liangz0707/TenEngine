# Contract: 016-Audio Module Public API

## Applicable Modules

- **Implementer**: 016-Audio (L2; audio source, listener, mixing, spatial audio)
- **Corresponding Spec**: `docs/module-specs/016-audio.md`
- **Dependencies**: 001-Core, 013-Resource

## Consumers

- None (L2 consumer tier; game logic or applications use this API directly)

## Implementation Status

**PENDING IMPLEMENTATION** - No header files found in `Engine/TenEngine-016-audio/include/`.

## Capabilities List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| SourceHandle | Audio source handle; CreateSource, Play, Pause, Stop, SetLoop, SetResource | From creation until explicit release |
| ListenerHandle | Audio listener; SetPosition, SetOrientation, BindToEntity | Managed by Audio module |
| BusHandle | Mix bus handle; CreateBus, SetVolume, Mute, EffectSlot | From creation until explicit release |
| Spatial Audio Params | SetPosition, Attenuation, Occlusion (optional) | Bound to Source/Listener |

### Capabilities (Provider Guarantees)

| No. | Capability | Description |
|-----|------------|-------------|
| 1 | Audio Source | CreateSource, Play, Pause, Stop, SetLoop, SetResource |
| 2 | Audio Listener | ListenerHandle, SetPosition, SetOrientation, BindToEntity |
| 3 | Audio Mixing | CreateBus, SetVolume, Mute, EffectSlot |
| 4 | Spatial Audio | SetPosition, Attenuation, Occlusion (optional) |

## Version / ABI

- Follows Constitution: Public API versioning; breaking changes increment MAJOR.

## Constraints

- Must be used after Core and Resource initialization; audio resources loaded via 013-Resource.

## TODO List

(Tasks from `docs/asset/` resource management/loading/storage design.)

- [ ] **Resource Integration** (if needed): Define AudioAssetDesc, IAudioResource to interface with 013; obtain audio resources via 013 Load; only hold ResourceId/handle; one directory per resource (description + actual data or inline + optional .wav/.ogg).

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 016-Audio contract |
| 2026-02-05 | Unified directory; capabilities list as table |
| 2026-02-22 | Marked as pending implementation; no code headers found |
