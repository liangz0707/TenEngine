# 029-World Module ABI

- **Contract**: [029-world-public-api.md](./029-world-public-api.md) (capabilities and type descriptions)
- **This file**: 029-World public ABI explicit table.

## ABI Table

| Module | Namespace | Class/Type | Interface | Header | Symbol | Description |
|--------|-----------|------------|-----------|--------|--------|-------------|
| 029-World | te::world | LevelHandle | struct | te/world/WorldTypes.h | LevelHandle | Level handle; value, IsValid(), operator==, operator!= |
| 029-World | te::world | RenderableItem | struct | te/world/WorldTypes.h | RenderableItem | worldMatrix[16], element (IRenderElement*), submeshIndex, modelResourceId, boundsMin[3], boundsMax[3], userData; for 020-Pipeline collection |
| 029-World | te::world | WorldManager | class | te/world/WorldManager.h | WorldManager | Singleton; Level lifecycle, SceneRef, traversal, CollectRenderables, ExportLevelToDesc, SaveLevel |
| 029-World | te::world | WorldManager | method | te/world/WorldManager.h | CreateLevelFromDesc | `LevelHandle CreateLevelFromDesc(SpatialIndexType indexType, AABB const& bounds, LevelAssetDesc const& desc);` Overload: `LevelHandle CreateLevelFromDesc(SpatialIndexType indexType, AABB const& bounds, ResourceId levelResourceId);` |
| 029-World | te::world | WorldManager | method | te/world/WorldManager.h | UnloadLevel | `void UnloadLevel(LevelHandle handle);` |
| 029-World | te::world | WorldManager | method | te/world/WorldManager.h | GetSceneRef | `SceneRef GetSceneRef(LevelHandle handle) const;` |
| 029-World | te::world | WorldManager | method | te/world/WorldManager.h | GetCurrentLevelScene | `SceneRef GetCurrentLevelScene() const;` |
| 029-World | te::world | WorldManager | method | te/world/WorldManager.h | GetRootNodes | `void GetRootNodes(LevelHandle handle, std::vector<ISceneNode*>& out) const;` |
| 029-World | te::world | WorldManager | method | te/world/WorldManager.h | Traverse | `void Traverse(LevelHandle handle, std::function<void(ISceneNode*)> const& callback) const;` |
| 029-World | te::world | WorldManager | method | te/world/WorldManager.h | CollectRenderables | `void CollectRenderables(LevelHandle handle, std::function<void(ISceneNode*, RenderableItem const&)> const& callback) const;` Overloads: CollectRenderables(SceneRef, callback), CollectRenderables(SceneRef, IResourceManager*, callback) |
| 029-World | te::world | WorldManager | method | te/world/WorldManager.h | ExportLevelToDesc | `bool ExportLevelToDesc(LevelHandle handle, LevelAssetDesc& out) const;` Exports scene to LevelAssetDesc for Save |
| 029-World | te::world | WorldManager | method | te/world/WorldManager.h | SaveLevel | `bool SaveLevel(LevelHandle handle, char const* path) const;` Exports, creates LevelResource, calls IResourceManager::Save |
| 029-World | te::world | LevelAssetDesc | struct | te/world/LevelAssetDesc.h | LevelAssetDesc | .level description; roots (SceneNodeDesc tree); owned by 029 and registered with 002; supports binary .level and JSON .level.json, format determined by path extension |
| 029-World | te::world | SceneNodeDesc | struct | te/world/LevelAssetDesc.h | SceneNodeDesc | name, localTransform, modelGuid, children; owned by 029 and registered with 002 |
| 029-World | te::world | ILevelResource | interface | te/world/LevelResource.h | ILevelResource | GetLevelAssetDesc; 013 LoadSync(Level) returns IResource* castable to this type |
| 029-World | te::world | LevelResourceFactory | struct | te/world/LevelResource.h | LevelResourceFactory | `static IResource* Create(ResourceType type);` Factory for 013 RegisterResourceFactory |
| 029-World | te::world | CreateLevelResourceFromDesc | free function | te/world/LevelResource.h | CreateLevelResourceFromDesc | `IResource* CreateLevelResourceFromDesc(LevelAssetDesc const& desc);` For Save (Editor export flow) |
| 029-World | te::world | IModelResource | abstract interface | te/world/ModelResource.h | IModelResource | GetMesh, GetMaterialCount, GetMaterial, GetSubmeshMaterialIndex; 013 LoadSync(..., Model) returns IResource* castable to this type |
| 029-World | te::world | ModelAssetDesc | struct | te/world/ModelAssetDesc.h | ModelAssetDesc | meshGuids, materialGuids, submeshMaterialIndices; owned by 029 and registered with 002 |
| 029-World | te::world | ModelComponent | struct | te/world/ModelComponent.h | ModelComponent | Inherits Component; modelResourceId; registered to 005/002 via RegisterWorldModule |
| 029-World | te::world | -- | free function | te/world/WorldModuleInit.h | RegisterWorldModule | `void RegisterWorldModule();` Registers ModelComponent etc. 029 component types |
| 029-World | te::world | LightType | enum | te/world/LightComponent.h | LightType | Point = 0, Directional, Spot |
| 029-World | te::world | LightComponent | struct | te/world/LightComponent.h | LightComponent | Inherits Component; type, color[3], intensity, range, direction[3], spotAngle |
| 029-World | te::world | CameraComponent | struct | te/world/CameraComponent.h | CameraComponent | Inherits Component; fovY, nearZ, farZ, isActive |
| 029-World | te::world | ReflectionProbeType | enum | te/world/ReflectionProbeComponent.h | ReflectionProbeType | Box = 0, Sphere |
| 029-World | te::world | ReflectionProbeComponent | struct | te/world/ReflectionProbeComponent.h | ReflectionProbeComponent | Inherits Component; type, extent[3], resolution |
| 029-World | te::world | DecalComponent | struct | te/world/DecalComponent.h | DecalComponent | Inherits Component; albedoTextureId, size[3], blend |

## Relationship with 004-Scene, 013-Resource, 005-Entity

- World calls 004 **CreateSceneFromDesc**, **UnloadScene**, **GetRootNodes**/Traverse; calls 013 Load, ResourceId/handle resolution; calls 005 **EntityManager::CreateEntity**, **DestroyEntity**, **ModelComponent** etc.
- Specific symbols and signatures per 004-scene-public-api, 013-resource-public-api, 005-entity and this module's implementation.

## Change Log

| Date | Change |
|------|--------|
| 2026-02-10 | Complete ABI table: LevelHandle, RenderableItem, WorldManager and CreateLevelFromDesc/UnloadLevel/GetSceneRef/GetCurrentLevelScene/GetRootNodes/Traverse/CollectRenderables; IModelResource, ModelAssetDesc marked as to implement |
| 2026-02-10 | Implemented IModelResource, ModelAssetDesc, ModelComponent; CollectRenderables only callbacks for Entities with ModelComponent; added CollectRenderables(handle, IResourceManager*, callback) overload for model resolution; RegisterWorldModule, WorldModuleInit |
| 2026-02-10 | Added LevelAssetDesc, SceneNodeDesc, ILevelResource; CreateLevelFromDesc(LevelAssetDesc) and CreateLevelFromDesc(ResourceId); CollectRenderables(SceneRef, ...) overload; register LevelAssetDesc/SceneNodeDesc/ModelAssetDesc with 002; 013 Level factory registration; UnloadLevel order: Entity first then UnloadScene |
| 2026-02-10 | Level dual format: supports binary .level and JSON .level.json; format auto-selected by 002 GetFormatFromPath(path) based on extension |
| 2026-02-11 | Added LightComponent, CameraComponent, ReflectionProbeComponent, DecalComponent; WorldManager added CollectLights, CollectCameras, CollectReflectionProbes, CollectDecals |
| 2026-02-22 | Updated to match actual implementation: RenderableItem fields (element, modelResourceId, boundsMin/Max, userData); WorldManager methods (ExportLevelToDesc, SaveLevel); removed CollectLights/Cameras/ReflectionProbes/Decals (not implemented); added LevelResourceFactory, CreateLevelResourceFromDesc |
