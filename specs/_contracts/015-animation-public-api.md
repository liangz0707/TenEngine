# 契约：015-Animation 模块对外 API

## 适用模块

- **实现方**：**015-Animation**（动画剪辑与播放、骨骼、Timeline、可选状态机）
- **对应规格**：`docs/module-specs/015-animation.md`
- **依赖**：001-Core（001-core-public-api）、002-Object（002-object-public-api）、005-Entity（005-entity-public-api）

## 消费者（T0 下游）

- 020-Pipeline（蒙皮矩阵、与 Mesh 蒙皮数据对接）
- 012-Mesh（骨骼名称/索引对应、蒙皮数据）
- 024-Editor（动画预览、Timeline 编辑，若存在）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ClipHandle | 动画剪辑句柄；LoadClip、Sample、Interpolate、Loop、ClipRange | 创建后直至显式释放 |
| SkeletonHandle | 骨骼层级、BindPose、BoneIndexFromName、与 Mesh 骨骼索引对应 | 创建后直至显式释放 |
| PlaybackHandle | 播放控制；Play、Pause、SetTime、Blend、AttachToEntity | 创建后直至显式释放 |
| SkinMatrixBuffer | 蒙皮矩阵（Uniform 或缓冲）；与 Pipeline/Mesh 对接 | 每帧或按需更新 |

下游仅通过上述类型与句柄访问；与 Mesh 骨骼/蒙皮、Pipeline 蒙皮矩阵提交须协同。

## 能力列表（提供方保证）

1. **Clip**：LoadClip、Sample、Interpolate、Loop、ClipRange。
2. **Skeleton**：GetBoneHierarchy、BindPose、BoneIndexFromName；与 Mesh 对应。
3. **Playback**：Play、Pause、SetTime、Blend、AttachToEntity。
4. **StateMachine（可选）**：AddState、AddTransition、SetParameter、BlendTree。

## 调用顺序与约束

- 须在 Core、Object、Entity 初始化之后使用；骨骼名称/索引与 Mesh 约定一致。
- 蒙皮矩阵提交时机与 Pipeline DrawCall 须协调。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：015-Animation 对应本契约；与 docs/module-specs/015-animation.md 一致 |
