# US-audio-001：播放音效（2D 音效、加载、播放/停止/循环）

- **标题**：游戏逻辑或 Editor 可**加载**音频资源（经 Resource 统一接口）、**播放** 2D 音效（播放/停止/循环）、调节音量；与 Resource 对接加载，与平台音频 API 通过抽象层对接。
- **编号**：US-audio-001

---

## 1. 角色/触发

- **角色**：游戏逻辑、Editor
- **触发**：需要播放**音效**（如 UI 点击、技能音效）；音效资源经 **requestLoadAsync(path, ResourceType::Audio, ...)** 加载；播放时指定音量、循环等参数。

---

## 2. 端到端流程

1. 调用方通过 **Resource** 的 **requestLoadAsync(path, ResourceType::Audio, callback)** 加载音频资源；回调中拿到 **IAudioResource** 或等价句柄。
2. 调用方将音频句柄交给 **Audio** 模块：**playSound(handle, volume, loop)**；Audio 模块通过平台音频 API（抽象层）播放；返回 **SoundInstanceId** 或句柄，用于 **stopSound**、**setVolume**。
3. 播放结束后可触发回调（可选）；资源生命周期由 Resource 与引用方约定。
4. 下游（游戏/Editor）仅通过 Audio 模块 API 与 Resource 句柄访问；不直接依赖平台音频 API。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 016-Audio | playSound、stopSound、setVolume、SoundInstanceId；与 Resource 音频句柄、平台音频 API 抽象层对接 |
| 013-Resource | 音频资源统一 requestLoadAsync、IAudioResource 或等价 |
| 001-Core | 可选：分配器、线程（Audio 内部使用） |

---

## 4. 每模块职责与 I/O

### 016-Audio

- **职责**：提供 **playSound**、**stopSound**、**setVolume**；接受 Resource 加载的音频句柄；与平台音频 API 通过抽象层对接；可选 3D 音效、混音组（后续故事）。
- **输入**：音频资源句柄（来自 Resource）、音量、循环、可选位置（3D）。
- **输出**：SoundInstanceId、播放状态；下游据此控制播放。

---

## 5. 派生 ABI（与契约对齐）

- **016-audio-ABI**：playSound、stopSound、setVolume、SoundInstanceId；与 Resource 句柄、平台抽象对接。详见 `specs/_contracts/016-audio-ABI.md`。

---

## 6. 验收要点

- 可经 Resource 加载音频并交给 Audio 模块播放；可停止、调节音量、循环播放。
- API 与平台音频解耦，通过抽象层对接。
