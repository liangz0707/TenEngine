# 029-World 模块 ABI

- **契约**：[029-world-public-api.md](./029-world-public-api.md)（能力与类型描述）
- **本文件**：029-World 对外 ABI 显式表。

## ABI 表

| 模块名 | 命名空间 | 类名/类型 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|-----------|----------|--------|------|------|
| 029-World | te::world | LevelHandle | 结构体 | te/world/WorldTypes.h | LevelHandle | 关卡句柄；value/IsValid/operator== |
| 029-World | te::world | RenderableItem | 结构体 | te/world/WorldTypes.h | RenderableItem | worldMatrix、modelResource、submeshIndex；供 020-Pipeline 收集 |
| 029-World | te::world | WorldManager | 类 | te/world/WorldManager.h | WorldManager | 单例；Level 生命周期、SceneRef、遍历、CollectRenderables |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | CreateLevelFromDesc | LevelHandle CreateLevelFromDesc(indexType, bounds, LevelAssetDesc)；重载 CreateLevelFromDesc(indexType, bounds, ResourceId) |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | UnloadLevel | void UnloadLevel(LevelHandle) |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | GetSceneRef | SceneRef GetSceneRef(LevelHandle) const |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | GetCurrentLevelScene | SceneRef GetCurrentLevelScene() const |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | GetRootNodes | void GetRootNodes(LevelHandle, vector<ISceneNode*>&) const |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | Traverse | void Traverse(LevelHandle, function<void(ISceneNode*)>) const |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | CollectRenderables | void CollectRenderables(LevelHandle, callback) const；仅对带 ModelComponent 的 Entity 回调 |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | CollectRenderables | void CollectRenderables(LevelHandle, IResourceManager*, callback) const；重载 CollectRenderables(SceneRef, callback)、CollectRenderables(SceneRef, IResourceManager*, callback) |
| 029-World | te::world | LevelAssetDesc | 结构体 | te/world/LevelAssetDesc.h | LevelAssetDesc | .level 描述；roots（SceneNodeDesc 树）；029 拥有并向 002 注册；支持二进制 .level 与 JSON .level.json，格式由路径扩展名决定 |
| 029-World | te::world | SceneNodeDesc | 结构体 | te/world/LevelAssetDesc.h | SceneNodeDesc | name、localTransform、modelGuid、children；029 拥有并向 002 注册 |
| 029-World | te::world | ILevelResource | 接口 | te/world/LevelResource.h | ILevelResource | GetLevelAssetDesc；013 LoadSync(Level) 返回 IResource* 可转型为此类型 |
| 029-World | te::world | IModelResource | 抽象接口 | te/world/ModelResource.h | IModelResource | GetMesh、GetMaterialCount、GetMaterial、GetSubmeshMaterialIndex；013 LoadSync(..., Model) 返回 IResource* 可转型为此类型 |
| 029-World | te::world | ModelAssetDesc | 结构体 | te/world/ModelAssetDesc.h | ModelAssetDesc | meshGuids、materialGuids、submeshMaterialIndices；029 拥有并向 002 注册 |
| 029-World | te::world | ModelComponent | 结构体 | te/world/ModelComponent.h | ModelComponent | 继承 Component；modelResourceId；由 RegisterWorldModule 注册到 005/002 |
| 029-World | te::world | — | 自由函数 | te/world/WorldModuleInit.h | RegisterWorldModule | void RegisterWorldModule(); 注册 ModelComponent 等 029 组件类型 |
| 029-World | te::world | LightComponent | 结构体 | te/world/LightComponent.h | LightComponent | 继承 Component；LightType、color、intensity、range、direction、spotAngle |
| 029-World | te::world | CameraComponent | 结构体 | te/world/CameraComponent.h | CameraComponent | 继承 Component；fovY、nearZ、farZ、isActive |
| 029-World | te::world | ReflectionProbeComponent | 结构体 | te/world/ReflectionProbeComponent.h | ReflectionProbeComponent | 继承 Component；ReflectionProbeType、extent、resolution |
| 029-World | te::world | DecalComponent | 结构体 | te/world/DecalComponent.h | DecalComponent | 继承 Component；albedoTextureId、size、blend |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | CollectLights | void CollectLights(SceneRef, function<void(ISceneNode*, LightComponent const&)>) const |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | CollectCameras | void CollectCameras(SceneRef, function<void(ISceneNode*, CameraComponent const&)>) const |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | CollectReflectionProbes | void CollectReflectionProbes(SceneRef, function<void(ISceneNode*, ReflectionProbeComponent const&)>) const |
| 029-World | te::world | WorldManager | 方法 | te/world/WorldManager.h | CollectDecals | void CollectDecals(SceneRef, function<void(ISceneNode*, DecalComponent const&)>) const |

## 与 004-Scene、013-Resource、005-Entity 的调用关系

- World 调用 004 **CreateSceneFromDesc**、**UnloadScene**、**GetRootNodes**/Traverse；调用 013 Load、ResourceId/句柄解析；调用 005 **EntityManager::CreateEntity**、**DestroyEntity**、**ModelComponent** 等。
- 具体符号与签名以 004-scene-public-api、013-resource-public-api、005-entity 及本模块实现为准。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| 2026-02-10 | 完整 ABI 表：LevelHandle、RenderableItem、WorldManager 及 CreateLevelFromDesc/UnloadLevel/GetSceneRef/GetCurrentLevelScene/GetRootNodes/Traverse/CollectRenderables；IModelResource、ModelAssetDesc 标为待实现 |
| 2026-02-10 | 实现 IModelResource、ModelAssetDesc、ModelComponent；CollectRenderables 仅对带 ModelComponent 的 Entity 回调；增加 CollectRenderables(handle, IResourceManager*, callback) 重载以解析 modelResource；RegisterWorldModule、WorldModuleInit |
| 2026-02-10 | 新增 LevelAssetDesc、SceneNodeDesc、ILevelResource；CreateLevelFromDesc(LevelAssetDesc) 与 CreateLevelFromDesc(ResourceId)；CollectRenderables(SceneRef,…) 重载；向 002 注册 LevelAssetDesc/SceneNodeDesc/ModelAssetDesc；013 Level 工厂注册；UnloadLevel 顺序：先 Entity 再 UnloadScene |
| 2026-02-10 | Level 双格式：.level 二进制与 .level.json JSON，格式由 002 GetFormatFromPath 按路径扩展名选择 |
| 2026-02-11 | 新增 LightComponent、CameraComponent、ReflectionProbeComponent、DecalComponent；WorldManager 新增 CollectLights、CollectCameras、CollectReflectionProbes、CollectDecals |
