#include <te/pipelinecore/CollectPass.h>
#include <te/pipelinecore/RenderItem.h>

namespace te::pipelinecore {

void CollectRenderItemsParallel(ILogicalPipeline const* pipeline, FrameContext const& /*ctx*/,
                                IRenderItemList* out) {
  if (!pipeline || !out) return;
  out->Clear();
  (void)pipeline;  // 首版：无场景查询，仅清空；020 实现收集逻辑
}

void MergeRenderItems(IRenderItemList const* const* partialLists, size_t count,
                     IRenderItemList* merged) {
  if (!merged) return;
  merged->Clear();
  for (size_t i = 0; i < count; ++i) {
    if (!partialLists[i]) continue;
    size_t n = partialLists[i]->Size();
    for (size_t j = 0; j < n; ++j) {
      RenderItem const* r = partialLists[i]->At(j);
      if (r) merged->Push(*r);
    }
  }
}

}  // namespace te::pipelinecore
