# 契约：016-Audio 模块对外 API

## 适用模块

- **实现方**：016-Audio（L2；音源、监听、混音、空间音效）
- **对应规格**：`docs/module-specs/016-audio.md`
- **依赖**：001-Core、013-Resource

## 消费者

- 无（L2 消费端；游戏逻辑或应用直接使用本 API）

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SourceHandle | 音源句柄；CreateSource、Play、Pause、Stop、SetLoop、SetResource | 创建后直至显式释放 |
| ListenerHandle | 听众；SetPosition、SetOrientation、BindToEntity | 由 Audio 管理 |
| BusHandle | 混音总线；CreateBus、SetVolume、Mute、EffectSlot | 创建后直至显式释放 |
| 空间音效参数 | SetPosition、Attenuation、Occlusion（可选） | 与 Source/Listener 绑定 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 音源 | CreateSource、Play、Pause、Stop、SetLoop、SetResource |
| 2 | 监听 | ListenerHandle、SetPosition、SetOrientation、BindToEntity |
| 3 | 混音 | CreateBus、SetVolume、Mute、EffectSlot |
| 4 | 空间音效 | SetPosition、Attenuation、Occlusion（可选） |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Resource 初始化之后使用；音频资源经 013 Load 获取。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 016-Audio 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
