# 契约：004-Scene 模块对外 API

## 适用模块

- **实现方**：**004-Scene**（T0 场景/关卡与层次结构）
- **对应规格**：`docs/module-specs/004-scene.md`
- **依赖**：001-Core（001-core-public-api）, 002-Object（002-object-public-api）

## 消费者（T0 下游）

- 005-Entity, 014-Physics, 020-Pipeline, 022-2D, 024-Editor。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SceneRef / WorldRef | 场景/世界容器引用 | 加载后直至卸载 |
| NodeId | 场景图节点 ID；父子关系、层级路径 | 与节点同生命周期 |
| Transform | 局部/世界变换（与 Core.Math 或共用类型一致） | 与节点/实体同步 |
| HierarchyIterator | 层级遍历、按名/按类型查找 | 迭代器，单次有效或由调用方管理 |
| Active 状态 | 节点/子树是否参与更新/渲染 | 与节点绑定 |
| Level 资源句柄 | 关卡加载/卸载边界；与 Resource 配合 | 按 Resource 契约 |

## 能力列表（提供方保证）

1. **场景图**：节点树、父子关系、局部/世界变换、脏标记与变换更新。
2. **层级**：Traverse、FindByName/ByType、GetPath、GetId、SetActive。
3. **World/Level**：GetCurrentWorld、SetActiveWorld、LoadLevel、UnloadLevel；与 Entity 根实体挂接、与 Resource 关卡引用对接。
4. **激活/禁用**：节点与子树参与更新/渲染的开关；下游可查询 Active 状态。

## API 雏形（简化声明）

### 类型与句柄（C++17）

- **WorldRef**：不透明句柄，表示场景/世界容器；生命周期为加载后直至卸载。  
  - 实现可采用 `using WorldRef = void*;` 或 `struct WorldRef { void* handle; };` 等，由 ABI 约定。
- **NodeId**：不透明句柄，表示场景图节点 ID；与节点同生命周期。  
  - 实现可采用 `using NodeId = void*;` 或 `struct NodeId { uintptr_t id; };` 等。
- **Transform**：局部/世界变换，与 **Core.Math** 或共用数学类型一致（如 `struct Transform { Vector3 position; Quaternion rotation; Vector3 scale; };` 或 4×4 矩阵）；与节点/实体同步。
- **HierarchyIterator**：层级遍历、按名/按类型查找用迭代器；**单次有效**（遍历结束后即失效，不可复用）。  
  - 实现为可移动类型，Traverse/FindByName/FindByType 返回；调用方不得在遍历结束后再使用。
- **Active**：节点/子树是否参与更新/渲染；与节点绑定。  
  - 使用 `bool` 或等价类型；SetActive 设置，下游可查询。
- **LevelHandle**：关卡加载/卸载边界；与 **013-Resource** 约定。  
  - 不透明句柄；LoadLevel 返回，UnloadLevel 参数；生命周期按 Resource 契约。

（以上类型名与 Core.Math 的 Vector3/Quaternion 等以 001-core-public-api 及项目共用头文件为准。）

### 场景图（SceneGraph）

- `NodeId CreateNode(WorldRef world);`  
  - 在指定 World 下创建节点，返回 NodeId；失败返回无效 NodeId（由实现约定语义）。
- `bool SetParent(NodeId node, NodeId parent);`  
  - 将 `node` 的父节点设为 `parent`；若会导致父子成环或非法层级，返回 `false` 且不修改场景图；成功返回 `true`。
- `NodeId GetParent(NodeId node);`  
  - 返回节点的父节点 NodeId；无父节点时返回无效 NodeId。
- `void GetChildren(NodeId node, /* out */ NodeId* children, size_t* count);`  
  - 或使用契约允许的容器类型返回子节点列表；具体签名与 001-Core 容器一致。
- `Transform GetLocalTransform(NodeId node);`  
- `void SetLocalTransform(NodeId node, Transform const& t);`  
- `Transform GetWorldTransform(NodeId node);`  
  - WorldTransform 由 UpdateTransforms 更新；未更新前行为由实现约定（如上一帧缓存）。
- `void SetDirty(NodeId node);`  
  - 标记节点（及子树）为脏，下次 UpdateTransforms 时更新 WorldTransform。
- `void UpdateTransforms(WorldRef world);`  
  - 按依赖顺序更新该 World 下所有脏节点的 WorldTransform，并清除脏标记。  
  - 多 World 时，仅更新传入的 World；当前活动 World 的切换在下一帧或本次 UpdateTransforms 入口生效（见 SetActiveWorld）。

### 层级（Hierarchy）

- `HierarchyIterator Traverse(WorldRef world, NodeId root);`  
  - 从 `root` 起按层级顺序遍历子树；返回**单次有效**迭代器，遍历结束后即失效。
- `HierarchyIterator FindByName(WorldRef world, NodeId root, char const* name);`  
  - 在 `root` 子树内按名称查找；返回单次有效迭代器（指向首个匹配或 end）。
- `HierarchyIterator FindByType(WorldRef world, NodeId root, /* TypeId 或等价 */ void* typeFilter);`  
  - 在 `root` 子树内按类型查找；类型过滤与 002-Object 约定一致；返回单次有效迭代器。
- `void GetPath(NodeId node, /* out */ char* pathBuffer, size_t bufferSize);`  
  - 或返回契约允许的字符串类型；路径格式由实现约定。
- `NodeId GetId(HierarchyIterator const& it);`  
  - 返回迭代器当前指向的 NodeId；迭代器失效后调用未定义。
- `void SetActive(NodeId node, bool active);`  
  - 设置节点（及可选子树）的 Active 状态；下游可查询以决定是否参与更新/渲染。
- `bool GetActive(NodeId node);`  
  - 查询节点 Active 状态。

（HierarchyIterator 的 `Next`/`IsValid` 等由实现提供，契约保证单次有效、不可复用。）

### World / Level

- `WorldRef GetCurrentWorld();`  
  - 返回当前活动 WorldRef；多 World 时，SetActiveWorld 在下一帧或下一次 UpdateTransforms 后生效。
- `void SetActiveWorld(WorldRef world);`  
  - 将当前活动 World 设为 `world`；**下一帧或下一次 UpdateTransforms 后**生效，本帧内 GetCurrentWorld 仍返回旧 WorldRef。
- `WorldRef AddWorld();`  
  - 创建并返回新 WorldRef；生命周期为加载后直至显式卸载。
- `/* 与 013-Resource 约定 */ LevelHandle LoadLevel(WorldRef world, /* Resource 句柄或路径等 */ ...);`  
  - 在指定 World 加载关卡；句柄、参数与失败语义由 plan 与 013-Resource 对接约定。
- `void UnloadLevel(WorldRef world, LevelHandle level);`  
  - 卸载指定 Level；语义与 Resource 契约一致，由对接约定。

### Entity 根挂接

- 与 **005-Entity** 对接约定：挂接根实体的接口归属（Scene 提供 AttachEntityRoot 或 Entity 提供 AttachToWorld）及签名在 plan 与 005-Entity 对接后补全；本雏形仅预留“在当前（或指定）World 上挂接 Entity 根实体”的语义，具体 API 写入契约时与 005-Entity 同步。

## 调用顺序与约束

- 须在 Core、Object 可用之后使用；关卡加载可与 013-Resource 约定句柄与加载时机。
- 变换更新顺序与脏标记须保证一致性；多 World 时当前活动场景切换语义明确。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 004-Scene 模块规格与依赖表新增契约 |
| 2026-01-29 | API 雏形：由 plan（feature 004-scene-fullversion-001）同步 |
