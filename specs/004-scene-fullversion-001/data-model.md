# Data Model: 004-Scene 完整功能集

**Branch**: 004-scene-fullversion-001 | **Date**: 2026-01-29

规约与契约见 `docs/module-specs/004-scene.md`、`specs/_contracts/004-scene-public-api.md`。本切片实现完整功能集，以下实体与关系仅描述对外暴露的类型与语义，不规定内部存储布局。

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 | 说明 |
|------|------|----------|------|
| **WorldRef** | 场景/世界容器引用 | 加载后直至卸载 | 不透明句柄；GetCurrentWorld、AddWorld 返回；SetActiveWorld 参数。 |
| **NodeId** | 场景图节点 ID | 与节点同生命周期 | 不透明句柄；创建节点返回；Parent/Children、GetPath、GetId、SetActive 等使用。 |
| **Transform** | 局部/世界变换 | 与节点/实体同步 | 与 Core.Math 或共用类型一致（如 position + rotation + scale 或 Matrix4）；LocalTransform/WorldTransform 读写。 |
| **HierarchyIterator** | 层级遍历、按名/按类型查找 | **单次有效** | Traverse/FindByName/FindByType 返回；遍历结束后即失效，不可复用。 |
| **Active** | 节点/子树是否参与更新/渲染 | 与节点绑定 | 布尔或等价状态；SetActive 设置，下游可查询。 |
| **LevelHandle** | 关卡加载/卸载边界 | 按 Resource 契约 | 与 013-Resource 约定；LoadLevel 返回，UnloadLevel 参数。 |

## 实体与关系

- **World**：每个 WorldRef 对应一个世界容器；包含一棵场景图（根节点）、当前活动标记（多 World 时）、可选 Entity 根挂接与 Level 引用。World 之间相互独立。
- **Node**：每个 NodeId 对应一个场景图节点；有 Parent（可选）、Children 集合、LocalTransform、WorldTransform（由 UpdateTransforms 更新）、Active 状态、可选名称与类型信息（供 FindByName/FindByType）。父子关系为树状，成环或非法层级由 API 拒绝并返回错误。
- **HierarchyIterator**：单次有效；在单次 Traverse 或 Find 过程中有效，用于按层级顺序或查询结果访问 NodeId；迭代中不得修改该子树结构（未定义）。

## 状态与约束

- **脏标记**：节点 LocalTransform 或层级变化后须 SetDirty；UpdateTransforms 按依赖顺序更新所有脏节点的 WorldTransform，并清除脏标记。变换更新顺序与脏标记须一致。
- **当前活动 World**：多 World 时，SetActiveWorld 指定下一帧或下一次 UpdateTransforms 后的“当前 World”；GetCurrentWorld 返回当前活动 WorldRef；变换更新与层级查询针对当前活动 World。
- **父子成环/非法层级**：SetParent 等若导致成环或非法层级，API 返回错误，不修改场景图。

## 与上游类型对齐

- **Transform**：position/rotation/scale 或 4×4 矩阵、四元数等，与 001-Core 数学类型或项目共用数学库一致；具体类型名以 Core 契约或共用头文件为准。
- **名称/类型**：节点名称与类型信息用于 FindByName/FindByType、GetPath；若与 002-Object 类型描述联动，仅使用 002-Object 契约中已声明的接口。
