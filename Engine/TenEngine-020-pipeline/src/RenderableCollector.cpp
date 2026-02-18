/**
 * @file RenderableCollector.cpp
 * @brief Implementation of RenderableCollector functions.
 */

#include <te/pipeline/detail/RenderableCollector.h>
#include <te/pipeline/Culling.h>

#include <te/pipelinecore/RenderItem.h>

#include <algorithm>
#include <cmath>
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

// === Renderable Collection ===

void CollectRenderablesToRenderItemList(
    CollectParams const& params,
    te::resource::IResourceManager* resourceManager,
    pipelinecore::IRenderItemList* outItems,
    CollectStats* outStats) {

  if (!params.scene || !outItems) return;

  outItems->Clear();

  if (outStats) {
    *outStats = CollectStats{};
  }

  // In a real implementation, this would:
  // 1. Query 029-World for entities with ModelComponent
  // 2. For each entity:
  //    a. Get IModelResource pointer
  //    b. Get world matrix
  //    c. Calculate bounds
  //    d. Perform frustum culling if enabled
  //    e. Select LOD if enabled
  //    f. Create RenderItem and add to list

  // This is a placeholder that demonstrates the interface
  // Actual implementation requires integration with 029-World module

  // Example structure (pseudocode):
  // auto* world = static_cast<WorldManager const*>(params.scene);
  // world->CollectRenderables([](RenderableItem const& ri) {
  //   RenderItem item;
  //   item.element = ri.element;
  //   item.transform = ri.worldMatrix;
  //   item.submeshIndex = ri.submeshIndex;
  //   item.bounds = ri.bounds;
  //   outItems->Push(item);
  // });
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

  if (!scene || !outLights) return;

  outLights->Clear();

  if (outStats) {
    outStats->totalLights = 0;
    outStats->collectedLights = 0;
  }

  // In a real implementation, this would:
  // 1. Query 029-World for entities with LightComponent
  // 2. For each light:
  //    a. Get light type, position, direction, color, intensity, range
  //    b. Perform frustum culling (except directional lights)
  //    c. Create LightItem and add to list

  // Placeholder implementation
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

  if (!scene || !outCameras) return;

  outCameras->Clear();

  if (outStats) {
    outStats->totalCameras = 0;
    outStats->activeCamera = 0;
  }

  // In a real implementation, this would:
  // 1. Query 029-World for entities with CameraComponent
  // 2. For each camera:
  //    a. Get FOV, near, far, aspect ratio
  //    b. Get world matrix
  //    c. Mark active camera
  //    d. Create CameraItem and add to list
}

void* GetActiveCamera(pipelinecore::ISceneWorld const* scene) {
  // In a real implementation, query 029-World for the active camera
  (void)scene;
  return nullptr;
}

// === Reflection Probe Collection ===

void CollectReflectionProbesToItemList(
    pipelinecore::ISceneWorld const* scene,
    Frustum const* frustum,
    pipelinecore::IReflectionProbeItemList* outProbes) {

  if (!scene || !outProbes) return;

  outProbes->Clear();

  // Placeholder - would query 029-World for reflection probe components
}

// === Decal Collection ===

void CollectDecalsToItemList(
    pipelinecore::ISceneWorld const* scene,
    Frustum const* frustum,
    pipelinecore::IDecalItemList* outDecals) {

  if (!scene || !outDecals) return;

  outDecals->Clear();

  // Placeholder - would query 029-World for decal components
}

// === Parallel Collection ===

void CollectRenderablesParallel(
    CollectParams const& params,
    te::resource::IResourceManager* resourceManager,
    uint32_t threadCount,
    pipelinecore::IRenderItemList* outItems,
    CollectStats* outStats) {

  if (!params.scene || !outItems) return;

  outItems->Clear();

  if (threadCount <= 1) {
    // Single-threaded fallback
    CollectRenderablesToRenderItemList(params, resourceManager, outItems, outStats);
    return;
  }

  // Parallel collection strategy:
  // 1. Partition scene into regions
  // 2. Each thread collects from its region
  // 3. Merge results at the end

  std::vector<pipelinecore::IRenderItemList*> partialLists(threadCount);
  std::vector<std::thread> threads(threadCount);

  for (uint32_t i = 0; i < threadCount; ++i) {
    partialLists[i] = pipelinecore::CreateRenderItemList();

    threads[i] = std::thread([&params, resourceManager, &partialLists, i, threadCount]() {
      // In a real implementation, each thread would:
      // 1. Get its partition of the scene
      // 2. Collect renderables from that partition
      // 3. Store in partialLists[i]

      // Placeholder - just use the same collection for now
      (void)i;
      (void)threadCount;
    });
  }

  // Wait for all threads
  for (auto& t : threads) {
    t.join();
  }

  // Merge results
  pipelinecore::MergeRenderItems(
    const_cast<pipelinecore::IRenderItemList const* const*>(partialLists.data()),
    threadCount, outItems);

  // Cleanup partial lists
  for (auto* list : partialLists) {
    pipelinecore::DestroyRenderItemList(list);
  }

  if (outStats) {
    outStats->collectedRenderables = static_cast<uint32_t>(outItems->Size());
  }
}

}  // namespace te::pipeline
