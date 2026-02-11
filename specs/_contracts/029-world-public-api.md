# 契约：029-World 模块对外 API

## 适用模块

- **实现方**：029-World（L2；Level/场景资源对上的唯一入口：Level 加载与卸载、关卡句柄、当前关卡场景的获取由本模块对外提供；内部调用 004-Scene 的场景管理算法与 013-Resource 的加载）
- **对应规格**：`docs/module-specs/029-world.md`
- **依赖**：004-Scene、013-Resource、005-Entity

## 消费者

- 020-Pipeline、024-Editor（需 Level 句柄或当前关卡 SceneRef 时通过本模块获取；可选）

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| LevelHandle | 关卡资源句柄；与 013 的 Level 资源对应；029 持有并可与 SceneRef 绑定 | 创建后直至 UnloadLevel |
| SceneRef | 来自 004-Scene；029 在 CreateLevelFromDesc 时通过调用 004 CreateSceneFromDesc 获得并持有，对外提供给上层用于遍历/查询 | 与 Level 绑定 |
| IModelResource | 模型资源视图；聚合 IMeshResource*、IMaterialResource* 与 submeshMaterialIndices；由 013 经 RequestLoadAsync(..., Model, ...) 加载后返回，类型与接口归属 029 | 013 缓存或调用方解析持有 |
| ModelAssetDesc | .model 描述；meshGuids、materialGuids、submeshMaterialIndices 等；029 拥有并向 002 注册；013 反序列化 .model 得到后交 029 或 013 组装 RResource | 与 .model 资源绑定 |
| RenderableItem | 单条可渲染项；worldMatrix、modelResource、submeshIndex；由 CollectRenderables 回调提供 | 单次回调内有效 |
| LightComponent | 灯光组件；LightType（Point/Directional/Spot）、color、intensity、range、direction、spotAngle；继承 Component | 与 Entity 绑定 |
| CameraComponent | 相机组件；fovY、nearZ、farZ、isActive；继承 Component；FrameContext.camera 为空时可由 Active 相机提供视图 | 与 Entity 绑定 |
| ReflectionProbeComponent | 反射探针组件；ReflectionProbeType（Box/Sphere）、extent、resolution；继承 Component | 与 Entity 绑定 |
| DecalComponent | 贴花组件；albedoTextureId、size、blend；继承 Component | 与 Entity 绑定 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | Level 生命周期 | CreateLevelFromDesc：从 013 取得 LevelAssetDesc、nodeModelRefs 后，转换为 004 的 SceneDesc 与按节点的不透明句柄，再调用 004 CreateSceneFromDesc(SceneDesc, …)，返回 LevelHandle/SceneRef；UnloadLevel：释放 Level 句柄并调用 004 UnloadScene；（可选）LoadLevelAsync 与 013 协作封装 |
| 2 | 当前场景获取 | GetCurrentLevelScene/GetSceneRef：返回当前关卡对应的 SceneRef；上层经此获取 SceneRef 后调用 004 的遍历/查询 API |
| 3 | 委托式场景遍历 | GetRootNodes(LevelHandle)、Traverse(LevelHandle, callback) 等，内部转调 004 |
| 4 | 渲染物收集 | CollectRenderables(LevelHandle, callback)：遍历场景，对带 ModelComponent 的 Entity 调用 callback(ISceneNode*, RenderableItem)；020-Pipeline 经此获取待渲染项，不再直接使用 004 GetNodeModelGuid / 005 GetModelGuid |
| 5 | 灯光收集 | CollectLights(SceneRef, callback)：遍历场景，对带 LightComponent 的 Entity 调用 callback(ISceneNode*, LightComponent)；020 经此填充 ILightItemList |
| 6 | 相机收集 | CollectCameras(SceneRef, callback)：遍历场景，对带 CameraComponent 的 Entity 调用 callback(ISceneNode*, CameraComponent) |
| 7 | 反射探针收集 | CollectReflectionProbes(SceneRef, callback)：遍历场景，对带 ReflectionProbeComponent 的 Entity 调用 callback(ISceneNode*, ReflectionProbeComponent) |
| 8 | 贴花收集 | CollectDecals(SceneRef, callback)：遍历场景，对带 DecalComponent 的 Entity 调用 callback(ISceneNode*, DecalComponent) |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 004-Scene、013-Resource 初始化之后使用。Level 加载流程：上层调 029 → 029 用 013 Load 取得 LevelAssetDesc、nodeModelRefs → 029 转换为 004 的 SceneDesc 与不透明句柄 → 029 调用 004 CreateSceneFromDesc → 029 持有关卡句柄与 SceneRef。

## Level 文件格式约定

- Level 资源支持**两种文件形式**，格式由 013 加载时经 002 的 **GetFormatFromPath(path)** 根据路径扩展名自动选择：
  - **二进制**：路径为 `*.level` 或非 .json/.xml 扩展名时，使用 Binary 格式（002 SerializationFormat::Binary）。
  - **JSON**：路径以 `.json` 结尾（大小写不敏感，如 `levels/main.level.json` 或 `levels/main.json`）时，使用 JSON 格式（002 SerializationFormat::JSON）。
- 同一逻辑关卡可存为 `xxx.level`（二进制）或 `xxx.level.json`（JSON，便于版本管理与手工编辑）；ResolvePath 返回的 path 决定实际格式。

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计。）

- [x] **描述归属**：LevelAssetDesc、SceneNodeDesc、ModelAssetDesc、IModelResource 归属 029；已向 002 注册 LevelAssetDesc、SceneNodeDesc、ModelAssetDesc；013 Level 工厂由 029 注册。
- [x] **CreateLevelFromDesc**：CreateLevelFromDesc(LevelAssetDesc) 与 CreateLevelFromDesc(ResourceId)（内部 013 LoadSync(Level)）；NodeFactoryFn 用 EntityManager::CreateEntity 并挂 ModelComponent。
- [x] **UnloadLevel**：先销毁本 Level 下所有 Entity，再调用 004 UnloadScene；顺序约定见计划。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 场景拆分为 004-Scene 与 029-World；029 契约 |
| 2026-02-05 | 统一目录；能力列表用表格；029 调用 004 前将 LevelAssetDesc 转为 SceneDesc |
| 2026-02-05 | IModelResource、ModelAssetDesc 归属转入 029-World（原 013-Resource） |
| 2026-02-10 | 依赖增加 005-Entity；能力增加 CollectRenderables（渲染物收集）；类型增加 RenderableItem；020 待渲染项来源改为本模块 CollectRenderables |
| 2026-02-10 | 实现 LevelAssetDesc/SceneNodeDesc、CreateLevelFromDesc(LevelAssetDesc|ResourceId)、UnloadLevel 顺序约定、CollectRenderables(SceneRef)、002 类型注册、013 Level 工厂；待渲染项由本模块 CollectRenderables 统一提供 |
| 2026-02-10 | Level 双格式：支持二进制 .level 与 JSON .level.json；格式由 002 GetFormatFromPath(path) 按路径扩展名自动选择 |
| 2026-02-11 | 新增 LightComponent、CameraComponent、ReflectionProbeComponent、DecalComponent；WorldManager 新增 CollectLights、CollectCameras、CollectReflectionProbes、CollectDecals(SceneRef, callback) |