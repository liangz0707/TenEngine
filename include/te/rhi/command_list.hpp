/**
 * @file command_list.hpp
 * @brief RHI command list (contract: specs/_contracts/008-rhi-public-api.md section 4).
 */
#ifndef TE_RHI_COMMAND_LIST_HPP
#define TE_RHI_COMMAND_LIST_HPP

#include "te/rhi/types.hpp"
#include "te/rhi/resources.hpp"
#include "te/rhi/raytracing.hpp"
#include <cstddef>
#include <cstdint>

namespace te {
namespace rhi {

struct IQueue;
struct IFence;
struct ISemaphore;

/** Load operation for render pass attachments. */
enum class LoadOp : uint32_t { Load = 0, Clear = 1, DontCare = 2 };

/** Store operation for render pass attachments. */
enum class StoreOp : uint32_t { Store = 0, DontCare = 1 };

/** Render pass description (P2): color/depth attachments, load/store ops. */
constexpr uint32_t kMaxColorAttachments = 8u;
struct RenderPassDesc {
  uint32_t colorAttachmentCount{0};
  ITexture* colorAttachments[kMaxColorAttachments]{};
  ITexture* depthStencilAttachment{nullptr};
  LoadOp colorLoadOp{LoadOp::DontCare};
  StoreOp colorStoreOp{StoreOp::DontCare};
  LoadOp depthLoadOp{LoadOp::DontCare};
  StoreOp depthStoreOp{StoreOp::DontCare};
  float clearColor[4]{0.f, 0.f, 0.f, 1.f};
  float clearDepth{1.f};
  uint32_t clearStencil{0};
};

/** Buffer region for copy (offset, size). */
struct BufferRegion {
  size_t offset{0};
  size_t size{0};
};

/** Texture region for copy (offset, extent, mip, array layer). */
struct TextureRegion {
  uint32_t x{0}, y{0}, z{0};
  uint32_t width{0}, height{0}, depth{1};
  uint32_t mipLevel{0};
  uint32_t arrayLayer{0};
};

/** Viewport (x, y, width, height, minDepth, maxDepth). */
struct Viewport {
  float x{0.f};
  float y{0.f};
  float width{0.f};
  float height{0.f};
  float minDepth{0.f};
  float maxDepth{1.f};
};

/** Scissor rectangle (x, y, width, height). */
struct ScissorRect {
  int32_t x{0};
  int32_t y{0};
  uint32_t width{0};
  uint32_t height{0};
};

/** Command buffer; valid for single recording cycle; allocated by IDevice. */
struct ICommandList {
  virtual ~ICommandList() = default;
  virtual void Begin() = 0;
  virtual void End() = 0;
  virtual void Draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0) = 0;
  virtual void DrawIndexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, int32_t vertex_offset = 0, uint32_t first_instance = 0) = 0;
  virtual void Dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1) = 0;
  virtual void Copy(void const* src, void* dst, size_t size) = 0;
  /** Fine-grained barrier: per-resource + state transition (BufferBarrier/TextureBarrier, srcState/dstState). */
  virtual void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers,
                               uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) = 0;
  virtual void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) = 0;
  virtual void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) = 0;
  virtual void BeginRenderPass(RenderPassDesc const& desc) = 0;
  virtual void EndRenderPass() = 0;
  virtual void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) = 0;
  virtual void CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) = 0;
  virtual void CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) = 0;
  virtual void BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result) = 0;
  virtual void DispatchRays(DispatchRaysDesc const& desc) = 0;
};

void Begin(ICommandList* cmd);
void End(ICommandList* cmd);
void Submit(ICommandList* cmd, IQueue* queue);
void Submit(ICommandList* cmd, IQueue* queue, IFence* signalFence, ISemaphore* waitSem, ISemaphore* signalSem);

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_COMMAND_LIST_HPP
