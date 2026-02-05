# 契约：004-Scene 模块对外 API

## 适用模块

- **实现方**：004-Scene（L1；仅场景管理算法：场景图、层级、变换、激活/禁用；不持有资源、不依赖 Resource。Level/关卡句柄由 029-World 负责，上层通过 029 调用本模块）
- **对应规格**：`docs/module-specs/004-scene.md`
- **依赖**：001-Core、002-Object

## 消费者

- 029-World（内部调 004 CreateSceneFromDesc/UnloadScene）、005-Entity、014-Physics、020-Pipeline、024-Editor（经 029 获取 SceneRef 后调本模块遍历 API 或 029 委托 API）

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SceneRef / WorldRef | 场景/世界容器引用 | 加载后直至卸载 |
| NodeId | 场景图节点 ID；父子关系、层级路径；与 Entity 一一对应或映射 | 与节点同生命周期 |
| Transform | 局部/世界变换（与 Core.Math 或共用类型一致） | 与节点/实体同步 |
| ISceneWorld | 场景世界接口：CreateEntity、DestroyEntity、GetEntities、GetRootNodes、UpdateTransforms、FindNodeByName、FindNodeById、CreateNode、GetSceneRef | 与 SceneRef 绑定 |
| ISceneNode | 场景节点接口：GetParent、SetParent、GetChildren、GetLocalTransform、SetLocalTransform、GetWorldTransform、GetWorldMatrix、GetNodeId、GetPath、SetActive、IsActive | 与节点同生命周期 |
| HierarchyIterator | 层级遍历、按名/按类型查找 | 迭代器，单次有效或由调用方管理 |
| Active 状态 | 节点/子树是否参与更新/渲染 | 与节点绑定 |
| Node 上不透明句柄 | CreateSceneFromDesc 时按节点传入；004 不解析、不依赖 029/013 类型，仅存储 | 与节点同生命周期 |
| SceneDesc | 004 定义的入参：节点数组（父索引、局部变换、名称、每节点不透明 model 句柄）；029 将 LevelAssetDesc 转换后传入 | 单次调用 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 场景图 | 节点树、父子关系、局部/世界变换、脏标记与变换更新；ISceneWorld::UpdateTransforms |
| 2 | 层级 | Traverse、FindByName/ByType、GetPath、GetId、SetActive；GetRootNodes、GetChildren、FindNodeByName、FindNodeById；调用方经 029 获取 SceneRef 后使用 |
| 3 | World/Scene 容器 | GetCurrentWorld、GetActiveSceneWorld、SetActiveScene；CreateSceneFromDesc(SceneDesc, …)（入参为 004 定义，029 转换后调用）、UnloadScene；与 Entity 根实体挂接 |
| 4 | 激活/禁用 | SetActive、IsActive；节点与子树参与更新/渲染的开关 |

入参 CreateSceneFromDesc 不引用 029/013 类型，避免 L1→L2 依赖。命名空间 `te::scene`；头文件 SceneTypes.h、SceneWorld.h、SceneNode.h、SceneManager.h 等。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Object 可用后使用。Level 加载由上层经 029 进行；029 将 LevelAssetDesc 转为 SceneDesc 后调 004 CreateSceneFromDesc。变换更新与脏标记须一致；多 World 时当前活动场景语义由 004 提供、029 可封装。

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计。）

- [ ] **无资源职责**：004 不负责资源与关卡描述；CreateSceneFromDesc(SceneDesc) 入参由 029 转换后传入；004 不持有 *AssetDesc、不依赖 013、不解析节点上的不透明句柄为资源类型。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 004-Scene 契约；仅场景管理算法；上层经 029 调用 |
| 2026-02-05 | CreateSceneFromDesc 入参改为 004 的 SceneDesc，避免依赖 029/013 |
| 2026-02-05 | 统一目录；能力列表用表格；不引用 ABI 文件 |
