/**
 * @file Culling.h
 * @brief 020-Pipeline: Frustum culling and visibility determination.
 *
 * Provides:
 * - Frustum culling for objects
 * - LOD selection
 * - Distance-based culling
 * - Occlusion culling interface (future)
 */

#pragma once

#include <te/rendercore/types.hpp>
#include <cstdint>
#include <cstddef>

namespace te::pipelinecore {
struct IRenderItemList;
struct RenderItem;
struct RenderItemBounds;
struct LightItem;
struct ILightItemList;
struct ICameraItemList;
struct CameraItem;
class ISceneWorld;
}

namespace te::pipeline {

/// Frustum plane in Ax + By + Cz + D = 0 form
struct FrustumPlane {
  float a, b, c, d;  // Normal (a,b,c) and distance (d)
};

/// View frustum with 6 planes
struct Frustum {
  FrustumPlane planes[6];  // Left, Right, Top, Bottom, Near, Far

  /// Test if a point is inside the frustum
  bool ContainsPoint(float x, float y, float z) const;

  /// Test if a sphere is inside or intersects the frustum
  bool ContainsSphere(float x, float y, float z, float radius) const;

  /// Test if an AABB is inside or intersects the frustum
  bool ContainsAABB(float minX, float minY, float minZ,
                    float maxX, float maxY, float maxZ) const;
};

/// LOD selection parameters
struct LODParams {
  float lodBias{1.0f};           // LOD distance multiplier
  float lodDistanceFactor{1.0f}; // Additional factor
  uint32_t maxLOD{0};            // Highest LOD index (0 = highest detail)
  bool forceLOD{false};          // If true, use forcedLODIndex
  uint32_t forcedLODIndex{0};    // Forced LOD index
};

/// Culling statistics
struct CullingStats {
  uint32_t totalObjects{0};
  uint32_t visibleObjects{0};
  uint32_t culledObjects{0};
  uint32_t occludedObjects{0};
  uint32_t lodObjects[4] = {0};  // Per-LOD counts
};

// === Frustum Functions ===

/// Build frustum from view-projection matrix
void BuildFrustumFromMatrix(float const* viewProjMatrix, Frustum* outFrustum);

/// Build frustum from camera parameters
void BuildFrustumFromCamera(
  float const* viewMatrix,
  float const* projectionMatrix,
  Frustum* outFrustum);

// === Culling Functions ===

/// Test if a render item is visible in the frustum
bool IsVisibleInFrustum(
  pipelinecore::RenderItem const* item,
  Frustum const& frustum);

/// Test if a light item is visible in the frustum
bool IsLightVisibleInFrustum(
  pipelinecore::LightItem const* light,
  Frustum const& frustum);

/// Perform frustum culling on a render item list
/// Returns number of visible items
uint32_t FrustumCull(
  pipelinecore::IRenderItemList const* input,
  Frustum const& frustum,
  pipelinecore::IRenderItemList* visibleOutput);

/// Perform frustum culling on lights
uint32_t FrustumCullLights(
  pipelinecore::ILightItemList const* input,
  Frustum const& frustum,
  pipelinecore::ILightItemList* visibleOutput);

// === LOD Selection ===

/// Select appropriate LOD based on distance
uint32_t SelectLOD(
  pipelinecore::RenderItem const* item,
  float cameraX, float cameraY, float cameraZ,
  LODParams const& params);

/// Calculate distance from camera to render item
float CalculateDistance(
  pipelinecore::RenderItem const* item,
  float cameraX, float cameraY, float cameraZ);

/// Get LOD distances for a given LOD count
void CalculateLODDistances(
  float baseDistance,
  uint32_t lodCount,
  float* outDistances);

// === Combined Culling ===

/// Perform full culling pipeline (frustum + LOD)
/// Fills visibleOutput with items that pass all tests
void PerformCulling(
  pipelinecore::IRenderItemList const* input,
  Frustum const& frustum,
  float cameraX, float cameraY, float cameraZ,
  LODParams const& lodParams,
  pipelinecore::IRenderItemList* visibleOutput,
  CullingStats* outStats = nullptr);

// === Utility Functions ===

/// Normalize a frustum plane
void NormalizePlane(FrustumPlane* plane);

/// Test AABB against frustum (returns intersection type)
/// 0 = outside, 1 = intersecting, 2 = inside
int TestAABBFrustum(
  float minX, float minY, float minZ,
  float maxX, float maxY, float maxZ,
  Frustum const& frustum);

}  // namespace te::pipeline
