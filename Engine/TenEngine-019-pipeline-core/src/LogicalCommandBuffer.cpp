#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/rendercore/types.hpp>
#include <te/mesh/Mesh.h>
#include <te/mesh/MeshResource.h>
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
      if (!r || !r->mesh) continue;
      LogicalDraw d;
      d.mesh = r->mesh;
      d.material = r->material;
      d.submeshIndex = static_cast<uint32_t>(r->sortKey & 0xFFFFFFFFu);
      d.instanceCount = 1;
      d.firstInstance = 0;
      te::mesh::MeshResource const* meshRes = reinterpret_cast<te::mesh::MeshResource const*>(r->mesh);
      te::mesh::MeshHandle mh = meshRes ? meshRes->GetMeshHandle() : te::mesh::MeshHandle{};
      if (mh) {
        te::mesh::SubmeshDesc const* sub = te::mesh::GetSubmesh(mh, d.submeshIndex);
        if (sub) {
          d.indexCount = sub->count;
          d.firstIndex = sub->offset;
        }
      }
      impl->draws.push_back(d);
    }
    if (!impl->draws.empty()) {
      std::sort(impl->draws.begin(), impl->draws.end(), [](LogicalDraw const& a, LogicalDraw const& b) {
        if (a.material != b.material) return a.material < b.material;
        if (a.mesh != b.mesh) return a.mesh < b.mesh;
        return a.submeshIndex < b.submeshIndex;
      });
      std::vector<LogicalDraw> merged;
      merged.reserve(impl->draws.size());
      for (size_t i = 0; i < impl->draws.size(); ) {
        LogicalDraw d = impl->draws[i];
        size_t j = i + 1;
        while (j < impl->draws.size() &&
               impl->draws[j].material == d.material &&
               impl->draws[j].mesh == d.mesh &&
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
