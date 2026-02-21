# 016-Audio Module ABI

- **Contract**: [016-audio-public-api.md](./016-audio-public-api.md) (capabilities and type descriptions)
- **This File**: 016-Audio external ABI explicit table.
- **Reference**: Unity AudioSource/AudioListener, UE Sound; audio source, listener, mixing, spatial audio.
- **Naming**: Member methods use **PascalCase**; description column provides **complete function signatures**.

## Implementation Status

**PENDING IMPLEMENTATION** - No header files found in `Engine/TenEngine-016-audio/include/`.

## ABI Table

Column definitions: **Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description**

### Audio Source

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 016-Audio | te::audio | IAudioSource | Abstract Interface | Play/Pause/Stop | te/audio/AudioSource.h | IAudioSource::Play, Pause, Stop | `void Play();` `void Pause();` `void Stop();` |
| 016-Audio | te::audio | IAudioSource | Abstract Interface | Loop | te/audio/AudioSource.h | IAudioSource::SetLoop, IsLooping | `void SetLoop(bool loop);` `bool IsLooping() const;` |
| 016-Audio | te::audio | IAudioSource | Abstract Interface | Set Resource | te/audio/AudioSource.h | IAudioSource::SetResource | `void SetResource(IAudioResource* resource);` Interface with 013-Resource audio resources |
| 016-Audio | te::audio | IAudioSource | Abstract Interface | Position (Spatial Audio) | te/audio/AudioSource.h | IAudioSource::SetPosition, GetPosition | `void SetPosition(Vector3 const& pos);` `Vector3 GetPosition() const;` |
| 016-Audio | te::audio | — | Free Function/Factory | Create Source | te/audio/AudioSource.h | CreateAudioSource | `IAudioSource* CreateAudioSource();` Returns nullptr on failure |

### Audio Listener

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 016-Audio | te::audio | IAudioListener | Abstract Interface | Position and Orientation | te/audio/AudioListener.h | IAudioListener::SetPosition, SetOrientation | `void SetPosition(Vector3 const& pos);` `void SetOrientation(Quaternion const& q);` |
| 016-Audio | te::audio | IAudioListener | Abstract Interface | Bind Entity | te/audio/AudioListener.h | IAudioListener::BindToEntity | `void BindToEntity(IEntity* entity);` Sync with Scene/Entity transform (optional) |
| 016-Audio | te::audio | — | Free Function/Singleton | Get Main Listener | te/audio/AudioListener.h | GetMainListener | `IAudioListener* GetMainListener();` Managed by Audio; caller does not own pointer |

### Mixer / Bus

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 016-Audio | te::audio | IAudioBus | Abstract Interface | Volume and Mute | te/audio/AudioBus.h | IAudioBus::SetVolume, SetMute | `void SetVolume(float volume);` `void SetMute(bool mute);` |
| 016-Audio | te::audio | IAudioBus | Abstract Interface | Effect Slot | te/audio/AudioBus.h | IAudioBus::SetEffectSlot | `void SetEffectSlot(uint32_t slot, IAudioEffect* effect);` Optional |
| 016-Audio | te::audio | — | Free Function/Factory | Create Bus | te/audio/AudioBus.h | CreateAudioBus | `IAudioBus* CreateAudioBus(char const* name);` Returns nullptr on failure |

### Spatial Audio (Optional)

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 016-Audio | te::audio | IAudioSource | Abstract Interface | Attenuation | te/audio/AudioSource.h | IAudioSource::SetAttenuation | `void SetAttenuation(float minDist, float maxDist);` 3D attenuation (optional) |
| 016-Audio | te::audio | IAudioSource | Abstract Interface | Occlusion | te/audio/AudioSource.h | IAudioSource::SetOcclusion | `void SetOcclusion(float factor);` Occlusion factor (optional) |

*Source: Contract capabilities Source, Listener, Mixer, Spatial; Reference Unity Audio, UE Sound.*

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 016-Audio ABI |
| 2026-02-05 | Unified directory format |
| 2026-02-22 | Marked as pending implementation; no code headers found |
