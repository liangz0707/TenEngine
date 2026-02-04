# 004-Scene 模块 ABI（全量 — 实现参考）

- **用途**：tasks 与 implement 阶段**必须基于本全量 ABI** 进行实现（原始 + 新增 + 修改）。
- **契约**：[004-scene-public-api.md](../../_contracts/004-scene-public-api.md)
- **命名空间约定**：本分支统一使用 **`te::`**；004-Scene 符号位于 **`te::scene`**，头文件路径 **`te/scene/`**（与 `te::pipelinecore` 等一致）。
- **参考**：Unity SceneManager/Transform、UE UWorld/Level。

以下为**全量 ABI 内容**（含现有 ABI 表与 TODO 必须项对应的新增条目）。

---

## 1. 类型与枚举

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | LoadSceneMode | 全局枚举 | 场景加载模式 | te/scene/SceneManager.h | LoadSceneMode::Single, Additive | Single=替换当前场景；Additive=叠加加载 |
| 004-Scene | te::scene | SceneRef | struct/句柄 | 场景引用 | te/scene/SceneTypes.h | SceneRef | 已加载场景，用于 SetActiveScene/UnloadScene |
| 004-Scene | te::scene | NodeId | struct/句柄 | 场景图节点 ID | te/scene/SceneTypes.h | NodeId | 场景图节点，层级路径与查找 |
| 004-Scene | te::scene | WorldRef | struct/句柄 | 世界容器引用 | te/scene/SceneTypes.h | WorldRef | 可与 SceneRef 统一为同一类型别名 |
| 004-Scene | te::scene | Transform | struct | 局部/世界变换 | te/scene/SceneTypes.h 或 Core.Math | Transform | position, rotation, scale；与 Core.Math 或契约约定一致 |
| 004-Scene | te::scene | HierarchyIterator | 类 | 层级遍历/查找迭代器 | te/scene/Hierarchy.h | HierarchyIterator | 单次有效、可移动；Traverse/FindByName/FindByType 返回 |
| 004-Scene | te::scene | LevelHandle | struct/句柄 | 关卡句柄 | te/scene/Level.h | LevelHandle | 与 013-Resource 约定；LoadLevel 返回，UnloadLevel 参数 |

---

## 2. 契约 API（public-api 自由函数 — 必须实现）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | — | 自由函数 | 创建节点 | scene/scene_graph.hpp | CreateNode | `NodeId CreateNode(WorldRef world);` |
| 004-Scene | te::scene | — | 自由函数 | 设置父节点 | scene/scene_graph.hpp | SetParent | `bool SetParent(NodeId node, NodeId parent);` 成环返回 false |
| 004-Scene | te::scene | — | 自由函数 | 获取父节点 | scene/scene_graph.hpp | GetParent | `NodeId GetParent(NodeId node);` |
| 004-Scene | te::scene | — | 自由函数 | 获取子节点 | scene/scene_graph.hpp | GetChildren | `void GetChildren(NodeId node, NodeId* children, size_t* count);` |
| 004-Scene | te::scene | — | 自由函数 | 局部/世界变换 | scene/scene_graph.hpp | GetLocalTransform, SetLocalTransform, GetWorldTransform | 见契约 |
| 004-Scene | te::scene | — | 自由函数 | 脏标记与更新 | scene/scene_graph.hpp | SetDirty, UpdateTransforms | `void SetDirty(NodeId); void UpdateTransforms(WorldRef);` |
| 004-Scene | te::scene | — | 自由函数 | 层级遍历与查找 | scene/hierarchy.hpp | Traverse, FindByName, FindByType | 返回 HierarchyIterator（单次有效） |
| 004-Scene | te::scene | — | 自由函数 | 路径与 ID | scene/hierarchy.hpp | GetPath, GetId | `void GetPath(NodeId, char* pathBuffer, size_t bufferSize); NodeId GetId(HierarchyIterator const&);` |
| 004-Scene | te::scene | — | 自由函数 | 激活状态 | scene/hierarchy.hpp | SetActive, GetActive | `void SetActive(NodeId, bool); bool GetActive(NodeId);` |
| 004-Scene | te::scene | — | 自由函数 | 当前 World | scene/world.hpp | GetCurrentWorld, SetActiveWorld, AddWorld | SetActiveWorld 下一帧或下次 UpdateTransforms 生效 |
| 004-Scene | te::scene | — | 自由函数 | 关卡加载/卸载 | scene/level.hpp | LoadLevel, UnloadLevel | 与 013-Resource 约定；见契约 |

---

## 3. 场景加载与活动场景（SceneManager 风格）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | — | 自由函数 | 同步加载场景 | te/scene/SceneManager.h | LoadScene | `ISceneWorld* LoadScene(char const* pathOrName, LoadSceneMode mode);` |
| 004-Scene | te::scene | — | 自由函数 | 异步加载场景 | te/scene/SceneManager.h | LoadSceneAsync | 回调或 IAsyncLoadHandle |
| 004-Scene | te::scene | — | 自由函数 | 卸载场景 | te/scene/SceneManager.h | UnloadScene | `void UnloadScene(SceneRef scene);` 销毁场景图与节点数据 |
| 004-Scene | te::scene | — | 自由函数 | 当前活动场景 | te/scene/SceneManager.h | GetActiveScene, SetActiveScene | 当前活动 WorldRef/SceneRef |
| 004-Scene | te::scene | — | 自由函数 | 已加载场景列表 | te/scene/SceneManager.h | GetLoadedSceneCount, GetSceneAt | 0 ≤ index < GetLoadedSceneCount() |

---

## 4. 场景世界（ISceneWorld）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | CreateEntity, DestroyEntity, GetEntities | te/scene/SceneWorld.h | 见现有 ABI | 与 005-Entity 约定 |
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | GetRootNodes, GetSceneRef, UpdateTransforms | te/scene/SceneWorld.h | 见现有 ABI | 根节点、世界引用、变换更新 |
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | FindNodeByName, FindNodeById, CreateNode | te/scene/SceneWorld.h | 见现有 ABI | 按名/ID 查找、创建子节点 |
| 004-Scene | te::scene | — | 自由函数 | GetSceneWorld | te/scene/SceneWorld.h | GetSceneWorld | 当前活动世界 |

---

## 5. 场景图与节点（ISceneNode）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | ISceneNode | 抽象接口 | GetParent, SetParent, GetChildren | te/scene/SceneNode.h | 见现有 ABI | 父子关系 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | GetLocalTransform, SetLocalTransform, GetWorldTransform, GetWorldMatrix | te/scene/SceneNode.h | 见现有 ABI | 变换 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | GetNodeId, GetPath, SetActive, IsActive | te/scene/SceneNode.h | 见现有 ABI | ID、路径、激活 |

---

## 6. 数据与注册（TODO 必须实现 — 本 feature 新增）

### 6.1 类型注册（002-Object）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | — | 启动时调用 | LevelAssetDesc 注册 | 004 初始化 | RegisterType&lt;LevelAssetDesc&gt; | 通过 002 在引擎启动时注册 |
| 004-Scene | te::scene | — | 启动时调用 | SceneNodeDesc 注册 | 004 初始化 | RegisterType&lt;SceneNodeDesc&gt; | 通过 002 注册 |

### 6.2 数据结构（反序列化与存储）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | LevelAssetDesc | struct/描述符 | 关卡资源描述 | te/scene/LevelTypes.h | LevelAssetDesc | formatVersion, debugDescription, rootNodes, defaultWorldSettings |
| 004-Scene | te::scene | SceneNodeDesc | struct/描述符 | 场景节点描述 | te/scene/SceneNodeTypes.h | SceneNodeDesc | name, localTransform, children, modelGuid, entityPrefabGuid, components, active |
| 004-Scene | te::scene | TransformDesc | struct/描述符 | 变换描述 | te/scene/SceneTypes.h | TransformDesc | position, rotation, scale |

### 6.3 对外接口（TODO 必须提供）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | — | 自由函数 | 节点模型 GUID | te/scene/SceneNode.h | GetNodeModelGuid | `ResourceId GetNodeModelGuid(ISceneNode* node);` 与 013 约定 |
| 004-Scene | te::scene | — | 自由函数 | 节点实体预制 GUID | te/scene/SceneNode.h | GetNodeEntityPrefabGuid | `ResourceId GetNodeEntityPrefabGuid(ISceneNode* node);` 与 005/013 约定 |

### 6.4 反序列化与调用流程

- **LoadScene(pathOrResourceId, mode)**：解析路径或 ResourceId → 001.FileRead 读 .level → 002.Deserialize 得到 LevelAssetDesc → 从 LevelAssetDesc 构建 ISceneWorld、ISceneNode 树；节点存储 modelGuid、entityPrefabGuid。
- **UnloadScene(scene)**：销毁该场景下所有 Entity 与节点，释放引用；与 013-Resource 协同。
- 下游通过 GetNodeModelGuid、GetNodeEntityPrefabGuid 取得 GUID 后自行加载资源并挂接。

---

*全量 ABI 完。实现时须覆盖本节与上文全部条目；plan.md 的「契约更新」小节仅含相对于 `specs/_contracts/004-scene-ABI.md` 的**新增与修改**部分。*
