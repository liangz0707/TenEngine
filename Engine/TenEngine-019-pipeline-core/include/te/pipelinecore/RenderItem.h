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
