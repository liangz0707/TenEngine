#pragma once

#include <te/pipelinecore/FrameContext.h>
#include <te/pipelinecore/LogicalPipeline.h>
#include <te/pipelinecore/RenderItem.h>

namespace te::pipelinecore {

/// 多线程收集 RenderItem；结果写入 out（需预先 Clear）
void CollectRenderItemsParallel(ILogicalPipeline const* pipeline, FrameContext const& ctx,
                                IRenderItemList* out);

/// 合并多份部分收集结果到 merged
void MergeRenderItems(IRenderItemList const* const* partialLists, size_t count,
                     IRenderItemList* merged);

}  // namespace te::pipelinecore
