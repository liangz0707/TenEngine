#include <te/pipelinecore/RenderItem.h>
#include <te/rendercore/types.hpp>
#include <memory>
#include <vector>

namespace te::pipelinecore {

namespace {

class RenderItemListImpl : public IRenderItemList {
 public:
  size_t Size() const override { return items_.size(); }
  RenderItem const* At(size_t i) const override {
    return (i < items_.size()) ? &items_[i] : nullptr;
  }
  void Clear() override { items_.clear(); }
  void Push(RenderItem const& item) override { items_.push_back(item); }

 private:
  std::vector<RenderItem> items_;
};

}  // namespace

RenderItem* CreateRenderItem() { return new RenderItem(); }
IRenderItemList* CreateRenderItemList() { return new RenderItemListImpl(); }
void DestroyRenderItem(RenderItem* r) { delete r; }
void DestroyRenderItemList(IRenderItemList* l) { delete l; }

te::rendercore::ResultCode PrepareRenderResources(IRenderItemList const* /*items*/,
                                                  te::rhi::IDevice* device) {
  if (!device) return te::rendercore::ResultCode::InvalidHandle;
  return te::rendercore::ResultCode::Success;
}

te::rendercore::ResultCode PrepareRenderMaterial(IMaterialHandle const* /*material*/,
                                                 te::rhi::IDevice* device) {
  if (!device) return te::rendercore::ResultCode::InvalidHandle;
  return te::rendercore::ResultCode::Success;
}

te::rendercore::ResultCode PrepareRenderMesh(IMeshHandle const* /*mesh*/,
                                             te::rhi::IDevice* device) {
  if (!device) return te::rendercore::ResultCode::InvalidHandle;
  return te::rendercore::ResultCode::Success;
}

}  // namespace te::pipelinecore
