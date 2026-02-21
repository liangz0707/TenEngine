# 020-Pipeline Module ABI

- **Contract**: [020-pipeline-public-api.md](./020-pipeline-public-api.md) (capabilities and type descriptions)
- **This File**: 020-Pipeline external ABI explicit table.
- **Render Mode**: Rendering supports **Debug**, **Hybrid**, **Resource** modes (selectable via compile option or runtime config); use unified CheckWarning/CheckError/CheckStrict macros for error-prone areas, no exception handling.
- **Render Resource Explicit Control Points**: **Create logical render resources** (CreateRenderItem) see 019-PipelineCore; **Create/collect logical CommandBuffer** (CollectCommandBuffer, i.e., convertToLogicalCommandBuffer) see 019-PipelineCore; **Submit to actual GPU Command** (**SubmitCommandBuffer**, i.e., submitLogicalCommandBuffer / IDevice::executeLogicalCommandBuffer) see this module and 008-RHI; **Prepare render resources** (PrepareRenderMaterial, PrepareRenderMesh, prepareRenderResources) see 019-PipelineCore; **Create/update GPU resources** (CreateDeviceResource, UpdateDeviceResource) see 008-RHI.

## ABI Table

Column definitions: **Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description**

### Render Pipeline

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Set device | te/pipeline/RenderPipeline.h | IRenderPipeline::SetDevice | `void SetDevice(rhi::IDevice* device);` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get device | te/pipeline/RenderPipeline.h | IRenderPipeline::GetDevice | `rhi::IDevice* GetDevice() const;` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Set rendering config | te/pipeline/RenderPipeline.h | IRenderPipeline::SetRenderingConfig | `void SetRenderingConfig(RenderingConfig const* config);` Editor下发配置，下帧生效 |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get rendering config | te/pipeline/RenderPipeline.h | IRenderPipeline::GetRenderingConfig | `RenderingConfig const* GetRenderingConfig() const;` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Set swap chain | te/pipeline/RenderPipeline.h | IRenderPipeline::SetSwapChain | `void SetSwapChain(rhi::ISwapChain* swapChain);` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get swap chain | te/pipeline/RenderPipeline.h | IRenderPipeline::GetSwapChain | `rhi::ISwapChain* GetSwapChain() const;` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Set frame graph | te/pipeline/RenderPipeline.h | IRenderPipeline::SetFrameGraph | `void SetFrameGraph(pipelinecore::IFrameGraph* graph);` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get frame graph | te/pipeline/RenderPipeline.h | IRenderPipeline::GetFrameGraph | `pipelinecore::IFrameGraph* GetFrameGraph();` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get logical pipeline | te/pipeline/RenderPipeline.h | IRenderPipeline::GetLogicalPipeline | `pipelinecore::ILogicalPipeline* GetLogicalPipeline();` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Render frame (simple mode) | te/pipeline/RenderPipeline.h | IRenderPipeline::RenderFrame | `void RenderFrame(pipelinecore::FrameContext const& ctx);` Combines all phases in sequence on current thread |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Tick pipeline (multi-threaded) | te/pipeline/RenderPipeline.h | IRenderPipeline::TickPipeline | `bool TickPipeline(pipelinecore::FrameContext const& ctx);` Advances through phases A-D based on timing; returns true if frame completed |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Trigger render | te/pipeline/RenderPipeline.h | IRenderPipeline::TriggerRender | `void TriggerRender(pipelinecore::FrameContext const& ctx);` Non-blocking: queues work for threads B/C/D |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Is frame complete | te/pipeline/RenderPipeline.h | IRenderPipeline::IsFrameComplete | `bool IsFrameComplete() const;` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Wait for frame | te/pipeline/RenderPipeline.h | IRenderPipeline::WaitForFrame | `void WaitForFrame();` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get current slot | te/pipeline/RenderPipeline.h | IRenderPipeline::GetCurrentSlot | `pipelinecore::FrameSlotId GetCurrentSlot() const;` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get frames in flight | te/pipeline/RenderPipeline.h | IRenderPipeline::GetFramesInFlight | `uint32_t GetFramesInFlight() const;` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Set frames in flight | te/pipeline/RenderPipeline.h | IRenderPipeline::SetFramesInFlight | `void SetFramesInFlight(uint32_t count);` 1-4 |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get transient resource pool | te/pipeline/RenderPipeline.h | IRenderPipeline::GetTransientResourcePool | `pipelinecore::TransientResourcePool* GetTransientResourcePool();` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get submit context | te/pipeline/RenderPipeline.h | IRenderPipeline::GetSubmitContext | `pipelinecore::SubmitContext* GetSubmitContext();` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get pipeline context | te/pipeline/RenderPipeline.h | IRenderPipeline::GetPipelineContext | `PipelineContext* GetPipelineContext();` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Submit logical command buffer | te/pipeline/RenderPipeline.h | IRenderPipeline::SubmitLogicalCommandBuffer | `void SubmitLogicalCommandBuffer(pipelinecore::ILogicalCommandBuffer* cb);` **must be called on Thread D** |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Execute pass | te/pipeline/RenderPipeline.h | IRenderPipeline::ExecutePass | `void ExecutePass(size_t passIndex, pipelinecore::IFrameGraph* graph, rhi::ICommandList* cmd);` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get frame index | te/pipeline/RenderPipeline.h | IRenderPipeline::GetFrameIndex | `uint64_t GetFrameIndex() const;` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get average FPS | te/pipeline/RenderPipeline.h | IRenderPipeline::GetAverageFPS | `double GetAverageFPS() const;` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Get frame time | te/pipeline/RenderPipeline.h | IRenderPipeline::GetFrameTime | `double GetFrameTime() const;` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Initialize | te/pipeline/RenderPipeline.h | IRenderPipeline::Initialize | `bool Initialize();` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Shutdown | te/pipeline/RenderPipeline.h | IRenderPipeline::Shutdown | `void Shutdown();` |
| 020-Pipeline | te::pipeline | IRenderPipeline | Abstract Interface | Is initialized | te/pipeline/RenderPipeline.h | IRenderPipeline::IsInitialized | `bool IsInitialized() const;` |
| 020-Pipeline | te::pipeline | RenderPhase | enum | Render phase | te/pipeline/RenderPipeline.h | RenderPhase | `enum class RenderPhase : uint8_t { Idle, GameUpdate, BuildPipeline, Collect, Prepare, Record, Submit, Present };` |
| 020-Pipeline | te::pipeline | PhaseCompleteCallback | Callback | Phase complete callback | te/pipeline/RenderPipeline.h | PhaseCompleteCallback | `using PhaseCompleteCallback = void (*)(RenderPhase phase, void* userData);` |
| 020-Pipeline | te::pipeline | — | Free Function | Create render pipeline | te/pipeline/RenderPipeline.h | CreateRenderPipeline | `IRenderPipeline* CreateRenderPipeline();` |
| 020-Pipeline | te::pipeline | — | Free Function | Destroy render pipeline | te/pipeline/RenderPipeline.h | DestroyRenderPipeline | `void DestroyRenderPipeline(IRenderPipeline* pipeline);` |
| 020-Pipeline | te::pipeline | — | Free Function | Get phase name | te/pipeline/RenderPipeline.h | GetPhaseName | `char const* GetPhaseName(RenderPhase phase);` |
| 020-Pipeline | te::pipeline | — | Free Function | Is GPU phase | te/pipeline/RenderPipeline.h | IsGPUPhase | `bool IsGPUPhase(RenderPhase phase);` |

### Rendering Configuration

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 020-Pipeline | te::pipeline | ValidationLevel | enum | Validation level | te/pipeline/RenderingConfig.h | ValidationLevel | `enum class ValidationLevel : uint8_t { Disabled = 0, Warning = 1, Error = 2, Strict = 3 };` |
| 020-Pipeline | te::pipeline | RenderPath | enum | Render path | te/pipeline/RenderingConfig.h | RenderPath | `enum class RenderPath : uint8_t { Forward = 0, Deferred = 1, ForwardPlus = 2, Custom = 3 };` |
| 020-Pipeline | te::pipeline | VSyncMode | enum | VSync mode | te/pipeline/RenderingConfig.h | VSyncMode | `enum class VSyncMode : uint8_t { Off = 0, On = 1, Adaptive = 2, Mailbox = 3 };` |
| 020-Pipeline | te::pipeline | HDRMode | enum | HDR mode | te/pipeline/RenderingConfig.h | HDRMode | `enum class HDRMode : uint8_t { SDR = 0, HDR10 = 1, scRGB = 2, DolbyVision = 3 };` |
| 020-Pipeline | te::pipeline | AAMode | enum | Anti-aliasing mode | te/pipeline/RenderingConfig.h | AAMode | `enum class AAMode : uint8_t { None, MSAA2x, MSAA4x, MSAA8x, TAA, FXAA, SMAA };` |
| 020-Pipeline | te::pipeline | ShadowQuality | enum | Shadow quality | te/pipeline/RenderingConfig.h | ShadowQuality | `enum class ShadowQuality : uint8_t { Off, Low, Medium, High, Ultra };` |
| 020-Pipeline | te::pipeline | RenderingConfig | struct | Rendering configuration | te/pipeline/RenderingConfig.h | RenderingConfig | validationLevel, renderPath, vsyncMode, hdrMode, targetFrameRate, aaMode, msaaSamples, shadowQuality, shadowMapResolution, maxShadowCascades, shadowDistance, post-process flags, renderScale, dynamicResolution, culling/instancing/multithreading settings, maxFramesInFlight, transientResourcePoolSizeMB, effects flags; GetScaledResolution, IsMSAAEnabled, GetMSAASampleCount |
| 020-Pipeline | te::pipeline | — | Free Functions | Validation helpers | te/pipeline/RenderingConfig.h | CheckWarning, CheckError, CheckStrict | Inline validation functions controlled by ValidationLevel |
| 020-Pipeline | te::pipeline | — | Free Functions | Default configs | te/pipeline/RenderingConfig.h | GetDefaultConfig, GetHighQualityConfig, GetPerformanceConfig | Preset configurations |

### Pipeline Context

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 020-Pipeline | te::pipeline | RenderTarget | struct | Render target | te/pipeline/PipelineContext.h | RenderTarget | colorTarget, depthTarget, width, height, sampleCount, isSwapChain |
| 020-Pipeline | te::pipeline | FrameStats | struct | Frame statistics | te/pipeline/PipelineContext.h | FrameStats | frameIndex, frameTime, cpuTime, gpuTime, drawCallCount, instanceCount, triangleCount, vertexCount, visibleObjectCount, culledObjectCount, batchCount, passCount, resourceCount, memoryUsed |
| 020-Pipeline | te::pipeline | PipelineContext | class | Pipeline context | te/pipeline/PipelineContext.h | PipelineContext | SetDevice, GetDevice, SetRenderingConfig, GetRenderingConfig, SetFrameGraph, GetFrameGraph, SetSwapChain, GetSwapChain, BeginFrame, EndFrame, GetFrameContext, GetFrameSlot, GetFrameIndex, BuildLogicalPipeline, GetLogicalPipeline, CollectVisibleObjects, GetVisibleRenderItems, GetVisibleLights, GetActiveCameras, SetRenderItems, SetLights, PrepareResources, AreResourcesReady, GetTransientResourcePool, BeginCommandList, EndCommandList, ConvertToLogicalCommandBuffer, ExecutePasses, Submit, Present, GetSubmitContext, SetRenderTarget, GetRenderTarget, GetBackBuffer, BuildBatches, GetBatchCount, GetBatch, GetFrameStats, ResetFrameStats, IsValid, GetWidth, GetHeight, ExecutePostProcessPass, GetDepthBuffer |
| 020-Pipeline | te::pipeline | — | Free Functions | Create/Destroy | te/pipeline/PipelineContext.h | CreatePipelineContext, DestroyPipelineContext | |

### Pipeline Scheduler

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 020-Pipeline | te::pipeline | PipelineTask | Type Alias | Task function | te/pipeline/PipelineScheduler.h | PipelineTask | `using PipelineTask = std::function<void()>;` |
| 020-Pipeline | te::pipeline | SchedulerCallbacks | struct | Scheduler callbacks | te/pipeline/PipelineScheduler.h | SchedulerCallbacks | onGameUpdate, onBuildPipeline, onCollectRenderables, onPrepareResources, onRecordCommands, onSubmit, onPresent |
| 020-Pipeline | te::pipeline | SchedulerConfig | struct | Scheduler config | te/pipeline/PipelineScheduler.h | SchedulerConfig | threadCountA, threadCountC, enableThreadD, enableParallelCollect, maxFramesInFlight |
| 020-Pipeline | te::pipeline | PipelineScheduler | class | Pipeline scheduler | te/pipeline/PipelineScheduler.h | PipelineScheduler | Initialize, Shutdown, IsInitialized, SetCallbacks, Tick, StartFrame, WaitForFrame, IsFrameComplete, GetCurrentPhase, AdvancePhase, ExecutePhase, GetThreadCount, PostTask, GetFrameIndex, GetPhaseTime, GetAverageFrameTime |
| 020-Pipeline | te::pipeline | — | Free Functions | Create/Destroy | te/pipeline/PipelineScheduler.h | CreatePipelineScheduler, DestroyPipelineScheduler | |
| 020-Pipeline | te::pipeline | — | Free Functions | Thread helpers | te/pipeline/PipelineScheduler.h | IsMainThread, IsGPUThread, SetCurrentThreadName | |

### Thread Queue

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 020-Pipeline | te::pipeline | SingleThreadQueue | class | Single-thread queue | te/pipeline/ThreadQueue.h | SingleThreadQueue | `void Post(std::function<void()> task);` Thread-safe; executes tasks sequentially on worker thread |

### Culling

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 020-Pipeline | te::pipeline | FrustumPlane | struct | Frustum plane | te/pipeline/Culling.h | FrustumPlane | `struct FrustumPlane { float a, b, c, d; };` Ax + By + Cz + D = 0 |
| 020-Pipeline | te::pipeline | Frustum | struct | Frustum | te/pipeline/Culling.h | Frustum | planes[6]; ContainsPoint, ContainsSphere, ContainsAABB |
| 020-Pipeline | te::pipeline | LODParams | struct | LOD parameters | te/pipeline/Culling.h | LODParams | lodBias, lodDistanceFactor, maxLOD, forceLOD, forcedLODIndex |
| 020-Pipeline | te::pipeline | CullingStats | struct | Culling statistics | te/pipeline/Culling.h | CullingStats | totalObjects, visibleObjects, culledObjects, occludedObjects, lodObjects[4] |
| 020-Pipeline | te::pipeline | — | Free Function | Build frustum from matrix | te/pipeline/Culling.h | BuildFrustumFromMatrix | `void BuildFrustumFromMatrix(float const* viewProjMatrix, Frustum* outFrustum);` |
| 020-Pipeline | te::pipeline | — | Free Function | Build frustum from camera | te/pipeline/Culling.h | BuildFrustumFromCamera | `void BuildFrustumFromCamera(float const* viewMatrix, float const* projectionMatrix, Frustum* outFrustum);` |
| 020-Pipeline | te::pipeline | — | Free Function | Is visible in frustum | te/pipeline/Culling.h | IsVisibleInFrustum | `bool IsVisibleInFrustum(pipelinecore::RenderItem const* item, Frustum const& frustum);` |
| 020-Pipeline | te::pipeline | — | Free Function | Is light visible in frustum | te/pipeline/Culling.h | IsLightVisibleInFrustum | `bool IsLightVisibleInFrustum(pipelinecore::LightItem const* light, Frustum const& frustum);` |
| 020-Pipeline | te::pipeline | — | Free Function | Frustum cull | te/pipeline/Culling.h | FrustumCull | `uint32_t FrustumCull(pipelinecore::IRenderItemList const* input, Frustum const& frustum, pipelinecore::IRenderItemList* visibleOutput);` |
| 020-Pipeline | te::pipeline | — | Free Function | Frustum cull lights | te/pipeline/Culling.h | FrustumCullLights | `uint32_t FrustumCullLights(pipelinecore::ILightItemList const* input, Frustum const& frustum, pipelinecore::ILightItemList* visibleOutput);` |
| 020-Pipeline | te::pipeline | — | Free Function | Select LOD | te/pipeline/Culling.h | SelectLOD | `uint32_t SelectLOD(pipelinecore::RenderItem const* item, float cameraX, float cameraY, float cameraZ, LODParams const& params);` |
| 020-Pipeline | te::pipeline | — | Free Function | Calculate distance | te/pipeline/Culling.h | CalculateDistance | `float CalculateDistance(pipelinecore::RenderItem const* item, float cameraX, float cameraY, float cameraZ);` |
| 020-Pipeline | te::pipeline | — | Free Function | Calculate LOD distances | te/pipeline/Culling.h | CalculateLODDistances | `void CalculateLODDistances(float baseDistance, uint32_t lodCount, float* outDistances);` |
| 020-Pipeline | te::pipeline | — | Free Function | Perform culling | te/pipeline/Culling.h | PerformCulling | `void PerformCulling(pipelinecore::IRenderItemList const* input, Frustum const& frustum, float cameraX, float cameraY, float cameraZ, LODParams const& lodParams, pipelinecore::IRenderItemList* visibleOutput, CullingStats* outStats = nullptr);` |
| 020-Pipeline | te::pipeline | — | Free Function | Normalize plane | te/pipeline/Culling.h | NormalizePlane | `void NormalizePlane(FrustumPlane* plane);` |
| 020-Pipeline | te::pipeline | — | Free Function | Test AABB frustum | te/pipeline/Culling.h | TestAABBFrustum | `int TestAABBFrustum(...);` Returns 0=outside, 1=intersecting, 2=inside |

### Builtin Meshes

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 020-Pipeline | te::pipeline | BuiltinMeshId | enum | Builtin mesh ID | te/pipeline/BuiltinMeshes.h | BuiltinMeshId | FullscreenQuad, Sphere16/32/64, Cone, ConeTruncated, Cube, Box, Cylinder, Capsule, Plane2x2, Plane8x8 |
| 020-Pipeline | te::pipeline | BuiltinMeshParams | struct | Builtin mesh params | te/pipeline/BuiltinMeshes.h | BuiltinMeshParams | id, segments, rings, radius, height, generateNormals/UVs/Tangents |
| 020-Pipeline | te::pipeline | BuiltinMeshes | class | Builtin meshes | te/pipeline/BuiltinMeshes.h | BuiltinMeshes | SetDevice, GetFullscreenQuest, GetSphere, GetCone, GetConeTruncated, GetCube, GetBox, GetCylinder, GetCapsule, GetPlane, GetMesh, static GetFullscreenQuadVertices/Indices, GetSphereVertexCount/IndexCount, GenerateSphere, IsCached, ClearCache, GetMemoryUsed |
| 020-Pipeline | te::pipeline | — | Free Functions | Global access | te/pipeline/BuiltinMeshes.h | GetBuiltinMeshes, InitializeBuiltinMeshes, ShutdownBuiltinMeshes, CreateBuiltinMeshes, DestroyBuiltinMeshes | |

### Builtin Materials

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 020-Pipeline | te::pipeline | BuiltinMaterialId | enum | Builtin material ID | te/pipeline/BuiltinMaterials.h | BuiltinMaterialId | PostProcessCopy/Tonemap/Bloom/BloomDownsample/BloomUpsample/ColorGrading/Vignette/ChromaticAberration/FXAA/TAA, LightPoint/Spot/Directional/Ambient/IBL, ShadowDepthOpaque/AlphaTest/VSM, DebugWireframe/Normals/UVs/Depth/LightComplexity, ClearColor/ClearDepth/Blit |
| 020-Pipeline | te::pipeline | BuiltinMaterialParams | struct | Builtin material params | te/pipeline/BuiltinMaterials.h | BuiltinMaterialParams | id, customShaderPath, renderPassIndex, subpassIndex, alphaBlend, depthTest, depthWrite |
| 020-Pipeline | te::pipeline | BuiltinMaterials | class | Builtin materials | te/pipeline/BuiltinMaterials.h | BuiltinMaterials | SetDevice, GetPostProcessCopy/Tonemap/Bloom/BloomDownsample/BloomUpsample/ColorGrading/Vignette/ChromaticAberration/FXAA/TAA, GetLightPoint/Spot/Directional/Ambient/IBL, GetShadowDepthOpaque/AlphaTest/VSM, GetDebugWireframe/Normals/UVs/Depth/LightComplexity, GetClearColor/ClearDepth/Blit, GetMaterial, GetMaterialByName, WarmupCache, SetShaderSearchPath, IsCached, ClearCache, GetMemoryUsed |
| 020-Pipeline | te::pipeline | — | Free Functions | Global access | te/pipeline/BuiltinMaterials.h | GetBuiltinMaterials, InitializeBuiltinMaterials, ShutdownBuiltinMaterials, CreateBuiltinMaterials, DestroyBuiltinMaterials, GetBuiltinShaderPath, BuiltinShaderExists | |

### Logical Command Buffer Executor

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 020-Pipeline | te::pipeline | ExecutionStats | struct | Execution statistics | te/pipeline/LogicalCommandBufferExecutor.h | ExecutionStats | drawCalls, instanceCount, triangleCount, vertexCount |
| 020-Pipeline | te::pipeline | — | Free Function | Execute on device thread | te/pipeline/LogicalCommandBufferExecutor.h | ExecuteLogicalCommandBufferOnDeviceThread | `void ExecuteLogicalCommandBufferOnDeviceThread(rhi::ICommandList* cmd, pipelinecore::ILogicalCommandBuffer const* logicalCB, uint32_t frameSlot = 0);` **must be called on Thread D** |
| 020-Pipeline | te::pipeline | — | Free Function | Execute with stats | te/pipeline/LogicalCommandBufferExecutor.h | ExecuteLogicalCommandBufferOnDeviceThreadWithStats | `void ExecuteLogicalCommandBufferOnDeviceThreadWithStats(rhi::ICommandList* cmd, pipelinecore::ILogicalCommandBuffer const* logicalCB, uint32_t frameSlot, ExecutionStats* outStats);` |

### Renderable Collector

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 020-Pipeline | te::pipeline | CollectParams | struct | Collect parameters | te/pipeline/detail/RenderableCollector.h | CollectParams | scene, camera, frustum, lodParams, cameraPosition[3], passIndex, enableCulling, enableLOD |
| 020-Pipeline | te::pipeline | CollectStats | struct | Collect statistics | te/pipeline/detail/RenderableCollector.h | CollectStats | totalRenderables, collectedRenderables, culledRenderables, totalLights, collectedLights, totalCameras, activeCamera |
| 020-Pipeline | te::pipeline | — | Free Function | Collect to render item list | te/pipeline/detail/RenderableCollector.h | CollectRenderablesToRenderItemList | `void CollectRenderablesToRenderItemList(CollectParams const& params, te::resource::IResourceManager* resourceManager, pipelinecore::IRenderItemList* outItems, CollectStats* outStats = nullptr);` |
| 020-Pipeline | te::pipeline | — | Free Function | Collect all renderables | te/pipeline/detail/RenderableCollector.h | CollectAllRenderables | `void CollectAllRenderables(pipelinecore::ISceneWorld const* scene, te::resource::IResourceManager* resourceManager, pipelinecore::IRenderItemList* outItems);` |
| 020-Pipeline | te::pipeline | — | Free Function | Collect lights | te/pipeline/detail/RenderableCollector.h | CollectLightsToLightItemList | `void CollectLightsToLightItemList(pipelinecore::ISceneWorld const* scene, Frustum const* frustum, pipelinecore::ILightItemList* outLights, CollectStats* outStats = nullptr);` |
| 020-Pipeline | te::pipeline | — | Free Function | Collect all lights | te/pipeline/detail/RenderableCollector.h | CollectAllLights | |
| 020-Pipeline | te::pipeline | — | Free Function | Collect cameras | te/pipeline/detail/RenderableCollector.h | CollectCamerasToCameraItemList | |
| 020-Pipeline | te::pipeline | — | Free Function | Get active camera | te/pipeline/detail/RenderableCollector.h | GetActiveCamera | |
| 020-Pipeline | te::pipeline | — | Free Function | Collect reflection probes | te/pipeline/detail/RenderableCollector.h | CollectReflectionProbesToItemList | |
| 020-Pipeline | te::pipeline | — | Free Function | Collect decals | te/pipeline/detail/RenderableCollector.h | CollectDecalsToItemList | |
| 020-Pipeline | te::pipeline | — | Free Function | Collect parallel | te/pipeline/detail/RenderableCollector.h | CollectRenderablesParallel | |
| 020-Pipeline | te::pipeline | — | Free Function | Convert to bounds | te/pipeline/detail/RenderableCollector.h | ConvertToBounds | |
| 020-Pipeline | te::pipeline | — | Free Function | Calculate sort key | te/pipeline/detail/RenderableCollector.h | CalculateSortKey | |

*Source: User stories US-002 (single frame rendering), US-004 (pipeline-style multi-frame rendering), US-editor-001 (editor render settings config and save), US-rendering-003 (FrameGraph AddPass), US-rendering-004 (multi-thread pipeline phases).*

---

## Implementation Notes

- **Renderable Source**: Provided by **029-World WorldManager::CollectRenderables**; Pipeline obtains RenderableItem list via this interface, no longer directly uses 004 GetNodeModelGuid / 005 GetModelGuid. Depends on 029-World module.
- **Collection and LOD**: `CollectRenderablesToRenderItemList(sceneRef, resourceManager, out, frustum, cameraPositionWorld)` (te/pipeline/detail/RenderableCollector.h); **cameraPositionWorld** is optional `float const*` (world coordinates), when non-null used for 012 SelectLOD, only generates RenderItem for selected LOD submesh.
- **Data Flow**: Render task after collection calls 013 RequestStreaming(mesh->GetResourceId(), 0) for each mesh; Device task after Prepare filters by IResource::IsDeviceReady, only Converts and records ready items; before each Draw calls **matRes->UpdateDescriptorSetForFrame(rhiDevice, frameSlot)**, 008 **SetGraphicsPSO(matRes->GetGraphicsPSO())**, 008 **BindDescriptorSet(matRes->GetDescriptorSet())** (UB already written to descriptor set, no separate ub->Bind); ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB, **frameSlot**) and SubmitLogicalCommandBuffer pass current frame slot; per Pass calls 008 BeginOcclusionQuery/EndOcclusionQuery (placeholder).
- Data and interface TODOs have been migrated to this module's contract [020-pipeline-public-api.md](./020-pipeline-public-api.md) TODO list; this file only retains ABI table and implementation notes.

## Change Log

| Date | Change Description |
|------|---------------------|
| 2026-02-10 | ABI sync: RenderingConfig added ValidationLevel, CheckWarning/CheckError; TriggerRender changed to Render thread->Device thread dispatch; implementation notes added CollectRenderablesToRenderItemList(cameraPositionWorld), 013/009 data flow, OcclusionQuery placeholder |
| 2026-02-10 | Render pipeline completion: ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB, frameSlot); per-draw UpdateDescriptorSetForFrame, SetGraphicsPSO, BindDescriptorSet; SubmitLogicalCommandBuffer passes currentSlot |
| 2026-02-11 | BuiltinMeshes (te/pipeline/BuiltinMeshes.h), BuiltinMaterials (te/pipeline/BuiltinMaterials.h); RenderableCollector added CollectLightsToLightItemList, CollectCamerasToCameraItemList, CollectReflectionProbesToReflectionProbeItemList, CollectDecalsToDecalItemList; TriggerRender collects LightItemList, PassContext SetLightItemList, per PassKind only Scene Pass records logicalCB, LightItemList lifecycle DestroyLightItemList |
| 2026-02-22 | Synchronized with code; added PipelineContext, PipelineScheduler, SingleThreadQueue, ExecutionStats, CollectParams/Stats, RenderPhase; added full Culling API; updated all function signatures and enum values to match implementation; converted to English |
