# 契约：015-Animation 模块对外 API

## 适用模块

- **实现方**：015-Animation（L2；动画剪辑、骨骼动画、Timeline、状态机）
- **对应规格**：`docs/module-specs/015-animation.md`
- **依赖**：001-Core、002-Object、005-Entity

## 消费者

- 020-Pipeline（可选）、012-Mesh、024-Editor

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ClipHandle | 动画剪辑句柄；LoadClip、Sample、Interpolate、Loop、ClipRange | 创建后直至显式释放 |
| SkeletonHandle | 骨骼层级、BindPose、BoneIndexFromName、与 Mesh 骨骼索引对应 | 创建后直至显式释放 |
| 蒙皮矩阵等 | 与 Mesh 蒙皮数据、012/020 对接 | 按帧或按实例 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 剪辑 | LoadClip、Sample、Interpolate、Loop、ClipRange |
| 2 | 骨骼 | SkeletonHandle、BindPose、BoneIndexFromName；与 Mesh 骨骼索引约定一致 |
| 3 | 播放与状态 | 播放控制、Timeline、状态机（可选）；与 Entity 挂接 |
| 4 | 蒙皮 | 蒙皮矩阵输出；与 012-Mesh、020-Pipeline（可选）对接 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Object、Entity 初始化之后使用。蒙皮数据与 012-Mesh 骨骼名称/索引约定须一致。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 015-Animation 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
