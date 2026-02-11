#include <te/pipelinecore/RenderItem.h>
#include <te/rendercore/types.hpp>
#include <te/material/MaterialResource.h>
#include <te/mesh/MeshResource.h>
#include <te/mesh/MeshDevice.h>
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
                                                  uint32_t subpassCount) {
  if (!device) return te::rendercore::ResultCode::InvalidHandle;
  if (!items) return te::rendercore::ResultCode::Success;
  for (size_t i = 0; i < items->Size(); ++i) {
    RenderItem const* r = items->At(i);
    if (!r) continue;
    if (r->material) PrepareRenderMaterial(r->material, device, renderPass, subpassCount);
    if (r->mesh) PrepareRenderMesh(r->mesh, device);
  }
  return te::rendercore::ResultCode::Success;
}

te::rendercore::ResultCode PrepareRenderMaterial(IMaterialHandle const* material,
                                                 te::rhi::IDevice* device) {
  return PrepareRenderMaterial(material, device, nullptr, 0);
}

te::rendercore::ResultCode PrepareRenderMaterial(IMaterialHandle const* material,
                                                 te::rhi::IDevice* device,
                                                 te::rhi::IRenderPass* renderPass,
                                                 uint32_t subpassCount) {
  if (!device) return te::rendercore::ResultCode::InvalidHandle;
  if (!material) return te::rendercore::ResultCode::Success;
  te::material::MaterialResource* matRes = const_cast<te::material::MaterialResource*>(
      reinterpret_cast<te::material::MaterialResource const*>(material));
  matRes->SetDevice(device);
  matRes->EnsureDeviceResources(renderPass, subpassCount);
  return te::rendercore::ResultCode::Success;
}

te::rendercore::ResultCode PrepareRenderMesh(IMeshHandle const* mesh,
                                             te::rhi::IDevice* device) {
  if (!device) return te::rendercore::ResultCode::InvalidHandle;
  if (!mesh) return te::rendercore::ResultCode::Success;
  te::mesh::MeshResource const* meshRes = reinterpret_cast<te::mesh::MeshResource const*>(mesh);
  te::mesh::MeshHandle mh = meshRes->GetMeshHandle();
  if (mh)
    te::mesh::EnsureDeviceResources(mh, device);
  return te::rendercore::ResultCode::Success;
}

}  // namespace te::pipelinecore
