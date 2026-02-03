# US-animation-001：播放动画剪辑（Clip 播放、播放/暂停/循环、与 Entity 对接）

- **标题**：游戏逻辑或 Editor 可**加载**动画剪辑资源（经 Resource）、**播放**动画剪辑（播放/暂停/停止/循环）、按时间驱动；动画可驱动 Entity 上骨骼或属性，与 Entity 对接。
- **编号**：US-animation-001

---

## 1. 角色/触发

- **角色**：游戏逻辑、Editor
- **触发**：需要**播放**某动画剪辑（如角色 Idle/Run）；剪辑经 Resource 加载；播放时按 deltaTime 推进，可循环、可暂停；动画结果驱动骨骼或属性（与 Entity/Transform 对接）。

---

## 2. 端到端流程

1. 调用方通过 **Resource** 加载动画剪辑资源；将剪辑句柄交给 **Animation** 模块，绑定到某 **Entity**（或骨骼根）。
2. 调用 **playClip(entity, clipHandle, loop)**；Animation 模块每帧 **update(deltaTime)** 推进剪辑时间，计算当前帧骨骼/属性采样结果。
3. 采样结果写回 Entity 的 **TransformComponent** 或 **骨骼矩阵**（与 012-Mesh 蒙皮对接）；渲染时使用更新后的骨骼矩阵。
4. 调用方可通过 **pause**、**resume**、**stop** 控制播放；可选 **onClipEnd** 回调。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 015-Animation | playClip、pause、resume、stop、update；ClipHandle、与 Entity 绑定、骨骼/属性采样 |
| 005-Entity | 动画驱动目标（Transform、骨骼槽位）；Entity 与动画实例绑定 |
| 013-Resource | 动画剪辑资源统一加载 |
| 012-Mesh | 蒙皮数据、骨骼索引与权重；Animation 输出骨骼矩阵供蒙皮 |

---

## 4. 每模块职责与 I/O

### 015-Animation

- **职责**：提供 **playClip**、**pause**、**resume**、**stop**、**update(deltaTime)**；ClipHandle 与 Entity 绑定；骨骼/属性采样结果写回 Entity 或骨骼矩阵缓冲；与 Mesh 蒙皮对接。
- **输入**：Entity、ClipHandle、loop、deltaTime；Resource 加载的剪辑数据。
- **输出**：播放状态、骨骼矩阵或属性更新；供渲染/蒙皮使用。

---

## 5. 派生 ABI（与契约对齐）

- **015-animation-ABI**：playClip、pause、resume、stop、update、ClipHandle、骨骼矩阵输出。详见 `specs/_contracts/015-animation-ABI.md`。

---

## 6. 验收要点

- 可加载并播放动画剪辑；可暂停/恢复/停止、循环；按 deltaTime 推进。
- 动画结果可驱动 Entity 变换或骨骼矩阵，供渲染蒙皮使用。
