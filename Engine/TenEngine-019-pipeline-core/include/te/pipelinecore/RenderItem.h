#pragma once

#include <cstddef>
#include <cstdint>

namespace te::rhi {
struct IDevice;
struct IRenderPass;
}

namespace te::rendercore {
enum class ResultCode : uint32_t;
}

namespace te::pipelinecore {

struct IMaterialHandle;  // 011-Material / 020 提供
struct IMeshHandle;      // 012-Mesh / 020 提供

/// 可选世界矩阵与边界，用于按 Pass 与合批
struct RenderItemBounds {
  float min[3]{0.f, 0.f, 0.f};
  float max[3]{0.f, 0.f, 0.f};
};

/// 单条渲染项；场景模型、UI、材质、排序 key 等
struct RenderItem {
  IMeshHandle const* mesh{nullptr};
  IMaterialHandle const* material{nullptr};
  uint64_t sortKey{0u};
  /// 世界矩阵行主序 4x4，可选；020 收集时可从 RenderableItem 填充
  void* transform{nullptr};  // te::core::Matrix4 const* 或 nullptr
  /// 世界空间 AABB，可选；用于按 Pass 与合批
  RenderItemBounds bounds{};
};

/// 合并后的 RenderItem 列表接口
struct IRenderItemList {
  virtual ~IRenderItemList() = default;
  virtual size_t Size() const = 0;
  virtual RenderItem const* At(size_t i) const = 0;
  virtual void Clear() = 0;
  virtual void Push(RenderItem const& item) = 0;
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
  float spotAngle{0.f};  // 聚光锥角等
  void* transform{nullptr};  // te::core::Matrix4 const* 或 nullptr
};

/// 灯光项列表接口
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

/// 单条相机项；由 CollectCamerasToCameraItemList 填充
struct CameraItem {
  float fovY{1.0472f};
  float nearZ{0.1f};
  float farZ{1000.f};
  bool isActive{false};
  void* transform{nullptr};  // te::core::Matrix4 const*
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

/// 反射探针类型（与 029 ReflectionProbeType 对应）
enum class ReflectionProbeItemType : uint32_t {
  Box = 0,
  Sphere,
};

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

/// 贴花项；albedoTexture 由 020 解析 ResourceId 后填入
struct DecalItem {
  void* albedoTexture{nullptr};  // 020: ITexture* 或 IMaterialHandle*
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

/// 必须在线程 D 调用；遇 RHI 失败返回 ResultCode
te::rendercore::ResultCode PrepareRenderResources(IRenderItemList const* items,
                                                  te::rhi::IDevice* device);
te::rendercore::ResultCode PrepareRenderResources(IRenderItemList const* items,
                                                  te::rhi::IDevice* device,
                                                  te::rhi::IRenderPass* renderPass,
                                                  uint32_t subpassCount);
te::rendercore::ResultCode PrepareRenderMaterial(IMaterialHandle const* material,
                                                 te::rhi::IDevice* device);
te::rendercore::ResultCode PrepareRenderMaterial(IMaterialHandle const* material,
                                                 te::rhi::IDevice* device,
                                                 te::rhi::IRenderPass* renderPass,
                                                 uint32_t subpassCount);
te::rendercore::ResultCode PrepareRenderMesh(IMeshHandle const* mesh,
                                             te::rhi::IDevice* device);

}  // namespace te::pipelinecore
