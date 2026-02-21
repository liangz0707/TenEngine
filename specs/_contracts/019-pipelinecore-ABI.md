# 019-PipelineCore Module ABI

- **Contract**: [019-pipelinecore-public-api.md](./019-pipelinecore-public-api.md) (capabilities and type descriptions)
- **This File**: 019-PipelineCore external ABI explicit table.
- **CMake Target Name**: **`te_pipelinecore`** (consistent with `te_rhi`, `te_rendercore` naming). Downstream should use **`te_pipelinecore`** in `target_link_libraries`. Depends on `te_rhi`, `te_rendercore`.
- **Namespace**: **`te::pipelinecore`** (consistent with `te::rhi`, `te::rendercore` style).
- **Header Path**: **`te/pipelinecore/`** (consistent with `te/rhi/`, `te/rendercore/`).
- **Render Resource Explicit Control Points**: **Create logical render resources** (CreateRenderItem); **Create/collect logical CommandBuffer** (ConvertToLogicalCommandBuffer/CollectCommandBuffer); **Prepare render resources** (PrepareRenderResources, PrepareRenderElement); **Submit to actual GPU Command** see 020-Pipeline/008-RHI; **Create/update GPU resources** see 008-RHI.

## ABI Table

Column definitions: **Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description**

### Configuration

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 019-PipelineCore | te::pipelinecore | — | Constant | Max frames in flight | te/pipelinecore/Config.h | kMaxFramesInFlight | `constexpr uint32_t kMaxFramesInFlight = 4u;` Suggested 2-4; implementation configurable |
| 019-PipelineCore | te::pipelinecore | PipelineConfig | struct | Pipeline configuration | te/pipelinecore/Config.h | PipelineConfig | `struct PipelineConfig { uint32_t frameInFlightCount{2u}; };` Passed when creating Pipeline/SwapChain |
| 019-PipelineCore | te::pipelinecore | FrameSlotId | Type Alias | Frame slot index | te/pipelinecore/Config.h | FrameSlotId | `using FrameSlotId = uint32_t;` Range [0, frameInFlightCount) |

### Frame Context

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 019-PipelineCore | te::pipelinecore | ViewportDesc | struct | Viewport description | te/pipelinecore/FrameContext.h | ViewportDesc | `struct ViewportDesc { uint32_t width{0}; uint32_t height{0}; };` |
| 019-PipelineCore | te::pipelinecore | FrameContext | struct | Frame context | te/pipelinecore/FrameContext.h | FrameContext | `struct FrameContext { ISceneWorld const* scene{nullptr}; void const* camera{nullptr}; ViewportDesc viewport{}; FrameSlotId frameSlotId{0u}; };` 020 constructs and passes to BuildLogicalPipeline, CollectRenderItemsParallel |

### Scene World

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 019-PipelineCore | te::pipelinecore | ISceneWorld | Abstract Interface | Scene world minimal interface | te/pipelinecore/FrameGraph.h | ISceneWorld | `struct ISceneWorld { virtual ~ISceneWorld() = default; };` 020/004 implements; provides visible entity queries, scene root, etc. |

### Frame Graph

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 019-PipelineCore | te::pipelinecore | CullMode | enum | Cull mode | te/pipelinecore/FrameGraph.h | CullMode | `enum class CullMode : uint32_t { None = 0, FrustumCull, OcclusionCull, FrustumAndOcclusion };` |
| 019-PipelineCore | te::pipelinecore | RenderType | enum | Render type | te/pipelinecore/FrameGraph.h | RenderType | `enum class RenderType : uint32_t { Opaque = 0, Transparent, Overlay, Custom };` |
| 019-PipelineCore | te::pipelinecore | PassKind | enum | Pass kind | te/pipelinecore/FrameGraph.h | PassKind | `enum class PassKind : uint32_t { Scene = 0, Light, PostProcess, Effect, Custom };` |
| 019-PipelineCore | te::pipelinecore | PassContentSource | enum | Pass content source | te/pipelinecore/FrameGraph.h | PassContentSource | `enum class PassContentSource : uint32_t { FromModelComponent = 0, FromLightComponent, FromPassDefined, Custom };` |
| 019-PipelineCore | te::pipelinecore | AttachmentLoadOp | enum | Attachment load op | te/pipelinecore/FrameGraph.h | AttachmentLoadOp | `enum class AttachmentLoadOp : uint32_t { LoadOp_Load = 0, Clear = 1, DontCare = 2 };` |
| 019-PipelineCore | te::pipelinecore | AttachmentStoreOp | enum | Attachment store op | te/pipelinecore/FrameGraph.h | AttachmentStoreOp | `enum class AttachmentStoreOp : uint32_t { StoreOp_Store = 0, DontCare = 1 };` |
| 019-PipelineCore | te::pipelinecore | AttachmentLifetime | enum | Attachment lifetime | te/pipelinecore/FrameGraph.h | AttachmentLifetime | `enum class AttachmentLifetime : uint32_t { Transient = 0, Persistent = 1 };` |
| 019-PipelineCore | te::pipelinecore | PassOutputDesc | struct | Pass output description | te/pipelinecore/FrameGraph.h | PassOutputDesc | Width, height, colorAttachmentCount, useDepthStencil, colorFormats[] |
| 019-PipelineCore | te::pipelinecore | PassAttachmentDesc | struct | Pass attachment description | te/pipelinecore/FrameGraph.h | PassAttachmentDesc | handle, width, height, format, isDepthStencil, loadOp, storeOp, lifetime, sourcePassIndex |
| 019-PipelineCore | te::pipelinecore | PassCollectConfig | struct | Pass collect configuration | te/pipelinecore/FrameGraph.h | PassCollectConfig | scene, cullMode, renderType, output, passKind, contentSource, colorAttachments[], depthStencilAttachment, passName, materialName, meshName, readResourceIds[] |
| 019-PipelineCore | te::pipelinecore | IRenderObjectList | Abstract Interface | Render object list | te/pipelinecore/FrameGraph.h | IRenderObjectList | `virtual size_t Size() const = 0;` Read-only list |
| 019-PipelineCore | te::pipelinecore | PassContext | struct | Pass execution context | te/pipelinecore/FrameGraph.h | PassContext | GetCollectedObjects, SetCollectedObjects, GetRenderItemList(slot), GetLightItemList, SetRenderItemList, SetLightItemList |
| 019-PipelineCore | te::pipelinecore | PassExecuteCallback | Callback | Pass execution callback | te/pipelinecore/FrameGraph.h | PassExecuteCallback | `using PassExecuteCallback = void (*)(PassContext& ctx, te::rhi::ICommandList* cmd);` |
| 019-PipelineCore | te::pipelinecore | IPassBuilder | Abstract Interface | Pass builder | te/pipelinecore/FrameGraph.h | IPassBuilder | SetScene, SetCullMode, SetObjectTypeFilter, SetRenderType, SetOutput, SetExecuteCallback, DeclareRead, DeclareWrite, SetPassKind, SetContentSource, GetPassKind, GetContentSource, AddColorAttachment, SetDepthStencilAttachment |
| 019-PipelineCore | te::pipelinecore | IScenePassBuilder | Abstract Interface | Scene pass builder | te/pipelinecore/FrameGraph.h | IScenePassBuilder | Inherits IPassBuilder |
| 019-PipelineCore | te::pipelinecore | ILightPassBuilder | Abstract Interface | Light pass builder | te/pipelinecore/FrameGraph.h | ILightPassBuilder | Inherits IPassBuilder |
| 019-PipelineCore | te::pipelinecore | IPostProcessPassBuilder | Abstract Interface | Post-process pass builder | te/pipelinecore/FrameGraph.h | IPostProcessPassBuilder | SetMaterial, SetMesh, SetFullscreenQuest |
| 019-PipelineCore | te::pipelinecore | IEffectPassBuilder | Abstract Interface | Effect pass builder | te/pipelinecore/FrameGraph.h | IEffectPassBuilder | Inherits IPassBuilder |
| 019-PipelineCore | te::pipelinecore | IFrameGraph | Abstract Interface | Frame graph | te/pipelinecore/FrameGraph.h | IFrameGraph | AddPass(name), AddPass(name, PassKind), Compile, GetPassCount, GetPassCollectConfig, ExecutePass |
| 019-PipelineCore | te::pipelinecore | — | Free Function | Create frame graph | te/pipelinecore/FrameGraph.h | CreateFrameGraph | `IFrameGraph* CreateFrameGraph();` |
| 019-PipelineCore | te::pipelinecore | — | Free Function | Destroy frame graph | te/pipelinecore/FrameGraph.h | DestroyFrameGraph | `void DestroyFrameGraph(IFrameGraph* g);` |

### Logical Pipeline

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 019-PipelineCore | te::pipelinecore | ILogicalPipeline | Abstract Interface | Logical pipeline | te/pipelinecore/LogicalPipeline.h | ILogicalPipeline | `virtual size_t GetPassCount() const = 0;` `virtual void GetPassConfig(size_t index, PassCollectConfig* out) const = 0;` |
| 019-PipelineCore | te::pipelinecore | — | Free Function | Build logical pipeline | te/pipelinecore/LogicalPipeline.h | BuildLogicalPipeline | `ILogicalPipeline* BuildLogicalPipeline(IFrameGraph const* graph, FrameContext const& ctx);` Thread B; produces logical data only |
| 019-PipelineCore | te::pipelinecore | — | Free Function | Destroy logical pipeline | te/pipelinecore/LogicalPipeline.h | DestroyLogicalPipeline | `void DestroyLogicalPipeline(ILogicalPipeline* p);` |

### Render Items

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 019-PipelineCore | te::pipelinecore | RenderItemBounds | struct | Render item bounds | te/pipelinecore/RenderItem.h | RenderItemBounds | `struct RenderItemBounds { float min[3]; float max[3]; };` |
| 019-PipelineCore | te::pipelinecore | RenderItem | struct | Render item | te/pipelinecore/RenderItem.h | RenderItem | `struct RenderItem { IRenderElement* element; uint64_t sortKey; uint32_t submeshIndex; void* transform; RenderItemBounds bounds; void* skinMatrixBuffer; uint32_t skinMatrixOffset; };` |
| 019-PipelineCore | te::pipelinecore | IRenderItemList | Abstract Interface | Render item list | te/pipelinecore/RenderItem.h | IRenderItemList | Size, At, Clear, Push |
| 019-PipelineCore | te::pipelinecore | LightType | enum | Light type | te/pipelinecore/RenderItem.h | LightType | `enum class LightType : uint32_t { Point = 0, Directional, Spot };` |
| 019-PipelineCore | te::pipelinecore | LightItem | struct | Light item | te/pipelinecore/RenderItem.h | LightItem | type, position, direction, color, intensity, range, spotAngle, transform |
| 019-PipelineCore | te::pipelinecore | ILightItemList | Abstract Interface | Light item list | te/pipelinecore/RenderItem.h | ILightItemList | Size, At, Clear, Push |
| 019-PipelineCore | te::pipelinecore | CameraItem | struct | Camera item | te/pipelinecore/RenderItem.h | CameraItem | fovY, nearZ, farZ, isActive, transform |
| 019-PipelineCore | te::pipelinecore | ICameraItemList | Abstract Interface | Camera item list | te/pipelinecore/RenderItem.h | ICameraItemList | Size, At, Clear, Push |
| 019-PipelineCore | te::pipelinecore | ReflectionProbeItemType | enum | Reflection probe type | te/pipelinecore/RenderItem.h | ReflectionProbeItemType | `enum class ReflectionProbeItemType : uint32_t { Box = 0, Sphere };` |
| 019-PipelineCore | te::pipelinecore | ReflectionProbeItem | struct | Reflection probe item | te/pipelinecore/RenderItem.h | ReflectionProbeItem | type, extent, resolution, transform |
| 019-PipelineCore | te::pipelinecore | IReflectionProbeItemList | Abstract Interface | Reflection probe list | te/pipelinecore/RenderItem.h | IReflectionProbeItemList | Size, At, Clear, Push |
| 019-PipelineCore | te::pipelinecore | DecalItem | struct | Decal item | te/pipelinecore/RenderItem.h | DecalItem | albedoTexture, size, blend, transform |
| 019-PipelineCore | te::pipelinecore | IDecalItemList | Abstract Interface | Decal item list | te/pipelinecore/RenderItem.h | IDecalItemList | Size, At, Clear, Push |
| 019-PipelineCore | te::pipelinecore | — | Free Functions | Create/Destroy items | te/pipelinecore/RenderItem.h | CreateRenderItem, DestroyRenderItem, CreateRenderItemList, DestroyRenderItemList, CreateLightItem, DestroyLightItem, CreateLightItemList, DestroyLightItemList, etc. | Factory functions for all item types |
| 019-PipelineCore | te::pipelinecore | — | Free Function | Prepare render resources | te/pipelinecore/RenderItem.h | PrepareRenderResources | Multiple overloads with IDevice, IRenderPass, IDescriptorSetLayout, IResourceManager; **must be called on Thread D** |
| 019-PipelineCore | te::pipelinecore | — | Free Function | Prepare render element | te/pipelinecore/RenderItem.h | PrepareRenderElement | Multiple overloads; **must be called on Thread D** |

### Logical Command Buffer

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 019-PipelineCore | te::pipelinecore | LogicalDraw | struct | Logical draw command | te/pipelinecore/LogicalCommandBuffer.h | LogicalDraw | element, submeshIndex, indexCount, firstIndex, vertexOffset, instanceCount, firstInstance, skinMatrixBuffer, skinMatrixOffset |
| 019-PipelineCore | te::pipelinecore | ILogicalCommandBuffer | Abstract Interface | Logical command buffer | te/pipelinecore/LogicalCommandBuffer.h | ILogicalCommandBuffer | `virtual size_t GetDrawCount() const = 0;` `virtual void GetDraw(size_t index, LogicalDraw* out) const = 0;` |
| 019-PipelineCore | te::pipelinecore | — | Free Function | Convert to logical command buffer | te/pipelinecore/LogicalCommandBuffer.h | ConvertToLogicalCommandBuffer | `ResultCode ConvertToLogicalCommandBuffer(IRenderItemList const* items, ILogicalPipeline const* pipeline, ILogicalCommandBuffer** out);` Alias CollectCommandBuffer; **must be called on Thread D** |
| 019-PipelineCore | te::pipelinecore | — | Free Function | Destroy logical command buffer | te/pipelinecore/LogicalCommandBuffer.h | DestroyLogicalCommandBuffer | `void DestroyLogicalCommandBuffer(ILogicalCommandBuffer* cb);` |

### Collection

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 019-PipelineCore | te::pipelinecore | — | Free Function | Collect render items parallel | te/pipelinecore/CollectPass.h | CollectRenderItemsParallel | `void CollectRenderItemsParallel(ILogicalPipeline const* pipeline, FrameContext const& ctx, IRenderItemList* out);` Thread C; multi-threaded |
| 019-PipelineCore | te::pipelinecore | — | Free Function | Merge render items | te/pipelinecore/CollectPass.h | MergeRenderItems | `void MergeRenderItems(IRenderItemList const* const* partialLists, size_t count, IRenderItemList* merged);` Thread C merge |
| 019-PipelineCore | te::pipelinecore | — | Free Function | Sort render items | te/pipelinecore/CollectPass.h | SortRenderItemsByDistance | `void SortRenderItemsByDistance(IRenderItemList* list, float const* cameraPosition);` For transparent objects |
| 019-PipelineCore | te::pipelinecore | — | Free Function | Cull render items | te/pipelinecore/CollectPass.h | CullRenderItems | `void CullRenderItems(IRenderItemList const* input, float const* frustumPlanes, IRenderItemList* output);` Frustum culling |

### Profiling

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 019-PipelineCore | te::pipelinecore | — | Macro | Profiling switch | te/pipelinecore/Profiling.h | TE_PIPELINECORE_PROFILING | Define to enable profiling; dev build can get pass timing |
| 019-PipelineCore | te::pipelinecore | PassProfilingScope | struct | Pass timing | te/pipelinecore/Profiling.h | PassProfilingScope | RAII, pass start/end timing |
| 019-PipelineCore | te::pipelinecore | OnCompileProfiling | Callback | Compile timing | te/pipelinecore/Profiling.h | OnCompileProfiling | `using OnCompileProfiling = void (*)(uint64_t compileTimeMicros);` |

### Transient Resource Pool

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 019-PipelineCore | te::pipelinecore | TransientResourceType | enum | Transient resource type | te/pipelinecore/ResourceManager.h | TransientResourceType | `enum class TransientResourceType : uint8_t { Texture = 0, Buffer = 1 };` |
| 019-PipelineCore | te::pipelinecore | TransientTextureDesc | struct | Transient texture desc | te/pipelinecore/ResourceManager.h | TransientTextureDesc | width, height, depth, format, mipLevels, arrayLayers, sampleCount, initialState, debugName |
| 019-PipelineCore | te::pipelinecore | TransientBufferDesc | struct | Transient buffer desc | te/pipelinecore/ResourceManager.h | TransientBufferDesc | size, usage, initialState, debugName |
| 019-PipelineCore | te::pipelinecore | TransientResourceHandle | struct | Transient resource handle | te/pipelinecore/ResourceManager.h | TransientResourceHandle | id, type; IsValid, IsTexture, IsBuffer, Invalid |
| 019-PipelineCore | te::pipelinecore | ResourceLifetimeInfo | struct | Resource lifetime info | te/pipelinecore/ResourceManager.h | ResourceLifetimeInfo | handle, firstUsePass, lastUsePass, createPass, releasePass, isUsed |
| 019-PipelineCore | te::pipelinecore | ResourceBarrier | struct | Resource barrier | te/pipelinecore/ResourceManager.h | ResourceBarrier | resource, srcState, dstState, beforePass |
| 019-PipelineCore | te::pipelinecore | TransientResourcePool | class | Transient resource pool | te/pipelinecore/ResourceManager.h | TransientResourcePool | SetDevice, SetCreateCallbacks, BeginFrame, EndFrame, DeclareTransientTexture/Buffer, GetTextureDesc/BufferDesc, MarkResourceRead/Write, ReleaseAfterPass, Compile, GetBarriersForPass, GetAllBarriers, GetOrCreateTexture/Buffer, GetTexture/Buffer, IsResourceCreated, InsertBarriersForPass, GetLifetimeInfo, GetAllocatedTextureCount/BufferSize, GetTotalMemoryUsed, SetDefaultDimensions, CreateTextureFromAttachment |
| 019-PipelineCore | te::pipelinecore | ResourceBarrierBuilder | class | Resource barrier builder | te/pipelinecore/ResourceManager.h | ResourceBarrierBuilder | AddTextureTransition, AddBufferTransition, Build, Clear |
| 019-PipelineCore | te::pipelinecore | — | Free Functions | Create/Destroy pool | te/pipelinecore/ResourceManager.h | CreateTransientResourcePool, DestroyTransientResourcePool, CreateResourceBarrierBuilder, DestroyResourceBarrierBuilder | |

### Submit Context

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 019-PipelineCore | te::pipelinecore | QueueId | enum | Queue type | te/pipelinecore/SubmitContext.h | QueueId | `enum class QueueId : uint8_t { Graphics = 0, Compute = 1, Copy = 2, Count = 3 };` |
| 019-PipelineCore | te::pipelinecore | SyncPrimitiveType | enum | Sync primitive type | te/pipelinecore/SubmitContext.h | SyncPrimitiveType | `enum class SyncPrimitiveType : uint8_t { Fence, Semaphore };` |
| 019-PipelineCore | te::pipelinecore | SyncPoint | class | Sync point | te/pipelinecore/SubmitContext.h | SyncPoint | InitializeAsFence, InitializeAsSemaphore, IsValid, GetFence, GetSemaphore, Wait, Signal, Reset, GetType |
| 019-PipelineCore | te::pipelinecore | QueueSyncPoint | struct | Queue sync point | te/pipelinecore/SubmitContext.h | QueueSyncPoint | queue, waitSemaphore, signalSemaphore, waitValue, signalValue |
| 019-PipelineCore | te::pipelinecore | SubmitBatch | struct | Submit batch | te/pipelinecore/SubmitContext.h | SubmitBatch | queue, commandLists, waitSyncs, signalSyncs, signalFence |
| 019-PipelineCore | te::pipelinecore | SubmitContext | class | Submit context | te/pipelinecore/SubmitContext.h | SubmitContext | SetDevice, GetQueue, GetGraphicsQueue, GetComputeQueue, GetCopyQueue, BeginCommandList, EndCommandList, SubmitQueue, SubmitAll, SubmitBatch, CreateSemaphore, DestroySemaphore, CreateFence, DestroyFence, WaitQueueIdle, WaitAllIdle, GetCurrentFrameFence, WaitForCurrentFrame, AdvanceFrame, GetCurrentFrameIndex, GetFramesInFlight, Reset |
| 019-PipelineCore | te::pipelinecore | MultiQueueScheduler | class | Multi-queue scheduler | te/pipelinecore/SubmitContext.h | MultiQueueScheduler | Initialize, SetQueuePriority, GetQueuePriority, SubmitGraphics, SubmitCompute, SubmitCopy, CreateComputeToGraphicsSync, CreateCopyToGraphicsSync, CreateCopyToComputeSync, Execute, WaitAll, NextFrame, GetPendingWorkCount |
| 019-PipelineCore | te::pipelinecore | — | Free Functions | Create/Destroy | te/pipelinecore/SubmitContext.h | CreateSubmitContext, DestroySubmitContext, CreateMultiQueueScheduler, DestroyMultiQueueScheduler | |

**ResultCode**: Uses `te::rendercore::ResultCode` from 009-RenderCore.

*Source: User stories US-004 (pipeline-style multi-frame rendering), US-rendering-003 (FrameGraph AddPass), US-rendering-004 (multi-thread pipeline phases; Thread D = sole GPU/Device thread, all GPU operations and resource creation must be on Thread D).*

## Change Log

| Date | Change Description |
|------|---------------------|
| 2026-02-10 | ABI sync: IFrameGraph GetPassCount, ExecutePass; PassContext SetCollectedObjects; RenderItem transform, bounds; ConvertToLogicalCommandBuffer sorting and instanced batching |
| 2026-02-11 | FrameGraph extension: PassKind, PassContentSource, PassAttachmentDesc; IFrameGraph AddPass(name, PassKind), GetPassCollectConfig; IPassBuilder SetPassKind/SetContentSource/AddColorAttachment/SetDepthStencilAttachment; derived PassBuilder; PassContext GetRenderItemList(slot), GetLightItemList, SetLightItemList; ILogicalPipeline GetPassConfig; RenderItem.h LightItem, CameraItem, ReflectionProbeItem, DecalItem and Create/Destroy |
| 2026-02-22 | Synchronized with code; added TransientResourcePool, TransientResourceHandle, ResourceBarrierBuilder, ResourceBarrier, ResourceLifetimeInfo; added SubmitContext, SyncPoint, QueueSyncPoint, SubmitBatch, MultiQueueScheduler, QueueId, SyncPrimitiveType; updated all function signatures to match implementation |
