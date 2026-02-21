# Contract: 020-Pipeline Module Public API

## Applicable Modules

- **Implementer**: 020-Pipeline (L3; scene collection, culling, DrawCall, command buffer generation, submission; 015-Animation optional)
- **Corresponding Spec**: `docs/module-specs/020-pipeline.md`
- **Dependencies**: 001-Core, 004-Scene, 005-Entity, 019-PipelineCore, 009-RenderCore, 010-Shader, 011-Material, 012-Mesh, 028-Texture, 013-Resource, 021-Effects; 015-Animation (optional)

## Consumers

- 021-Effects, 022-2D, 023-Terrain, 024-Editor, 027-XR

## Capabilities List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| IRenderPipeline | Render pipeline interface; SetDevice, SetSwapChain, SetFrameGraph, RenderFrame, TickPipeline, TriggerRender | Application lifetime |
| RenderingConfig | Rendering configuration; ValidationLevel, RenderPath, VSyncMode, HDRMode, AAMode, ShadowQuality, post-process flags, culling/instancing/multithreading settings | Configuration |
| RenderTarget / FrameStats | Render target description; Frame statistics | Per-frame |
| PipelineContext | Pipeline context; device, swapChain, frameGraph, logicalPipeline, renderItems, lights, transient resources, submit context | Single frame or render |
| PipelineScheduler / SchedulerCallbacks / SchedulerConfig | Multi-threaded pipeline scheduler; phase callbacks; thread counts and configuration | Application lifetime |
| BuiltinMeshes / BuiltinMeshId | Built-in meshes; FullscreenQuad, Sphere, Cone, Cube, Box, Cylinder, Capsule, Plane | Internal cache |
| BuiltinMaterials / BuiltinMaterialId | Built-in materials; PostProcess, Light, Shadow, Debug, Utility types | Internal cache |
| Frustum / FrustumPlane / LODParams / CullingStats | Culling structures; frustum planes, LOD parameters, culling statistics | Per-cull operation |
| ExecutionStats | Execution statistics; drawCalls, instanceCount, triangleCount, vertexCount | Per-execution |
| CollectParams / CollectStats | Collection parameters and statistics | Per-collection |
| SingleThreadQueue | Single-thread task queue; Post tasks to worker thread | Application lifetime |

Collection: Renderables provided by **029-World WorldManager::CollectRenderables** (LevelHandle or SceneRef); callback returns RenderableItem (worldMatrix, modelResource, submeshIndex); 020 does not depend on 004 node modelGuid or 005 GetModelGuid, 029 iterates entities with ModelComponent and fills RenderableItem. Parsed/cached via 013 IModelResource; EnsureDeviceResources triggers 011/012/028 DResource creation; interfaces with 019 PrepareRenderResources. Lights/cameras/reflection probes/decals filled via **CollectLightsToLightItemList**, **CollectCamerasToCameraItemList**, **CollectReflectionProbesToItemList**, **CollectDecalsToItemList** (029 Collect* + 019 ItemList); PassContext SetLightItemList for Pass use. Command buffer and RHI submission see `pipeline-to-rci.md`.

### Capabilities (Provider Guarantees)

| No. | Capability | Description |
|-----|------------|-------------|
| 1 | Culling | CollectVisible, FrustumCull, OcclusionQuery (optional), SelectLOD, PerformCulling; interfaces with Scene/Entity; BuildFrustumFromMatrix/FromCamera, IsVisibleInFrustum, FrustumCull, FrustumCullLights, SelectLOD, CalculateDistance, CalculateLODDistances |
| 2 | Batching | BuildBatches, MaterialSlot, MeshSlot, Transform, Instancing, MergeBatch; parsed via 013 to get Mesh/Material |
| 3 | PassExecution | ExecutePass, GBuffer(PassKind::Scene), Lighting(PassKind::Light), PostProcess; dispatch by PassCollectConfig.passKind (only Scene Pass records logicalCB); interfaces with PipelineCore graph; ExecuteLogicalCommandBufferOnDeviceThread |
| 4 | Submit | BuildCommandBuffer, SubmitToRHI, Present, XRSubmit (optional); interfaces with RHI/SwapChain/XR |
| 5 | Scheduling | PipelineScheduler multi-threaded A/B/C/D phases; SingleThreadQueue for dedicated worker; RenderPhase enum |
| 6 | Builtins | BuiltinMeshes (FullscreenQuad, Sphere, Cone, Cube, etc.); BuiltinMaterials (PostProcess, Light, Shadow, Debug) |

## Version / ABI

- Follows Constitution: Public API versioning; breaking changes increment MAJOR.

## Constraints

- Must be used after all upstream modules are initialized. Submission format and timing with RHI see `pipeline-to-rci.md`; viewport and swap chain interface with Editor/XR must be explicit. Render mode (Debug/Hybrid/Resource) configurable via RenderingConfig.validationLevel; validation uses **CheckWarning(config, msg)**, **CheckError(config, msg)**, **CheckStrict(config, msg, condition)** (te/pipeline/RenderingConfig.h).

## TODO List

(Tasks from `docs/asset/` resource management/loading/storage design.)

- [x] **Resource Parsing**: Renderables provided by 029 CollectRenderables; parsed/cached via 013 IModelResource*.
- [x] **EnsureDeviceResources**: Device task PrepareRenderResources triggers 011/012 Ensure; 019 PrepareRenderResources calls SetDevice+EnsureDeviceResources; LOD streaming after collection calls 013 RequestStreaming(GetResourceId(), 0) on mesh.
- [x] **Data and Flow**: TriggerRender dispatches Phase A to Render thread (BuildLogicalPipeline, CollectRenderablesToRenderItemList, RequestStreaming), Phase B to Device thread (Prepare, IsDeviceReady filter, Convert, per-Pass ExecutePass, record Draw, Submit); CollectRenderablesToRenderItemList supports optional cameraPositionWorld for 012 SelectLOD; before each Draw **matRes->UpdateDescriptorSetForFrame(device, frameSlot)**, 008 **SetGraphicsPSO**, 008 **BindDescriptorSet(matRes->GetDescriptorSet())** (replaces ub->Bind); ExecuteLogicalCommandBufferOnDeviceThread passes frameSlot; 020 records Draw using 012 GetVertexStride/GetIndexFormat; RenderingConfig contains ValidationLevel, CheckWarning/CheckError/CheckStrict report by config.

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 020-Pipeline contract |
| 2026-02-05 | Unified directory; capabilities list as table |
| 2026-02-10 | Renderable source changed to 029 WorldManager::CollectRenderables; removed reference to 004 GetNodeModelGuid; constraints and TODO update: RenderingConfig.validationLevel, CheckWarning/CheckError; TriggerRender dispatch, IsDeviceReady filter, RequestStreaming, SelectLOD, batching, per-Pass ExecutePass implemented |
| 2026-02-10 | Per-draw: UpdateDescriptorSetForFrame(frameSlot), SetGraphicsPSO, BindDescriptorSet; ExecuteLogicalCommandBufferOnDeviceThread passes frameSlot; SubmitLogicalCommandBuffer uses currentSlot |
| 2026-02-11 | BuiltinMeshes (FullscreenQuad, Sphere, Cone), BuiltinMaterials (PostProcess/Light stub); CollectLightsToLightItemList, CollectCamerasToCameraItemList, CollectReflectionProbesToReflectionProbeItemList, CollectDecalsToDecalItemList; RenderPipeline dispatch by PassKind, LightItemList lifecycle; PassContext SetLightItemList |
| 2026-02-22 | Synchronized with code; added PipelineContext, PipelineScheduler, SingleThreadQueue, ExecutionStats, CollectParams/Stats, RenderPhase; updated all type names and function signatures to match implementation |
