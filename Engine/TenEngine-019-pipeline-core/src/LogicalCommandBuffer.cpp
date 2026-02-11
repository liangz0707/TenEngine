#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/rendercore/types.hpp>
#include <te/rendercore/IRenderElement.hpp>
#include <te/rendercore/IRenderMesh.hpp>
#include <algorithm>
#include <vector>

namespace te::pipelinecore {

namespace {

class LogicalCommandBufferImpl : public ILogicalCommandBuffer {
 public:
  std::vector<LogicalDraw> draws;

  size_t GetDrawCount() const override { return draws.size(); }
  void GetDraw(size_t index, LogicalDraw* out) const override {
    if (!out || index >= draws.size()) return;
    *out = draws[index];
  }
};

}  // namespace

te::rendercore::ResultCode ConvertToLogicalCommandBuffer(IRenderItemList const* items,
                                                         ILogicalPipeline const* /*pipeline*/,
                                                         ILogicalCommandBuffer** out) {
  if (!out) return te::rendercore::ResultCode::InvalidHandle;
  LogicalCommandBufferImpl* impl = new LogicalCommandBufferImpl();
  if (items) {
    impl->draws.reserve(items->Size());
    for (size_t i = 0; i < items->Size(); ++i) {
      RenderItem const* r = items->At(i);
      if (!r || !r->element) continue;
      LogicalDraw d;
      d.element = r->element;
      d.submeshIndex = r->submeshIndex;
      d.instanceCount = 1;
      d.firstInstance = 0;
      d.skinMatrixBuffer = r->skinMatrixBuffer;
      d.skinMatrixOffset = r->skinMatrixOffset;

      te::rendercore::IRenderMesh const* reMesh = r->element->GetMesh();
      if (reMesh) {
        te::rendercore::SubmeshRange range;
        if (reMesh->GetSubmesh(d.submeshIndex, &range)) {
          d.indexCount = range.indexCount;
          d.firstIndex = range.indexOffset;
        }
      }
      impl->draws.push_back(d);
    }
    if (!impl->draws.empty()) {
      std::sort(impl->draws.begin(), impl->draws.end(), [](LogicalDraw const& a, LogicalDraw const& b) {
        if (a.element != b.element) return a.element < b.element;
        if (a.skinMatrixBuffer != b.skinMatrixBuffer) return a.skinMatrixBuffer < b.skinMatrixBuffer;
        return a.submeshIndex < b.submeshIndex;
      });
      std::vector<LogicalDraw> merged;
      merged.reserve(impl->draws.size());
      for (size_t i = 0; i < impl->draws.size(); ) {
        LogicalDraw d = impl->draws[i];
        size_t j = i + 1;
        while (j < impl->draws.size() &&
               impl->draws[j].element == d.element &&
               impl->draws[j].skinMatrixBuffer == d.skinMatrixBuffer &&
               impl->draws[j].skinMatrixOffset == d.skinMatrixOffset &&
               impl->draws[j].submeshIndex == d.submeshIndex) {
          d.instanceCount += impl->draws[j].instanceCount;
          ++j;
        }
        merged.push_back(d);
        i = j;
      }
      impl->draws = std::move(merged);
    }
  }
  *out = impl;
  return te::rendercore::ResultCode::Success;
}

void DestroyLogicalCommandBuffer(ILogicalCommandBuffer* cb) { delete cb; }

}  // namespace te::pipelinecore
