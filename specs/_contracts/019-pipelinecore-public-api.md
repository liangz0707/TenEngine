# Contract: 019-PipelineCore Module Public API

## Applicable Modules

- **Implementer**: 019-PipelineCore (L3; command buffer format, Pass graph protocol (RDG style), RHI submission contract)
- **Corresponding Spec**: `docs/module-specs/019-pipeline-core.md`
- **Dependencies**: 008-RHI, 009-RenderCore

## Consumers

- 020-Pipeline, 021-Effects

## Capabilities List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| PipelineConfig / kMaxFramesInFlight | Pipeline configuration; frameInFlightCount (2-4); max frames in flight suggestion | Static config |
| FrameSlotId | Frame slot index; range [0, frameInFlightCount) | Per-frame |
| FrameContext / ViewportDesc | Frame context; scene, camera, viewport, frameSlotId | Single frame |
| ISceneWorld | Scene world minimal interface; 020/004 implements | Managed by scene |
| IFrameGraph / IPassBuilder | Frame graph entry; AddPass, Compile, GetPassCount, ExecutePass, GetPassCollectConfig | Single graph build cycle |
| PassKind / PassContentSource | Pass type (Scene/Light/PostProcess/Effect/Custom) and content source | Bound to Pass config |
| PassOutputDesc / PassAttachmentDesc | Pass output description; render targets, depth, multi-RT, resolution, format | Bound to Pass |
| PassCollectConfig | Pass collection config; passKind, contentSource, colorAttachments, depthStencilAttachment | Bound to Pass |
| PassContext | Pass execution context; GetCollectedObjects, GetRenderItemList(slot), GetLightItemList, SetRenderItemList, SetLightItemList | Single Pass execution |
| PassExecuteCallback | Pass execution callback; void (*)(PassContext&, ICommandList*) | Callback |
| IRenderObjectList / IRenderItemList | Render object/item list; Size, At, Clear, Push | Single frame or collection |
| RenderItem / RenderItemBounds | Single render item; element, sortKey, submeshIndex, transform, bounds, skinMatrixBuffer/Offset | Single frame |
| LightItem / ILightItemList / LightType | Light item; type, position, direction, color, intensity, range, spotAngle, transform | Single frame |
| CameraItem / ICameraItemList | Camera item; fovY, nearZ, farZ, isActive, transform | Single frame |
| ReflectionProbeItem / IReflectionProbeItemList / ReflectionProbeItemType | Reflection probe item; type, extent, resolution, transform | Single frame |
| DecalItem / IDecalItemList | Decal item; albedoTexture, size, blend, transform | Single frame |
| ILogicalPipeline | Logical pipeline description; GetPassCount, GetPassConfig | From BuildLogicalPipeline |
| ILogicalCommandBuffer / LogicalDraw | Logical command buffer; GetDrawCount, GetDraw; LogicalDraw with element, submesh, index/vertex counts, skinMatrix | Single frame |
| TransientResourcePool / TransientResourceHandle | Transient resource pool; BeginFrame, DeclareTexture/Buffer, Compile, GetOrCreateTexture/Buffer, InsertBarriersForPass | Single frame |
| SubmitContext / SyncPoint / MultiQueueScheduler | Submit context; queue access, command recording, submission, synchronization; QueueId (Graphics/Compute/Copy) | Single submit cycle |

### Capabilities (Provider Guarantees)

| No. | Capability | Description |
|-----|------------|-------------|
| 1 | PassGraph | IFrameGraph AddPass(name), AddPass(name, PassKind); IPassBuilder SetScene, SetCullMode, SetObjectTypeFilter, SetRenderType, SetOutput, SetExecuteCallback, SetPassKind, SetContentSource, AddColorAttachment, SetDepthStencilAttachment, DeclareRead, DeclareWrite; IScenePassBuilder, ILightPassBuilder, IPostProcessPassBuilder, IEffectPassBuilder derived builders; Compile, GetPassCount, GetPassCollectConfig, ExecutePass; RDG style |
| 2 | ResourceLifetime | TransientResourcePool BeginFrame, DeclareTransientTexture/Buffer, MarkResourceRead/Write, Compile, GetOrCreateTexture/Buffer, InsertBarriersForPass, EndFrame; ResourceBarrierBuilder; ResourceLifetimeInfo |
| 3 | CommandFormat | ILogicalCommandBuffer; ConvertToLogicalCommandBuffer/CollectCommandBuffer; LogicalDraw with element, submesh, instance counts; RenderItem, RenderItemBounds |
| 4 | Submit | SubmitContext queue access, BeginCommandList, EndCommandList, SubmitQueue, SubmitAll, SubmitBatch; SyncPoint fence/semaphore; MultiQueueScheduler cross-queue sync; QueueId Graphics/Compute/Copy |
| 5 | Collect | CollectRenderItemsParallel, MergeRenderItems, SortRenderItemsByDistance, CullRenderItems |

## Version / ABI

- Follows Constitution: Public API versioning; breaking changes increment MAJOR.

## Constraints

- Must be used after RHI and RenderCore initialization. Pass resource declarations must be consistent with RenderCore PassProtocol. Resource barriers and lifetimes must not violate RHI requirements.

## TODO List

(Tasks from original ABI data-related TODOs.)

- [x] **Data**: RenderItem contains element, sortKey, submeshIndex; extended transform, bounds (RenderItemBounds); IRenderItemList.
- [x] **Interfaces**: PrepareRenderResources triggers device resource creation on Thread D; ConvertToLogicalCommandBuffer sorts by (material, mesh, submeshIndex) and merges instanced draws; IFrameGraph::GetPassCount, ExecutePass, GetPassCollectConfig; PassContext::SetCollectedObjects, Get/SetRenderItemList, Get/SetLightItemList.
- [x] **Submit**: SubmitContext, SyncPoint, MultiQueueScheduler for multi-queue synchronization.
- [x] **Resources**: TransientResourcePool for RDG-style transient resource management.

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 019-PipelineCore contract |
| 2026-02-05 | Unified directory; capabilities list as table; removed ABI reference |
| 2026-02-10 | TODO update: PrepareRenderMaterial/Mesh, Convert batching, ExecutePass, PassContext implemented |
| 2026-02-11 | FrameGraph extension: PassKind, PassContentSource, PassAttachmentDesc, derived PassBuilder; PassContext multi-slot RenderItemList, LightItemList; Item lists and Create/Destroy |
| 2026-02-22 | Synchronized with code; added TransientResourcePool, SubmitContext, SyncPoint, MultiQueueScheduler, ResourceBarrierBuilder; updated all type names to match implementation |
