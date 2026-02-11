#include <te/pipelinecore/CollectPass.h>
#include <te/pipelinecore/RenderItem.h>

namespace te::pipelinecore {

void CollectRenderItemsParallel(ILogicalPipeline const* pipeline, FrameContext const& /*ctx*/,
                                IRenderItemList* out) {
  if (!pipeline || !out) return;
  out->Clear();
  (void)pipeline;  // 首版：无场景查询，仅清空；020 负责 029→RenderItem 收集。可扩展：按 ILogicalPipeline 的 Pass 配置从 ISceneWorld 取数并填充 out，与 020 CollectRenderablesToRenderItemList 分工。
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
