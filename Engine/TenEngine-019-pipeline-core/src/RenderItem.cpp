#include <te/pipelinecore/RenderItem.h>
#include <te/rendercore/types.hpp>
#include <te/rendercore/IRenderElement.hpp>
#include <te/rendercore/IRenderMaterial.hpp>
#include <te/rendercore/IRenderMesh.hpp>
#include <te/rhi/command_list.hpp>
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
  void Set(size_t i, RenderItem const& item) override {
    if (i < items_.size()) {
      items_[i] = item;
    }
  }

 private:
  std::vector<RenderItem> items_;
};

class LightItemListImpl : public ILightItemList {
 public:
  size_t Size() const override { return items_.size(); }
  LightItem const* At(size_t i) const override {
    return (i < items_.size()) ? &items_[i] : nullptr;
  }
  void Clear() override { items_.clear(); }
  void Push(LightItem const& item) override { items_.push_back(item); }

 private:
  std::vector<LightItem> items_;
};

class CameraItemListImpl : public ICameraItemList {
 public:
  size_t Size() const override { return items_.size(); }
  CameraItem const* At(size_t i) const override {
    return (i < items_.size()) ? &items_[i] : nullptr;
  }
  void Clear() override { items_.clear(); }
  void Push(CameraItem const& item) override { items_.push_back(item); }

 private:
  std::vector<CameraItem> items_;
};

class ReflectionProbeItemListImpl : public IReflectionProbeItemList {
 public:
  size_t Size() const override { return items_.size(); }
  ReflectionProbeItem const* At(size_t i) const override {
    return (i < items_.size()) ? &items_[i] : nullptr;
  }
  void Clear() override { items_.clear(); }
  void Push(ReflectionProbeItem const& item) override { items_.push_back(item); }

 private:
  std::vector<ReflectionProbeItem> items_;
};

class DecalItemListImpl : public IDecalItemList {
 public:
  size_t Size() const override { return items_.size(); }
  DecalItem const* At(size_t i) const override {
    return (i < items_.size()) ? &items_[i] : nullptr;
  }
  void Clear() override { items_.clear(); }
  void Push(DecalItem const& item) override { items_.push_back(item); }

 private:
  std::vector<DecalItem> items_;
};

}  // namespace

RenderItem* CreateRenderItem() { return new RenderItem(); }
void DestroyRenderItem(RenderItem* r) { delete r; }
IRenderItemList* CreateRenderItemList() { return new RenderItemListImpl(); }
void DestroyRenderItemList(IRenderItemList* l) { delete l; }

LightItem* CreateLightItem() { return new LightItem(); }
void DestroyLightItem(LightItem* r) { delete r; }
ILightItemList* CreateLightItemList() { return new LightItemListImpl(); }
void DestroyLightItemList(ILightItemList* l) { delete l; }

CameraItem* CreateCameraItem() { return new CameraItem(); }
void DestroyCameraItem(CameraItem* r) { delete r; }
ICameraItemList* CreateCameraItemList() { return new CameraItemListImpl(); }
void DestroyCameraItemList(ICameraItemList* l) { delete l; }

ReflectionProbeItem* CreateReflectionProbeItem() { return new ReflectionProbeItem(); }
void DestroyReflectionProbeItem(ReflectionProbeItem* r) { delete r; }
IReflectionProbeItemList* CreateReflectionProbeItemList() { return new ReflectionProbeItemListImpl(); }
void DestroyReflectionProbeItemList(IReflectionProbeItemList* l) { delete l; }

DecalItem* CreateDecalItem() { return new DecalItem(); }
void DestroyDecalItem(DecalItem* r) { delete r; }
IDecalItemList* CreateDecalItemList() { return new DecalItemListImpl(); }
void DestroyDecalItemList(IDecalItemList* l) { delete l; }

te::rendercore::ResultCode PrepareRenderResources(IRenderItemList const* items,
                                                  te::rhi::IDevice* device) {
  return PrepareRenderResources(items, device, nullptr, 0);
}

te::rendercore::ResultCode PrepareRenderResources(IRenderItemList const* items,
                                                  te::rhi::IDevice* device,
                                                  te::rhi::IRenderPass* renderPass,
                                                  uint32_t subpassCount,
                                                  te::rhi::IDescriptorSetLayout* skinLayout) {
  return PrepareRenderResources(items, device, renderPass, subpassCount, skinLayout, nullptr);
}

te::rendercore::ResultCode PrepareRenderResources(IRenderItemList const* items,
                                                  te::rhi::IDevice* device,
                                                  te::rhi::IRenderPass* renderPass,
                                                  uint32_t subpassCount,
                                                  te::rhi::IDescriptorSetLayout* skinLayout,
                                                  te::resource::IResourceManager* /*resourceManager*/) {
  if (!device) return te::rendercore::ResultCode::InvalidHandle;
  if (!items) return te::rendercore::ResultCode::Success;
  for (size_t i = 0; i < items->Size(); ++i) {
    RenderItem const* r = items->At(i);
    if (!r || !r->element) continue;
    PrepareRenderElement(r->element, device, renderPass, subpassCount, skinLayout);
  }
  return te::rendercore::ResultCode::Success;
}

te::rendercore::ResultCode PrepareRenderElement(te::rendercore::IRenderElement* element,
                                                te::rhi::IDevice* device) {
  return PrepareRenderElement(element, device, nullptr, 0, nullptr);
}

te::rendercore::ResultCode PrepareRenderElement(te::rendercore::IRenderElement* element,
                                                te::rhi::IDevice* device,
                                                te::rhi::IRenderPass* renderPass,
                                                uint32_t subpassCount,
                                                te::rhi::IDescriptorSetLayout* skinLayout) {
  if (!device) return te::rendercore::ResultCode::InvalidHandle;
  if (!element) return te::rendercore::ResultCode::Success;
  te::rendercore::IRenderMaterial* mat = element->GetMaterial();
  te::rendercore::IRenderMesh* mesh = element->GetMesh();
  if (mat) mat->CreateDeviceResource(renderPass, subpassCount, skinLayout);
  if (mesh) mesh->UpdateDeviceResource(device);
  return te::rendercore::ResultCode::Success;
}

}  // namespace te::pipelinecore
