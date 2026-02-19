/**
 * @file CollectPass.h
 * @brief Render item collection utilities for frame rendering.
 *
 * Provides functions for collecting, merging, sorting, and culling render items
 * during the collect phase of frame rendering.
 */

#pragma once

#include <te/pipelinecore/FrameContext.h>
#include <te/pipelinecore/LogicalPipeline.h>
#include <te/pipelinecore/RenderItem.h>

namespace te::pipelinecore {

/// Multi-threaded collection of RenderItems from pipeline and scene.
/// Results are written to out (should be pre-cleared).
void CollectRenderItemsParallel(ILogicalPipeline const* pipeline, FrameContext const& ctx,
                                IRenderItemList* out);

/// Merge multiple partial collection results into merged list.
void MergeRenderItems(IRenderItemList const* const* partialLists, size_t count,
                     IRenderItemList* merged);

/// Sort render items by distance from camera (for transparent object rendering).
/// @param list List to sort in-place
/// @param cameraPosition Camera position as 3 floats (x, y, z)
void SortRenderItemsByDistance(IRenderItemList* list, float const* cameraPosition);

/// Frustum cull render items.
/// @param input Input list of items
/// @param frustumPlanes 6 frustum planes * 4 floats each (24 floats total)
/// @param output Output list for visible items
void CullRenderItems(IRenderItemList const* input, 
                     float const* frustumPlanes,
                     IRenderItemList* output);

}  // namespace te::pipelinecore
