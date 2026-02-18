/**
 * @file RenderableCollector.cpp
 * @brief Implementation of RenderableCollector functions.
 */

#include <te/pipeline/detail/RenderableCollector.h>
#include <te/pipeline/Culling.h>

#include <te/pipelinecore/RenderItem.h>
#include <te/world/WorldManager.h>
#include <te/world/WorldTypes.h>
#include <te/world/ModelComponent.h>
#include <te/scene/SceneWorld.h>
#include <te/scene/ISceneNode.h>
#include <te/entity/Entity.h>
#include <te/rendercore/IRenderElement.hpp>
#include <te/rendercore/IRenderMesh.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <thread>
#include <vector>

namespace te::pipeline {

// === Utility Functions ===

void ConvertToBounds(
    float const* worldMatrix,
    float const* localMin,
    float const* localMax,
    pipelinecore::RenderItemBounds* outBounds) {

  if (!worldMatrix || !localMin || !localMax || !outBounds) return;

  // Transform local bounds to world space
  // Simplified: assumes worldMatrix is 4x4 column-major
  float corners[8][3] = {
    {localMin[0], localMin[1], localMin[2]},
    {localMax[0], localMin[1], localMin[2]},
    {localMin[0], localMax[1], localMin[2]},
    {localMax[0], localMax[1], localMin[2]},
    {localMin[0], localMin[1], localMax[2]},
    {localMax[0], localMin[1], localMax[2]},
    {localMin[0], localMax[1], localMax[2]},
    {localMax[0], localMax[1], localMax[2]},
  };

  float worldMin[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
  float worldMax[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

  for (int i = 0; i < 8; ++i) {
    float x = corners[i][0];
    float y = corners[i][1];
    float z = corners[i][2];

    // Transform point by world matrix
    float wx = worldMatrix[0] * x + worldMatrix[4] * y + worldMatrix[8] * z + worldMatrix[12];
    float wy = worldMatrix[1] * x + worldMatrix[5] * y + worldMatrix[9] * z + worldMatrix[13];
    float wz = worldMatrix[2] * x + worldMatrix[6] * y + worldMatrix[10] * z + worldMatrix[14];

    worldMin[0] = std::min(worldMin[0], wx);
    worldMin[1] = std::min(worldMin[1], wy);
    worldMin[2] = std::min(worldMin[2], wz);
    worldMax[0] = std::max(worldMax[0], wx);
    worldMax[1] = std::max(worldMax[1], wy);
    worldMax[2] = std::max(worldMax[2], wz);
  }

  outBounds->min[0] = worldMin[0];
  outBounds->min[1] = worldMin[1];
  outBounds->min[2] = worldMin[2];
  outBounds->max[0] = worldMax[0];
  outBounds->max[1] = worldMax[1];
  outBounds->max[2] = worldMax[2];
}

uint64_t CalculateSortKey(
    pipelinecore::RenderItem const* item,
    float cameraX, float cameraY, float cameraZ,
    bool sortByDistance) {

  if (!item) return 0;

  uint64_t key = 0;

  if (sortByDistance) {
    float dist = CalculateDistance(item, cameraX, cameraY, cameraZ);
    // Convert distance to 24-bit integer (0-16777215)
    uint32_t distInt = static_cast<uint32_t>(std::min(dist, 16777215.0f));
    key = static_cast<uint64_t>(distInt) << 40;
  }

  // Include material/shader ID for state sorting
  if (item->element) {
    // Use pointer as material ID (simplified)
    uintptr_t ptr = reinterpret_cast<uintptr_t>(item->element);
    key |= (ptr & 0xFFFFFFFFFF); // Lower 40 bits
  }

  return key;
}

// === Helper: Check if bounds intersect frustum ===
static bool BoundsIntersectsFrustum(
    pipelinecore::RenderItemBounds const& bounds,
    Frustum const* frustum) {
  if (!frustum) return true;  // No frustum = no culling

  // Simplified AABB-frustum intersection
  // For each plane, check if AABB is completely behind
  // If not behind any plane, the AABB intersects or is inside
  // This is a placeholder - actual implementation would use proper frustum planes
  return true;
}

// === Renderable Collection ===

void CollectRenderablesToRenderItemList(
    CollectParams const& params,
    te::resource::IResourceManager* resourceManager,
    pipelinecore::IRenderItemList* outItems,
    CollectStats* outStats) {

  if (!outItems) return;

  outItems->Clear();

  if (outStats) {
    *outStats = CollectStats{};
  }

  // Get scene world from params
  // The ISceneWorld interface should provide access to 029 WorldManager
  // For now, we use WorldManager directly

  auto& worldMgr = te::world::WorldManager::GetInstance();
  te::scene::SceneRef sceneRef;

  // Try to get scene ref from params
  // The scene pointer could be the WorldManager's current level scene
  sceneRef = worldMgr.GetCurrentLevelScene();

  if (!sceneRef.IsValid()) {
    return;
  }

  uint32_t totalRenderables = 0;
  uint32_t collectedRenderables = 0;
  uint32_t culledRenderables = 0;

  // Collect renderables from WorldManager
  worldMgr.CollectRenderables(sceneRef, resourceManager,
    [&](te::scene::ISceneNode* node, te::world::RenderableItem const& ri) {
      totalRenderables++;

      // Skip if no element (resource not loaded)
      if (!ri.element) {
        return;
      }

      // Build RenderItem
      pipelinecore::RenderItem item{};
      item.element = ri.element;
      item.submeshIndex = ri.submeshIndex;

      // Copy world matrix pointer
      // Note: The world matrix should remain valid during the frame
      // In a real implementation, we'd copy or cache the matrix
      static thread_local float s_matrix[16];
      std::memcpy(s_matrix, ri.worldMatrix, sizeof(float) * 16);
      item.transform = s_matrix;

      // Copy bounds
      item.bounds.min[0] = ri.boundsMin[0];
      item.bounds.min[1] = ri.boundsMin[1];
      item.bounds.min[2] = ri.boundsMin[2];
      item.bounds.max[0] = ri.boundsMax[0];
      item.bounds.max[1] = ri.boundsMax[1];
      item.bounds.max[2] = ri.boundsMax[2];

      // Frustum culling
      if (params.enableCulling && params.frustum) {
        if (!BoundsIntersectsFrustum(item.bounds, params.frustum)) {
          culledRenderables++;
          return;
        }
      }

      // Calculate sort key
      item.sortKey = CalculateSortKey(
        &item,
        params.cameraPosition[0],
        params.cameraPosition[1],
        params.cameraPosition[2],
        true);

      // Add to output list
      outItems->Push(item);
      collectedRenderables++;
    });

  if (outStats) {
    outStats->totalRenderables = totalRenderables;
    outStats->collectedRenderables = collectedRenderables;
    outStats->culledRenderables = culledRenderables;
  }
}

void CollectAllRenderables(
    pipelinecore::ISceneWorld const* scene,
    te::resource::IResourceManager* resourceManager,
    pipelinecore::IRenderItemList* outItems) {

  CollectParams params{};
  params.scene = scene;
  params.enableCulling = false;
  params.enableLOD = false;

  CollectRenderablesToRenderItemList(params, resourceManager, outItems, nullptr);
}

// === Light Collection ===

void CollectLightsToLightItemList(
    pipelinecore::ISceneWorld const* scene,
    Frustum const* frustum,
    pipelinecore::ILightItemList* outLights,
    CollectStats* outStats) {

  if (!outLights) return;

  outLights->Clear();

  if (outStats) {
    outStats->totalLights = 0;
    outStats->collectedLights = 0;
  }

  // Get scene ref from WorldManager
  auto& worldMgr = te::world::WorldManager::GetInstance();
  te::scene::SceneRef sceneRef = worldMgr.GetCurrentLevelScene();

  if (!sceneRef.IsValid()) {
    return;
  }

  // Collect lights from WorldManager
  // This requires LightComponent integration with WorldManager
  // For now, placeholder implementation
  // TODO: Implement when LightComponent is available in 029-World
}

void CollectAllLights(
    pipelinecore::ISceneWorld const* scene,
    pipelinecore::ILightItemList* outLights) {

  CollectLightsToLightItemList(scene, nullptr, outLights, nullptr);
}

// === Camera Collection ===

void CollectCamerasToCameraItemList(
    pipelinecore::ISceneWorld const* scene,
    pipelinecore::ICameraItemList* outCameras,
    CollectStats* outStats) {

  if (!outCameras) return;

  outCameras->Clear();

  if (outStats) {
    outStats->totalCameras = 0;
    outStats->activeCamera = 0;
  }

  // Get scene ref from WorldManager
  auto& worldMgr = te::world::WorldManager::GetInstance();
  te::scene::SceneRef sceneRef = worldMgr.GetCurrentLevelScene();

  if (!sceneRef.IsValid()) {
    return;
  }

  // Collect cameras from WorldManager
  // This requires CameraComponent integration with WorldManager
  // For now, placeholder implementation
  // TODO: Implement when CameraComponent is available in 029-World
}

void* GetActiveCamera(pipelinecore::ISceneWorld const* scene) {
  // TODO: Implement when CameraComponent is available
  return nullptr;
}

// === Reflection Probe Collection ===

void CollectReflectionProbesToItemList(
    pipelinecore::ISceneWorld const* scene,
    Frustum const* frustum,
    pipelinecore::IReflectionProbeItemList* outProbes) {

  if (!outProbes) return;

  outProbes->Clear();

  // TODO: Implement when ReflectionProbeComponent is available
}

// === Decal Collection ===

void CollectDecalsToItemList(
    pipelinecore::ISceneWorld const* scene,
    Frustum const* frustum,
    pipelinecore::IDecalItemList* outDecals) {

  if (!outDecals) return;

  outDecals->Clear();

  // TODO: Implement when DecalComponent is available
}

// === Parallel Collection ===

void CollectRenderablesParallel(
    CollectParams const& params,
    te::resource::IResourceManager* resourceManager,
    uint32_t threadCount,
    pipelinecore::IRenderItemList* outItems,
    CollectStats* outStats) {

  if (!outItems) return;

  outItems->Clear();

  if (threadCount <= 1) {
    // Single-threaded fallback
    CollectRenderablesToRenderItemList(params, resourceManager, outItems, outStats);
    return;
  }

  // For now, fall back to single-threaded collection
  // Parallel collection requires scene partitioning which is complex
  CollectRenderablesToRenderItemList(params, resourceManager, outItems, outStats);
}

}  // namespace te::pipeline
