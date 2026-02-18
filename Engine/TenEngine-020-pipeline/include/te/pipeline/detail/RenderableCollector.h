/**
 * @file RenderableCollector.h
 * @brief 020-Pipeline: Collect renderables from scene/world.
 *
 * Provides functions to collect renderable objects from 029-World
 * and convert them to RenderItems for the pipeline.
 */

#pragma once

#include <te/pipelinecore/RenderItem.h>
#include <te/pipelinecore/FrameContext.h>
#include <te/rendercore/types.hpp>
#include <cstdint>
#include <cstddef>

namespace te::resource {
class IResourceManager;
}

namespace te::pipeline {

class ISceneWorld;
struct Frustum;
struct LODParams;

/// Collection parameters
struct CollectParams {
  pipelinecore::ISceneWorld const* scene{nullptr};
  void const* camera{nullptr};          // Camera component
  Frustum const* frustum{nullptr};      // Optional frustum for culling
  LODParams const* lodParams{nullptr};  // Optional LOD parameters
  float cameraPosition[3] = {0, 0, 0};  // For LOD selection
  uint32_t passIndex{0};                // Pass index for filtering
  bool enableCulling{true};             // Enable frustum culling
  bool enableLOD{true};                 // Enable LOD selection
};

/// Collection statistics
struct CollectStats {
  uint32_t totalRenderables{0};
  uint32_t collectedRenderables{0};
  uint32_t culledRenderables{0};
  uint32_t totalLights{0};
  uint32_t collectedLights{0};
  uint32_t totalCameras{0};
  uint32_t activeCamera{0};
};

// === Renderable Collection ===

/// Collect renderables from scene to render item list
void CollectRenderablesToRenderItemList(
  CollectParams const& params,
  te::resource::IResourceManager* resourceManager,
  pipelinecore::IRenderItemList* outItems,
  CollectStats* outStats = nullptr);

/// Collect all renderables without culling (for debug, shadow maps, etc.)
void CollectAllRenderables(
  pipelinecore::ISceneWorld const* scene,
  te::resource::IResourceManager* resourceManager,
  pipelinecore::IRenderItemList* outItems);

// === Light Collection ===

/// Collect lights from scene to light item list
void CollectLightsToLightItemList(
  pipelinecore::ISceneWorld const* scene,
  Frustum const* frustum,
  pipelinecore::ILightItemList* outLights,
  CollectStats* outStats = nullptr);

/// Collect all lights without culling
void CollectAllLights(
  pipelinecore::ISceneWorld const* scene,
  pipelinecore::ILightItemList* outLights);

// === Camera Collection ===

/// Collect cameras from scene to camera item list
void CollectCamerasToCameraItemList(
  pipelinecore::ISceneWorld const* scene,
  pipelinecore::ICameraItemList* outCameras,
  CollectStats* outStats = nullptr);

/// Get the active/main camera from scene
void* GetActiveCamera(pipelinecore::ISceneWorld const* scene);

// === Reflection Probe Collection ===

/// Collect reflection probes from scene
void CollectReflectionProbesToItemList(
  pipelinecore::ISceneWorld const* scene,
  Frustum const* frustum,
  pipelinecore::IReflectionProbeItemList* outProbes);

// === Decal Collection ===

/// Collect decals from scene
void CollectDecalsToItemList(
  pipelinecore::ISceneWorld const* scene,
  Frustum const* frustum,
  pipelinecore::IDecalItemList* outDecals);

// === Parallel Collection ===

/// Collect renderables in parallel (for Thread C)
/// Uses internal partitioning and merges results
void CollectRenderablesParallel(
  CollectParams const& params,
  te::resource::IResourceManager* resourceManager,
  uint32_t threadCount,
  pipelinecore::IRenderItemList* outItems,
  CollectStats* outStats = nullptr);

// === Utility ===

/// Convert world-space bounds to render item bounds
void ConvertToBounds(
  float const* worldMatrix,
  float const* localMin,
  float const* localMax,
  pipelinecore::RenderItemBounds* outBounds);

/// Calculate sort key for render item (for sorting)
uint64_t CalculateSortKey(
  pipelinecore::RenderItem const* item,
  float cameraX, float cameraY, float cameraZ,
  bool sortByDistance = true);

}  // namespace te::pipeline
