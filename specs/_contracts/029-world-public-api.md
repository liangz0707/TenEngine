# Contract: 029-World Module Public API

## Applicable Module

- **Implementor**: 029-World (L2; Level/scene resource single entry point: Level load/unload, level handle, current level scene retrieval; internally calls 004-Scene scene management algorithms and 013-Resource loading)
- **Spec**: `docs/module-specs/029-world.md`
- **Dependencies**: 004-Scene, 013-Resource, 005-Entity

## Consumers

- 020-Pipeline, 024-Editor (get Level handle or current level SceneRef via this module; optional)

## Capabilities

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| LevelHandle | Level resource handle; corresponds to 013 Level resource; 029 holds and can bind to SceneRef | Created until UnloadLevel |
| SceneRef | From 004-Scene; 029 obtains via 004 CreateSceneFromDesc during CreateLevelFromDesc and holds, provides to upper layer for traversal/query | Bound to Level |
| IModelResource | Model resource view; aggregates IMeshResource*, IMaterialResource* and submeshMaterialIndices; returned by 013 via RequestLoadAsync(..., Model, ...), type and interface owned by 029 | 013 cache or caller holds |
| ModelAssetDesc | .model description; meshGuids, materialGuids, submeshMaterialIndices; owned by 029 and registered with 002; 013 deserializes .model and passes to 029 or 013 assembles RResource | Bound to .model resource |
| RenderableItem | Single renderable item; worldMatrix, element (IRenderElement*), submeshIndex, modelResourceId, boundsMin/Max, userData; provided by CollectRenderables callback | Valid within single callback |
| LightComponent | Light component; LightType (Point/Directional/Spot), color, intensity, range, direction, spotAngle; inherits Component | Bound to Entity |
| CameraComponent | Camera component; fovY, nearZ, farZ, isActive; inherits Component; FrameContext.camera can use Active camera for view | Bound to Entity |
| ReflectionProbeComponent | Reflection probe component; ReflectionProbeType (Box/Sphere), extent, resolution; inherits Component | Bound to Entity |
| DecalComponent | Decal component; albedoTextureId, size, blend; inherits Component | Bound to Entity |

### Capabilities (Provider Guarantees)

| ID | Capability | Description |
|----|------------|-------------|
| 1 | Level Lifecycle | CreateLevelFromDesc: gets LevelAssetDesc and nodeModelRefs from 013, converts to 004 SceneDesc and opaque handles per node, calls 004 CreateSceneFromDesc(SceneDesc, ...), returns LevelHandle/SceneRef; UnloadLevel: releases Level handle and calls 004 UnloadScene |
| 2 | Current Scene Get | GetCurrentLevelScene/GetSceneRef: returns current level's SceneRef; upper layer gets SceneRef then calls 004 traversal/query APIs |
| 3 | Delegated Scene Traversal | GetRootNodes(LevelHandle), Traverse(LevelHandle, callback) etc., delegates to 004 |
| 4 | Renderable Collection | CollectRenderables(LevelHandle/SceneRef, callback): traverses scene, calls callback for each Entity with ModelComponent(ISceneNode*, RenderableItem); optional resourceManager overload for model resolution |
| 5 | Level Export/Save | ExportLevelToDesc(LevelHandle, out): exports scene to LevelAssetDesc for Save; SaveLevel(handle, path): exports, creates LevelResource, calls IResourceManager::Save |

## Version / ABI

- Follows Constitution: Public API versioning; breaking changes increment MAJOR.

## Constraints

- Must be used after 004-Scene and 013-Resource initialization. Level loading flow: upper layer calls 029 -> 029 uses 013 Load to get LevelAssetDesc and nodeModelRefs -> 029 converts to 004 SceneDesc and opaque handles -> 029 calls 004 CreateSceneFromDesc -> 029 holds level handle and SceneRef.

## Level File Format Convention

- Level resources support **two file forms**, format selected by 013 loading via 002's **GetFormatFromPath(path)** based on path extension:
  - **Binary**: Path is `*.level` or non-.json/.xml extension, uses Binary format (002 SerializationFormat::Binary).
  - **JSON**: Path ends with `.json` (case-insensitive, e.g. `levels/main.level.json` or `levels/main.json`), uses JSON format (002 SerializationFormat::JSON).
- Same logical level can be saved as `xxx.level` (binary) or `xxx.level.json` (JSON, for version control and manual editing); ResolvePath returns path determines actual format.

## TODO List

(Tasks from `docs/asset/` resource management/loading/storage design.)

- [x] **Description Ownership**: LevelAssetDesc, SceneNodeDesc, ModelAssetDesc, IModelResource owned by 029; LevelAssetDesc, SceneNodeDesc, ModelAssetDesc registered with 002; 013 Level factory registered by 029.
- [x] **CreateLevelFromDesc**: CreateLevelFromDesc(LevelAssetDesc) and CreateLevelFromDesc(ResourceId) (internal 013 LoadSync(Level)); NodeFactoryFn uses EntityManager::CreateEntity and attaches ModelComponent.
- [x] **UnloadLevel**: Destroys all Entities in this Level first, then calls 004 UnloadScene; order convention per plan.
- [x] **Export/Save**: ExportLevelToDesc, SaveLevel for Editor export flow.

## Change Log

| Date | Change |
|------|--------|
| T0 | Scene split into 004-Scene and 029-World; 029 contract |
| 2026-02-05 | Unified directory; capabilities in table format; 029 converts LevelAssetDesc to SceneDesc before calling 004 |
| 2026-02-05 | IModelResource, ModelAssetDesc ownership transferred to 029-World (from 013-Resource) |
| 2026-02-10 | Added 005-Entity dependency; added CollectRenderables capability; added RenderableItem type; 020 renderable source changed to this module's CollectRenderables |
| 2026-02-10 | Implemented LevelAssetDesc/SceneNodeDesc, CreateLevelFromDesc(LevelAssetDesc\|ResourceId), UnloadLevel order convention, CollectRenderables(SceneRef), 002 type registration, 013 Level factory; renderables provided by this module's CollectRenderables |
| 2026-02-10 | Level dual format: supports binary .level and JSON .level.json; format auto-selected by 002 GetFormatFromPath(path) based on extension |
| 2026-02-11 | Added LightComponent, CameraComponent, ReflectionProbeComponent, DecalComponent; WorldManager added CollectLights, CollectCameras, CollectReflectionProbes, CollectDecals(SceneRef, callback) |
| 2026-02-22 | Updated to match actual implementation: RenderableItem now has element, modelResourceId, boundsMin/Max, userData fields; WorldManager has ExportLevelToDesc, SaveLevel; removed CollectLights/Cameras/ReflectionProbes/Decals (not in current implementation) |
