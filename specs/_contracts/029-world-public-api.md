# 契约：029-World 模块对外 API

## 适用模块

- **实现方**：029-World（L2；Level/场景资源对上的唯一入口：Level 加载与卸载、关卡句柄、当前关卡场景的获取由本模块对外提供；内部调用 004-Scene 的场景管理算法与 013-Resource 的加载）
- **对应规格**：`docs/module-specs/029-world.md`
- **依赖**：004-Scene、013-Resource

## 消费者

- 020-Pipeline、024-Editor（需 Level 句柄或当前关卡 SceneRef 时通过本模块获取；可选）

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| LevelHandle | 关卡资源句柄；与 013 的 Level 资源对应；029 持有并可与 SceneRef 绑定 | 创建后直至 UnloadLevel |
| SceneRef | 来自 004-Scene；029 在 CreateLevelFromDesc 时通过调用 004 CreateSceneFromDesc 获得并持有，对外提供给上层用于遍历/查询 | 与 Level 绑定 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | Level 生命周期 | CreateLevelFromDesc：从 013 取得 LevelAssetDesc、nodeModelRefs 后，转换为 004 的 SceneDesc 与按节点的不透明句柄，再调用 004 CreateSceneFromDesc(SceneDesc, …)，返回 LevelHandle/SceneRef；UnloadLevel：释放 Level 句柄并调用 004 UnloadScene；（可选）LoadLevelAsync 与 013 协作封装 |
| 2 | 当前场景获取 | GetCurrentLevelScene/GetSceneRef：返回当前关卡对应的 SceneRef；上层经此获取 SceneRef 后调用 004 的遍历/查询 API |
| 3 | 委托式场景遍历（可选） | GetRootNodes(LevelHandle)、Traverse(LevelHandle, callback) 等，内部转调 004 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 004-Scene、013-Resource 初始化之后使用。Level 加载流程：上层调 029 → 029 用 013 Load 取得 LevelAssetDesc、nodeModelRefs → 029 转换为 004 的 SceneDesc 与不透明句柄 → 029 调用 004 CreateSceneFromDesc → 029 持有关卡句柄与 SceneRef。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 场景拆分为 004-Scene 与 029-World；029 契约 |
| 2026-02-05 | 统一目录；能力列表用表格；029 调用 004 前将 LevelAssetDesc 转为 SceneDesc |
