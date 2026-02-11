/**
 * @file FrameGraphResources.h
 * @brief 020-Pipeline: Create transient RTs from FrameGraph PassAttachmentDesc and build RenderPassDesc.
 */

#ifndef TE_PIPELINE_DETAIL_FRAME_GRAPH_RESOURCES_H
#define TE_PIPELINE_DETAIL_FRAME_GRAPH_RESOURCES_H

#include <te/pipelinecore/FrameGraph.h>
#include <te/rhi/command_list.hpp>
#include <te/rhi/device.hpp>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>

namespace te {
namespace pipeline {
namespace detail {

/// RHI depth format (VK_FORMAT_D24_UNORM_S8_UINT) when PassAttachmentDesc.format is 0 for depth
constexpr uint32_t kDefaultDepthStencilFormat = 130u;

/// Key for persistent texture cache (id, size, format, depth)
struct FrameGraphResKey {
  uint64_t id{0};
  uint32_t width{0};
  uint32_t height{0};
  uint32_t format{0};
  bool isDepthStencil{false};
  bool operator<(FrameGraphResKey const& o) const {
    if (id != o.id) return id < o.id;
    if (width != o.width) return width < o.width;
    if (height != o.height) return height < o.height;
    if (format != o.format) return format < o.format;
    return isDepthStencil < o.isDepthStencil;
  }
};

/// Cache for persistent attachments (create once, reuse across frames). Clear on viewport change or pipeline destroy.
struct FrameGraphPersistentCache {
  std::map<FrameGraphResKey, te::rhi::ITexture*> cache;
};

/// Result of building FrameGraph resources: RenderPassDesc, IRenderPass, and id->ITexture* for cleanup
struct FrameGraphResourceSet {
  te::rhi::RenderPassDesc rpDesc{};
  te::rhi::IRenderPass* renderPass{nullptr};
  std::unordered_map<uint64_t, te::rhi::ITexture*> idToTexture;
  std::vector<te::rhi::ITexture*> ownedTextures;
};

/**
 * Create textures from graph config and build RenderPassDesc + IRenderPass.
 * Transient: created each frame, destroyed in DestroyFrameGraphResources.
 * Persistent: if persistentCache non-null, created once and stored in cache; not destroyed in DestroyFrameGraphResources.
 * BackBuffer (handle id 0) must be provided; it is not created.
 * On failure returns a set with renderPass=nullptr and empty ownedTextures.
 */
FrameGraphResourceSet BuildFrameGraphResources(
    pipelinecore::IFrameGraph const* graph,
    uint32_t viewportWidth,
    uint32_t viewportHeight,
    te::rhi::IDevice* device,
    te::rhi::ITexture* backBuffer,
    FrameGraphPersistentCache* persistentCache = nullptr);

/** Destroy render pass and all owned transient textures. Does not destroy textures in persistentCache. */
void DestroyFrameGraphResources(FrameGraphResourceSet* set, te::rhi::IDevice* device);

/** Destroy all textures in the persistent cache and clear it. Call on viewport change or pipeline destroy. */
void DestroyFrameGraphPersistentCache(FrameGraphPersistentCache* cache, te::rhi::IDevice* device);

}  // namespace detail
}  // namespace pipeline
}  // namespace te

#endif  // TE_PIPELINE_DETAIL_FRAME_GRAPH_RESOURCES_H
