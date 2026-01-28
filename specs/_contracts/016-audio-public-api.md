# 契约：016-Audio 模块对外 API

## 适用模块

- **实现方**：**016-Audio**（音源、监听与混音，含空间音效）
- **对应规格**：`docs/module-specs/016-audio.md`
- **依赖**：001-Core（001-core-public-api）、013-Resource（013-resource-public-api）

## 消费者（T0 下游）

- 无（L2 消费端；游戏逻辑或 Editor 通过本 API 播放/混音）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SourceHandle | 音源句柄；CreateSource、Play、Pause、Stop、SetLoop、SetResource | 创建后直至显式释放 |
| ListenerHandle | 听众；SetPosition、SetOrientation、BindToEntity | 由 Audio 管理 |
| BusHandle | 混音总线；CreateBus、SetVolume、Mute、EffectSlot | 创建后直至显式释放 |
| 空间音效参数 | SetPosition、Attenuation、Occlusion（可选） | 与 Source/Listener 绑定 |

下游（游戏/Editor）通过上述类型与句柄访问；与 Resource 音频资源（WAV/OGG 等）、平台/第三方音频 API 通过抽象层对接。

## 能力列表（提供方保证）

1. **Source**：CreateSource、Play、Pause、Stop、SetLoop、SetResource；与 Resource 音频资源对接。
2. **Listener**：SetPosition、SetOrientation、BindToEntity；与 Scene/Entity 变换同步（可选）。
3. **Mixer**：CreateBus、SetVolume、Mute、EffectSlot。
4. **Spatial**：SetPosition、Attenuation、Occlusion；3D 定位与空间化（可选）。

## 调用顺序与约束

- 须在 Core、Resource 初始化之后使用；音频资源句柄与 Resource 约定一致。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：016-Audio 对应本契约；与 docs/module-specs/016-audio.md 一致 |
