# Data Model: 004-Scene Full Module

**Branch**: 004-scene-fullmodule-001 | **Date**: 2026-02-04

规约与契约见 `docs/module-specs/004-scene.md`、`specs/_contracts/004-scene-public-api.md`。本 feature 实现完整模块内容；以下实体与关系描述对外类型与语义，不规定内部存储布局。

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 | 说明 |
|------|------|----------|------|
| **WorldRef** | 场景/世界容器引用 | 加载后直至卸载 | 不透明句柄；GetCurrentWorld、AddWorld 返回；SetActiveWorld 参数。 |
| **NodeId** | 场景图节点 ID | 与节点同生命周期 | 不透明句柄；CreateNode 返回；Parent/Children、GetPath、GetId、SetActive 等使用。 |
| **Transform** | 局部/世界变换 | 与节点/实体同步 | 与 Core.Math 或共用类型一致（position + rotation + scale）；LocalTransform/WorldTransform。 |
| **HierarchyIterator** | 层级遍历、按名/按类型查找 | **单次有效** | Traverse/FindByName/FindByType 返回；遍历结束后即失效。 |
| **Active** | 节点/子树是否参与更新/渲染 | 与节点绑定 | bool；SetActive/GetActive。 |
| **LevelHandle** | 关卡加载/卸载边界 | 按 Resource 契约 | 与 013-Resource 约定；LoadLevel 返回，UnloadLevel 参数。 |

## 实体与关系

- **World**：每个 WorldRef 对应一个世界容器；包含一棵场景图（根节点）、当前活动标记、可选 Level 引用与 Entity 根挂接。
- **Node**：每个 NodeId 对应一个场景图节点；Parent、Children、LocalTransform、WorldTransform、Active、可选 name/typeTag（FindByName/FindByType）。父子树状；成环由 API 拒绝。
- **HierarchyIterator**：单次有效；Traverse/Find 返回；Next/GetId/IsValid。

## 数据（ABI TODO 必须项）

- **LevelAssetDesc**：formatVersion、debugDescription、rootNodes、defaultWorldSettings（与 002 注册、反序列化配合）。
- **SceneNodeDesc**：name、localTransform、children、modelGuid、entityPrefabGuid、components、active。
- **TransformDesc**：position、rotation、scale。

## 状态与约束

- **脏标记**：LocalTransform 或层级变化后 SetDirty；UpdateTransforms 更新脏节点 WorldTransform 并清除脏标记。
- **SetActiveWorld**：下一帧或下一次 UpdateTransforms 后生效；GetCurrentWorld 返回当前活动 WorldRef。
- **SetParent** 成环或非法层级时返回 false，场景图不变。
