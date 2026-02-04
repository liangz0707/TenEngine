# 015-Animation 模块 ABI

- **契约**：[015-animation-public-api.md](./015-animation-public-api.md)（能力与类型描述）
- **本文件**：015-Animation 对外 ABI 显式表。
- **参考**：Unity Animator/AnimationClip、UE AnimSequence/Skeleton；剪辑、骨骼、播放、蒙皮矩阵。
- **命名**：成员方法采用 **PascalCase**；说明列给出**完整函数签名**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 动画剪辑（Clip）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 015-Animation | te::animation | IAnimationClip | 抽象接口 | 加载剪辑 | te/animation/AnimationClip.h | IAnimationClip::Load | `bool Load(char const* path);` 失败返回 false |
| 015-Animation | te::animation | IAnimationClip | 抽象接口 | 采样 | te/animation/AnimationClip.h | IAnimationClip::Sample | `void Sample(float time, float* outTransforms, uint32_t boneCount) const;` 按时间取骨骼变换 |
| 015-Animation | te::animation | IAnimationClip | 抽象接口 | 时长与循环 | te/animation/AnimationClip.h | IAnimationClip::GetDuration, IsLooping | `float GetDuration() const;` `bool IsLooping() const;` |
| 015-Animation | te::animation | IAnimationClip | 抽象接口 | 剪辑范围 | te/animation/AnimationClip.h | IAnimationClip::GetClipRange | `void GetClipRange(float* start, float* end) const;` |
| 015-Animation | te::animation | — | 自由函数/工厂 | 创建剪辑 | te/animation/AnimationClip.h | CreateAnimationClip | `IAnimationClip* CreateAnimationClip();` 失败返回 nullptr；Load 后使用 |

### 骨骼（Skeleton）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 015-Animation | te::animation | ISkeleton | 抽象接口 | 骨骼层级 | te/animation/Skeleton.h | ISkeleton::GetBoneHierarchy | `void GetBoneHierarchy(uint32_t boneIndex, uint32_t* parentIndex) const;` 或等价迭代；与 Mesh 骨骼对应 |
| 015-Animation | te::animation | ISkeleton | 抽象接口 | 绑定姿态 | te/animation/Skeleton.h | ISkeleton::GetBindPose | `void GetBindPose(float* outTransforms, uint32_t boneCount) const;` 与 Mesh BindPose 一致 |
| 015-Animation | te::animation | ISkeleton | 抽象接口 | 按名称查骨骼索引 | te/animation/Skeleton.h | ISkeleton::GetBoneIndexFromName | `int32_t GetBoneIndexFromName(char const* name) const;` 未找到返回 -1；与 Mesh 骨骼名称约定一致 |
| 015-Animation | te::animation | ISkeleton | 抽象接口 | 骨骼数量 | te/animation/Skeleton.h | ISkeleton::GetBoneCount | `uint32_t GetBoneCount() const;` |
| 015-Animation | te::animation | — | 自由函数/工厂 | 创建骨骼 | te/animation/Skeleton.h | CreateSkeleton | `ISkeleton* CreateSkeleton(char const* pathOrData);` 失败返回 nullptr |

### 播放（Playback）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 015-Animation | te::animation | IAnimator | 抽象接口 | 播放/暂停 | te/animation/Animator.h | IAnimator::Play, Pause | `void Play();` `void Pause();` |
| 015-Animation | te::animation | IAnimator | 抽象接口 | 设置时间 | te/animation/Animator.h | IAnimator::SetTime | `void SetTime(float time);` |
| 015-Animation | te::animation | IAnimator | 抽象接口 | 混合 | te/animation/Animator.h | IAnimator::Blend | `void Blend(IAnimationClip* clip, float weight, float duration);` 与当前剪辑混合 |
| 015-Animation | te::animation | IAnimator | 抽象接口 | 绑定到实体 | te/animation/Animator.h | IAnimator::AttachToEntity | `void AttachToEntity(IEntity* entity);` 与 Entity 变换同步（可选） |
| 015-Animation | te::animation | IAnimator | 抽象接口 | 设置剪辑 | te/animation/Animator.h | IAnimator::SetClip | `void SetClip(IAnimationClip* clip);` |
| 015-Animation | te::animation | — | 自由函数/工厂 | 创建动画控制器 | te/animation/Animator.h | CreateAnimator | `IAnimator* CreateAnimator(ISkeleton* skeleton);` 失败返回 nullptr |

### 蒙皮矩阵（SkinMatrixBuffer）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 015-Animation | te::animation | IAnimator | 抽象接口 | 获取蒙皮矩阵 | te/animation/Animator.h | IAnimator::GetSkinMatrixBuffer | `float const* GetSkinMatrixBuffer(uint32_t* outCount) const;` 或 IBuffer*；与 Pipeline/Mesh 对接；每帧或按需更新 |
| 015-Animation | te::animation | — | 自由函数 | 更新蒙皮矩阵 | te/animation/SkinMatrix.h | UpdateSkinMatrices | `void UpdateSkinMatrices(IAnimator* animator, float* outBuffer, uint32_t boneCount);` 可选；由 Animator 内部或调用方调用 |

*来源：契约能力 Clip、Skeleton、Playback、SkinMatrixBuffer；参考 Unity Animator、UE AnimSequence/Skeleton。*
