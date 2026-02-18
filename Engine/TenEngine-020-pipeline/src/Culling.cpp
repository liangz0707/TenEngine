/**
 * @file Culling.cpp
 * @brief Implementation of culling functions.
 */

#include <te/pipeline/Culling.h>

#include <te/pipelinecore/RenderItem.h>

#include <cmath>
#include <algorithm>

namespace te::pipeline {

// === Frustum Implementation ===

bool Frustum::ContainsPoint(float x, float y, float z) const {
  for (int i = 0; i < 6; ++i) {
    if (planes[i].a * x + planes[i].b * y + planes[i].c * z + planes[i].d < 0) {
      return false;
    }
  }
  return true;
}

bool Frustum::ContainsSphere(float x, float y, float z, float radius) const {
  for (int i = 0; i < 6; ++i) {
    float distance = planes[i].a * x + planes[i].b * y + planes[i].c * z + planes[i].d;
    if (distance < -radius) {
      return false;
    }
  }
  return true;
}

bool Frustum::ContainsAABB(float minX, float minY, float minZ,
                           float maxX, float maxY, float maxZ) const {
  return TestAABBFrustum(minX, minY, minZ, maxX, maxY, maxZ, *this) > 0;
}

// === Frustum Functions ===

void NormalizePlane(FrustumPlane* plane) {
  float length = std::sqrt(
    plane->a * plane->a +
    plane->b * plane->b +
    plane->c * plane->c);

  if (length > 0.0001f) {
    float invLength = 1.0f / length;
    plane->a *= invLength;
    plane->b *= invLength;
    plane->c *= invLength;
    plane->d *= invLength;
  }
}

void BuildFrustumFromMatrix(float const* m, Frustum* outFrustum) {
  if (!m || !outFrustum) return;

  // Extract planes from view-projection matrix
  // Left plane
  outFrustum->planes[0].a = m[3] + m[0];
  outFrustum->planes[0].b = m[7] + m[4];
  outFrustum->planes[0].c = m[11] + m[8];
  outFrustum->planes[0].d = m[15] + m[12];

  // Right plane
  outFrustum->planes[1].a = m[3] - m[0];
  outFrustum->planes[1].b = m[7] - m[4];
  outFrustum->planes[1].c = m[11] - m[8];
  outFrustum->planes[1].d = m[15] - m[12];

  // Top plane
  outFrustum->planes[2].a = m[3] - m[1];
  outFrustum->planes[2].b = m[7] - m[5];
  outFrustum->planes[2].c = m[11] - m[9];
  outFrustum->planes[2].d = m[15] - m[13];

  // Bottom plane
  outFrustum->planes[3].a = m[3] + m[1];
  outFrustum->planes[3].b = m[7] + m[5];
  outFrustum->planes[3].c = m[11] + m[9];
  outFrustum->planes[3].d = m[15] + m[13];

  // Near plane
  outFrustum->planes[4].a = m[3] + m[2];
  outFrustum->planes[4].b = m[7] + m[6];
  outFrustum->planes[4].c = m[11] + m[10];
  outFrustum->planes[4].d = m[15] + m[14];

  // Far plane
  outFrustum->planes[5].a = m[3] - m[2];
  outFrustum->planes[5].b = m[7] - m[6];
  outFrustum->planes[5].c = m[11] - m[10];
  outFrustum->planes[5].d = m[15] - m[14];

  // Normalize all planes
  for (int i = 0; i < 6; ++i) {
    NormalizePlane(&outFrustum->planes[i]);
  }
}

void BuildFrustumFromCamera(
    float const* viewMatrix,
    float const* projectionMatrix,
    Frustum* outFrustum) {

  // Multiply view * projection to get view-projection matrix
  float viewProj[16];

  // Assuming column-major matrices
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      float sum = 0.0f;
      for (int k = 0; k < 4; ++k) {
        sum += projectionMatrix[i + k * 4] * viewMatrix[k + j * 4];
      }
      viewProj[i + j * 4] = sum;
    }
  }

  BuildFrustumFromMatrix(viewProj, outFrustum);
}

// === Culling Functions ===

bool IsVisibleInFrustum(
    pipelinecore::RenderItem const* item,
    Frustum const& frustum) {

  if (!item) return false;

  // Get bounds
  float const* min = item->bounds.min;
  float const* max = item->bounds.max;

  // If bounds are invalid, assume visible
  if (min[0] == 0.0f && min[1] == 0.0f && min[2] == 0.0f &&
      max[0] == 0.0f && max[1] == 0.0f && max[2] == 0.0f) {
    return true;
  }

  return frustum.ContainsAABB(min[0], min[1], min[2],
                               max[0], max[1], max[2]);
}

bool IsLightVisibleInFrustum(
    pipelinecore::LightItem const* light,
    Frustum const& frustum) {

  if (!light) return false;

  float radius = light->range;

  // For directional lights, always visible
  if (light->type == pipelinecore::LightType::Directional) {
    return true;
  }

  return frustum.ContainsSphere(
    light->position[0], light->position[1], light->position[2],
    radius);
}

uint32_t FrustumCull(
    pipelinecore::IRenderItemList const* input,
    Frustum const& frustum,
    pipelinecore::IRenderItemList* visibleOutput) {

  if (!input || !visibleOutput) return 0;

  visibleOutput->Clear();

  size_t count = input->Size();
  uint32_t visibleCount = 0;

  for (size_t i = 0; i < count; ++i) {
    auto const* item = input->At(i);
    if (IsVisibleInFrustum(item, frustum)) {
      visibleOutput->Push(*item);
      ++visibleCount;
    }
  }

  return visibleCount;
}

uint32_t FrustumCullLights(
    pipelinecore::ILightItemList const* input,
    Frustum const& frustum,
    pipelinecore::ILightItemList* visibleOutput) {

  if (!input || !visibleOutput) return 0;

  visibleOutput->Clear();

  size_t count = input->Size();
  uint32_t visibleCount = 0;

  for (size_t i = 0; i < count; ++i) {
    auto const* light = input->At(i);
    if (IsLightVisibleInFrustum(light, frustum)) {
      visibleOutput->Push(*light);
      ++visibleCount;
    }
  }

  return visibleCount;
}

// === LOD Selection ===

float CalculateDistance(
    pipelinecore::RenderItem const* item,
    float cameraX, float cameraY, float cameraZ) {

  if (!item || !item->bounds.min) {
    return 0.0f;
  }

  // Calculate center of bounds
  float centerX = (item->bounds.min[0] + item->bounds.max[0]) * 0.5f;
  float centerY = (item->bounds.min[1] + item->bounds.max[1]) * 0.5f;
  float centerZ = (item->bounds.min[2] + item->bounds.max[2]) * 0.5f;

  // Calculate distance
  float dx = centerX - cameraX;
  float dy = centerY - cameraY;
  float dz = centerZ - cameraZ;

  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

uint32_t SelectLOD(
    pipelinecore::RenderItem const* item,
    float cameraX, float cameraY, float cameraZ,
    LODParams const& params) {

  if (params.forceLOD) {
    return params.forcedLODIndex;
  }

  float distance = CalculateDistance(item, cameraX, cameraY, cameraZ);
  distance *= params.lodBias * params.lodDistanceFactor;

  // Simple LOD selection based on distance thresholds
  // LOD 0: 0-50, LOD 1: 50-100, LOD 2: 100-200, LOD 3: 200+
  const float thresholds[] = {50.0f, 100.0f, 200.0f};

  for (uint32_t i = 0; i < 3; ++i) {
    if (distance < thresholds[i]) {
      return i;
    }
  }

  return params.maxLOD;
}

void CalculateLODDistances(
    float baseDistance,
    uint32_t lodCount,
    float* outDistances) {

  if (!outDistances || lodCount == 0) return;

  // Exponential LOD distances
  for (uint32_t i = 0; i < lodCount; ++i) {
    outDistances[i] = baseDistance * std::pow(2.0f, static_cast<float>(i));
  }
}

void PerformCulling(
    pipelinecore::IRenderItemList const* input,
    Frustum const& frustum,
    float cameraX, float cameraY, float cameraZ,
    LODParams const& lodParams,
    pipelinecore::IRenderItemList* visibleOutput,
    CullingStats* outStats) {

  if (!input || !visibleOutput) return;

  visibleOutput->Clear();

  if (outStats) {
    *outStats = CullingStats{};
  }

  size_t count = input->Size();

  for (size_t i = 0; i < count; ++i) {
    auto const* item = input->At(i);

    if (outStats) {
      outStats->totalObjects++;
    }

    // Frustum culling
    if (!IsVisibleInFrustum(item, frustum)) {
      if (outStats) {
        outStats->culledObjects++;
      }
      continue;
    }

    // LOD selection (modify the item's submeshIndex based on LOD)
    // Note: In a real implementation, we'd create a new item with modified submeshIndex
    // For now, just pass through

    if (outStats) {
      outStats->visibleObjects++;
      uint32_t lod = SelectLOD(item, cameraX, cameraY, cameraZ, lodParams);
      if (lod < 4) {
        outStats->lodObjects[lod]++;
      }
    }

    visibleOutput->Push(*item);
  }
}

// === Utility Functions ===

int TestAABBFrustum(
    float minX, float minY, float minZ,
    float maxX, float maxY, float maxZ,
    Frustum const& frustum) {

  int result = 2; // Inside

  for (int i = 0; i < 6; ++i) {
    FrustumPlane const& p = frustum.planes[i];

    // Find the positive vertex (farthest along plane normal)
    float px = (p.a > 0) ? maxX : minX;
    float py = (p.b > 0) ? maxY : minY;
    float pz = (p.c > 0) ? maxZ : minZ;

    // Check if positive vertex is behind plane
    if (p.a * px + p.b * py + p.c * pz + p.d < 0) {
      return 0; // Outside
    }

    // Find the negative vertex
    float nx = (p.a > 0) ? minX : maxX;
    float ny = (p.b > 0) ? minY : maxY;
    float nz = (p.c > 0) ? minZ : maxZ;

    // Check if negative vertex is behind plane
    if (p.a * nx + p.b * ny + p.c * nz + p.d < 0) {
      result = 1; // Intersecting
    }
  }

  return result;
}

}  // namespace te::pipeline
