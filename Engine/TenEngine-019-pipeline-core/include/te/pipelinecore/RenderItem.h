#pragma once

#include <cstddef>
#include <cstdint>

namespace te::rhi {
struct IDevice;
struct IRenderPass;
struct IDescriptorSetLayout;
}

namespace te::rendercore {
enum class ResultCode : uint32_t;
struct IRenderElement;
}

namespace te::resource {
class IResourceManager;
}

namespace te::pipelinecore {

/// 可选世界矩阵与边界，用于按 Pass 与合批
struct RenderItemBounds {
  float min[3]{0.f, 0.f, 0.f};
  float max[3]{0.f, 0.f, 0.f};
};

/// 单条渲染项：仅以 element 为单元；收集时收集 element 或更新/设置 element 内数据
struct RenderItem {
  te::rendercore::IRenderElement* element{nullptr};
  uint64_t sortKey{0u};
  uint32_t submeshIndex{0};
  void* transform{nullptr};  // te::core::Matrix4 const* 或 nullptr
  float worldMatrix[16]{};   // Cached world matrix for sorting/culling
  RenderItemBounds bounds{};
  void* skinMatrixBuffer{nullptr};
  uint32_t skinMatrixOffset{0};
};

/// 合并后的 RenderItem 列表接口
struct IRenderItemList {
  virtual ~IRenderItemList() = default;
  virtual size_t Size() const = 0;
  virtual RenderItem const* At(size_t i) const = 0;
  virtual void Clear() = 0;
  virtual void Push(RenderItem const& item) = 0;
  virtual void Set(size_t i, RenderItem const& item) = 0;
};

/// 灯光类型
enum class LightType : uint32_t {
  Point = 0,
  Directional,
  Spot,
};

/// 单条灯光项；由 CollectLightsToLightItemList 填充
struct LightItem {
  LightType type{LightType::Point};
  float position[3]{0.f, 0.f, 0.f};
  float direction[3]{0.f, -1.f, 0.f};
  float color[3]{1.f, 1.f, 1.f};
  float intensity{1.f};
  float range{10.f};
  float spotAngle{0.f};
  void* transform{nullptr};  // te::core::Matrix4 const* 或 nullptr
};

struct ILightItemList {
  virtual ~ILightItemList() = default;
  virtual size_t Size() const = 0;
  virtual LightItem const* At(size_t i) const = 0;
  virtual void Clear() = 0;
  virtual void Push(LightItem const& item) = 0;
};

LightItem* CreateLightItem();
void DestroyLightItem(LightItem* r);
ILightItemList* CreateLightItemList();
void DestroyLightItemList(ILightItemList* l);

struct CameraItem {
  float fovY{1.0472f};
  float nearZ{0.1f};
  float farZ{1000.f};
  bool isActive{false};
  void* transform{nullptr};
};

struct ICameraItemList {
  virtual ~ICameraItemList() = default;
  virtual size_t Size() const = 0;
  virtual CameraItem const* At(size_t i) const = 0;
  virtual void Clear() = 0;
  virtual void Push(CameraItem const& item) = 0;
};

CameraItem* CreateCameraItem();
void DestroyCameraItem(CameraItem* r);
ICameraItemList* CreateCameraItemList();
void DestroyCameraItemList(ICameraItemList* l);

enum class ReflectionProbeItemType : uint32_t { Box = 0, Sphere };

struct ReflectionProbeItem {
  ReflectionProbeItemType type{ReflectionProbeItemType::Sphere};
  float extent[3]{10.f, 10.f, 10.f};
  uint32_t resolution{128};
  void* transform{nullptr};
};

struct IReflectionProbeItemList {
  virtual ~IReflectionProbeItemList() = default;
  virtual size_t Size() const = 0;
  virtual ReflectionProbeItem const* At(size_t i) const = 0;
  virtual void Clear() = 0;
  virtual void Push(ReflectionProbeItem const& item) = 0;
};

ReflectionProbeItem* CreateReflectionProbeItem();
void DestroyReflectionProbeItem(ReflectionProbeItem* r);
IReflectionProbeItemList* CreateReflectionProbeItemList();
void DestroyReflectionProbeItemList(IReflectionProbeItemList* l);

struct DecalItem {
  void* albedoTexture{nullptr};
  float size[3]{2.f, 2.f, 2.f};
  float blend{1.f};
  void* transform{nullptr};
};

struct IDecalItemList {
  virtual ~IDecalItemList() = default;
  virtual size_t Size() const = 0;
  virtual DecalItem const* At(size_t i) const = 0;
  virtual void Clear() = 0;
  virtual void Push(DecalItem const& item) = 0;
};

DecalItem* CreateDecalItem();
void DestroyDecalItem(DecalItem* r);
IDecalItemList* CreateDecalItemList();
void DestroyDecalItemList(IDecalItemList* l);

RenderItem* CreateRenderItem();
void DestroyRenderItem(RenderItem* r);
IRenderItemList* CreateRenderItemList();
void DestroyRenderItemList(IRenderItemList* l);

te::rendercore::ResultCode PrepareRenderResources(IRenderItemList const* items,
                                                  te::rhi::IDevice* device);
te::rendercore::ResultCode PrepareRenderResources(IRenderItemList const* items,
                                                  te::rhi::IDevice* device,
                                                  te::rhi::IRenderPass* renderPass,
                                                  uint32_t subpassCount,
                                                  te::rhi::IDescriptorSetLayout* skinLayout = nullptr);
te::rendercore::ResultCode PrepareRenderResources(IRenderItemList const* items,
                                                  te::rhi::IDevice* device,
                                                  te::rhi::IRenderPass* renderPass,
                                                  uint32_t subpassCount,
                                                  te::rhi::IDescriptorSetLayout* skinLayout,
                                                  te::resource::IResourceManager* resourceManager);

te::rendercore::ResultCode PrepareRenderElement(te::rendercore::IRenderElement* element,
                                                te::rhi::IDevice* device);
te::rendercore::ResultCode PrepareRenderElement(te::rendercore::IRenderElement* element,
                                                te::rhi::IDevice* device,
                                                te::rhi::IRenderPass* renderPass,
                                                uint32_t subpassCount,
                                                te::rhi::IDescriptorSetLayout* skinLayout = nullptr);

}  // namespace te::pipelinecore
