# 016-Audio 模块 ABI

- **契约**：[016-audio-public-api.md](./016-audio-public-api.md)（能力与类型描述）
- **本文件**：016-Audio 对外 ABI 显式表。
- **参考**：Unity AudioSource/AudioListener、UE Sound；音源、监听、混音、空间音效。
- **命名**：成员方法采用 **PascalCase**；说明列给出**完整函数签名**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 音源（Source）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 016-Audio | te::audio | IAudioSource | 抽象接口 | 播放/暂停/停止 | te/audio/AudioSource.h | IAudioSource::Play, Pause, Stop | `void Play();` `void Pause();` `void Stop();` |
| 016-Audio | te::audio | IAudioSource | 抽象接口 | 循环 | te/audio/AudioSource.h | IAudioSource::SetLoop, IsLooping | `void SetLoop(bool loop);` `bool IsLooping() const;` |
| 016-Audio | te::audio | IAudioSource | 抽象接口 | 设置资源 | te/audio/AudioSource.h | IAudioSource::SetResource | `void SetResource(IAudioResource* resource);` 与 013-Resource 音频资源对接 |
| 016-Audio | te::audio | IAudioSource | 抽象接口 | 位置（空间音效） | te/audio/AudioSource.h | IAudioSource::SetPosition, GetPosition | `void SetPosition(Vector3 const& pos);` `Vector3 GetPosition() const;` |
| 016-Audio | te::audio | — | 自由函数/工厂 | 创建音源 | te/audio/AudioSource.h | CreateAudioSource | `IAudioSource* CreateAudioSource();` 失败返回 nullptr |

### 监听（Listener）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 016-Audio | te::audio | IAudioListener | 抽象接口 | 位置与朝向 | te/audio/AudioListener.h | IAudioListener::SetPosition, SetOrientation | `void SetPosition(Vector3 const& pos);` `void SetOrientation(Quaternion const& q);` |
| 016-Audio | te::audio | IAudioListener | 抽象接口 | 绑定实体 | te/audio/AudioListener.h | IAudioListener::BindToEntity | `void BindToEntity(IEntity* entity);` 与 Scene/Entity 变换同步（可选） |
| 016-Audio | te::audio | — | 自由函数/单例 | 获取主监听 | te/audio/AudioListener.h | GetMainListener | `IAudioListener* GetMainListener();` 由 Audio 管理；调用方不拥有指针 |

### 混音（Mixer / Bus）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 016-Audio | te::audio | IAudioBus | 抽象接口 | 音量与静音 | te/audio/AudioBus.h | IAudioBus::SetVolume, SetMute | `void SetVolume(float volume);` `void SetMute(bool mute);` |
| 016-Audio | te::audio | IAudioBus | 抽象接口 | 效果槽 | te/audio/AudioBus.h | IAudioBus::SetEffectSlot | `void SetEffectSlot(uint32_t slot, IAudioEffect* effect);` 可选 |
| 016-Audio | te::audio | — | 自由函数/工厂 | 创建总线 | te/audio/AudioBus.h | CreateAudioBus | `IAudioBus* CreateAudioBus(char const* name);` 失败返回 nullptr |

### 空间音效（可选）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 016-Audio | te::audio | IAudioSource | 抽象接口 | 衰减 | te/audio/AudioSource.h | IAudioSource::SetAttenuation | `void SetAttenuation(float minDist, float maxDist);` 3D 衰减（可选） |
| 016-Audio | te::audio | IAudioSource | 抽象接口 | 遮蔽 | te/audio/AudioSource.h | IAudioSource::SetOcclusion | `void SetOcclusion(float factor);` 遮蔽系数（可选） |

*来源：契约能力 Source、Listener、Mixer、Spatial；参考 Unity Audio、UE Sound。*
