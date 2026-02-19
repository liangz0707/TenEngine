/**
 * @file CollectPass.cpp
 * @brief Implementation of CollectPass - parallel render item collection.
 */

#include <te/pipelinecore/CollectPass.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/pipelinecore/LogicalPipeline.h>

#include <cstring>
#include <thread>
#include <vector>

namespace te::pipelinecore {

// Forward declaration - would be implemented in 029-world
// This provides a loose coupling between 019-pipeline-core and 029-world
namespace {
  // Callback type for scene collection
  using SceneCollectCallback = void(*)(void* userData, RenderItem const& item);
  
  // External function pointer that can be set by 020-pipeline
  // This allows 019 to call into 029 without direct dependency
  static void (*g_collectFromScene)(void* sceneData, SceneCollectCallback cb, void* userData) = nullptr;
  
  void SetSceneCollectFunction(void (*fn)(void*, SceneCollectCallback, void*)) {
    g_collectFromScene = fn;
  }
}

// Helper to collect items into a list
struct CollectContext {
  IRenderItemList* list;
  size_t count{0};
};

static void CollectItemCallback(void* userData, RenderItem const& item) {
  auto* ctx = static_cast<CollectContext*>(userData);
  if (ctx && ctx->list) {
    ctx->list->Push(item);
    ctx->count++;
  }
}

void CollectRenderItemsParallel(ILogicalPipeline const* pipeline, FrameContext const& ctx,
                                IRenderItemList* out) {
  if (!pipeline || !out) return;
  out->Clear();

  // Get scene data from pipeline if available
  // The pipeline would have set up scene references during BuildLogicalPipeline
  void* sceneData = nullptr;  // Would come from pipeline or frame context
  
  // If we have a scene collection function registered, use it
  if (g_collectFromScene && sceneData) {
    CollectContext collectCtx{out, 0};
    g_collectFromScene(sceneData, CollectItemCallback, &collectCtx);
  }
  
  // Alternative: iterate through pipeline's built-in render item sources
  // This is a fallback for when scene integration is not set up
  auto* sourceList = pipeline->GetSourceItemList();
  if (sourceList) {
    size_t n = sourceList->Size();
    for (size_t i = 0; i < n; ++i) {
      RenderItem const* r = sourceList->At(i);
      if (r) {
        out->Push(*r);
      }
    }
  }
}

void MergeRenderItems(IRenderItemList const* const* partialLists, size_t count,
                     IRenderItemList* merged) {
  if (!merged) return;
  merged->Clear();
  
  // Calculate total size for pre-allocation
  size_t total = 0;
  for (size_t i = 0; i < count; ++i) {
    if (partialLists[i]) {
      total += partialLists[i]->Size();
    }
  }
  
  // Merge all lists
  for (size_t i = 0; i < count; ++i) {
    if (!partialLists[i]) continue;
    size_t n = partialLists[i]->Size();
    for (size_t j = 0; j < n; ++j) {
      RenderItem const* r = partialLists[i]->At(j);
      if (r) merged->Push(*r);
    }
  }
}

void SortRenderItemsByDistance(IRenderItemList* list, float const* cameraPosition) {
  if (!list || !cameraPosition) return;
  
  size_t n = list->Size();
  if (n <= 1) return;
  
  // Simple bubble sort for now (could use std::sort with iterator support)
  // Sort back-to-front for transparent objects, front-to-back for opaque
  for (size_t i = 0; i < n - 1; ++i) {
    for (size_t j = 0; j < n - i - 1; ++j) {
      RenderItem const* a = list->At(j);
      RenderItem const* b = list->At(j + 1);
      if (!a || !b) continue;
      
      // Calculate distances from camera
      float ax = a->worldMatrix[12] - cameraPosition[0];
      float ay = a->worldMatrix[13] - cameraPosition[1];
      float az = a->worldMatrix[14] - cameraPosition[2];
      float distA = ax * ax + ay * ay + az * az;
      
      float bx = b->worldMatrix[12] - cameraPosition[0];
      float by = b->worldMatrix[13] - cameraPosition[1];
      float bz = b->worldMatrix[14] - cameraPosition[2];
      float distB = bx * bx + by * by + bz * bz;
      
      // Swap if out of order (back to front for transparency)
      if (distA < distB) {
        RenderItem temp = *a;
        list->Set(j, *b);
        list->Set(j + 1, temp);
      }
    }
  }
}

void CullRenderItems(IRenderItemList const* input, 
                     float const* frustumPlanes,  // 6 planes * 4 floats = 24
                     IRenderItemList* output) {
  if (!input || !output) return;
  output->Clear();
  
  if (!frustumPlanes) {
    // No frustum, copy all
    size_t n = input->Size();
    for (size_t i = 0; i < n; ++i) {
      RenderItem const* r = input->At(i);
      if (r) output->Push(*r);
    }
    return;
  }
  
  // Perform frustum culling
  size_t n = input->Size();
  for (size_t i = 0; i < n; ++i) {
    RenderItem const* r = input->At(i);
    if (!r) continue;
    
    // Get bounding sphere from world matrix (approximate)
    float cx = r->worldMatrix[12];
    float cy = r->worldMatrix[13];
    float cz = r->worldMatrix[14];
    
    // Use bounding radius (would come from mesh data in full implementation)
    float radius = 1.0f;  // Default radius
    
    // Test against frustum planes
    bool inside = true;
    for (int p = 0; p < 6; ++p) {
      float const* plane = &frustumPlanes[p * 4];
      float dist = plane[0] * cx + plane[1] * cy + plane[2] * cz + plane[3];
      if (dist < -radius) {
        inside = false;
        break;
      }
    }
    
    if (inside) {
      output->Push(*r);
    }
  }
}

}  // namespace te::pipelinecore
