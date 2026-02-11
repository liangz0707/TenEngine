/** @file command_list.hpp
 *  008-RHI ABI: ICommandList, Viewport, ScissorRect, RenderPassDesc, Begin, End, Submit.
 */
#pragma once

#include <te/rhi/types.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/descriptor_set.hpp>
#include <te/rhi/raytracing.hpp>
#include <cstdint>
#include <cstddef>

namespace te {
namespace rhi {

enum class LoadOp : uint32_t { Load = 0, Clear = 1, DontCare = 2 };
enum class StoreOp : uint32_t { Store = 0, DontCare = 1 };

constexpr uint32_t kMaxColorAttachments = 8u;
constexpr uint32_t kMaxSubpasses = 8u;

struct Viewport {
  float x, y, width, height, minDepth, maxDepth;
};

struct ScissorRect {
  int32_t  x, y;
  uint32_t width, height;
};

struct RenderPassColorAttachment {
  ITexture* texture;
  uint32_t  format;  // VkFormat / DXGI_FORMAT etc.; 0 = infer from texture if backend supports
  LoadOp    loadOp;
  StoreOp   storeOp;
  float     clearColor[4];
};

struct RenderPassDepthStencilAttachment {
  ITexture* texture;
  uint32_t  format;
  LoadOp    loadOp;
  StoreOp   storeOp;
  float     clearDepth;
  uint32_t  clearStencil;
};

/// Per-subpass attachment references. Indices refer to global attachment list: 0..colorAttachmentCount-1 = colors, then depth (if used) at colorAttachmentCount.
constexpr uint32_t kDepthStencilAttachmentIndexNone = 0xFFFFFFFFu;

struct SubpassDesc {
  uint32_t colorAttachmentCount;
  uint32_t colorAttachmentIndices[kMaxColorAttachments];
  uint32_t depthStencilAttachmentIndex;  // kDepthStencilAttachmentIndexNone if no depth
};

struct RenderPassDesc {
  uint32_t                    colorAttachmentCount;
  RenderPassColorAttachment   colorAttachments[kMaxColorAttachments];
  RenderPassDepthStencilAttachment depthStencilAttachment;
  LoadOp                      colorLoadOp;
  StoreOp                     colorStoreOp;
  LoadOp                      depthLoadOp;
  StoreOp                     depthStoreOp;
  /// When 0, single subpass (current behavior): all color + depth form one subpass. When > 0, subpasses[] define each subpass.
  uint32_t                    subpassCount;
  SubpassDesc                 subpasses[kMaxSubpasses];
};

struct BufferRegion {
  IBuffer* buffer;
  size_t   offset;
  size_t   size;
};

struct TextureRegion {
  ITexture* texture;
  uint32_t  mipLevel;
  uint32_t  arrayLayer;
  uint32_t  x, y, z;
  uint32_t  width, height, depth;
};

struct IRenderPass {
  /// For PSO creation: number of color attachments in this subpass. Return 1 if index out of range.
  virtual uint32_t GetSubpassColorAttachmentCount(uint32_t subpassIndex) const { (void)subpassIndex; return 1u; }
  virtual ~IRenderPass() = default;
};

struct ICommandList {
  virtual void Begin() = 0;
  virtual void End() = 0;
  virtual void Draw(uint32_t vertex_count, uint32_t instance_count = 1,
                    uint32_t first_vertex = 0, uint32_t first_instance = 0) = 0;
  virtual void DrawIndexed(uint32_t index_count, uint32_t instance_count = 1,
                           uint32_t first_index = 0, int32_t vertex_offset = 0,
                           uint32_t first_instance = 0) = 0;
  virtual void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) = 0;
  virtual void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) = 0;
  virtual void SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) = 0;
  virtual void SetVertexBuffer(uint32_t slot, IBuffer* buffer, size_t offset, uint32_t stride) = 0;
  virtual void SetIndexBuffer(IBuffer* buffer, size_t offset, uint32_t indexFormat) = 0;  // 0 = 16bit, 1 = 32bit
  virtual void SetGraphicsPSO(IPSO* pso) = 0;
  virtual void BindDescriptorSet(IDescriptorSet* set) = 0;
  virtual void BeginRenderPass(RenderPassDesc const& desc, IRenderPass* pass = nullptr) = 0;
  virtual void NextSubpass() = 0;
  virtual void EndRenderPass() = 0;
  virtual void BeginOcclusionQuery(uint32_t queryIndex) = 0;
  virtual void EndOcclusionQuery(uint32_t queryIndex) = 0;
  virtual void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst,
                          size_t dstOffset, size_t size) = 0;
  virtual void CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst,
                                   TextureRegion const& dstRegion) = 0;
  virtual void CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion,
                                  IBuffer* dst, size_t dstOffset) = 0;
  virtual void BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc,
                                          IBuffer* scratch, IBuffer* result) = 0;
  virtual void DispatchRays(DispatchRaysDesc const& desc) = 0;
  virtual void Dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1) = 0;
  virtual void Copy(void const* src, void* dst, size_t size) = 0;
  virtual void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers,
                               uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) = 0;
  virtual ~ICommandList() = default;
};

void Begin(ICommandList* cmd);
void End(ICommandList* cmd);
void Submit(ICommandList* cmd, IQueue* queue);
void Submit(ICommandList* cmd, IQueue* queue, IFence* signalFence,
            ISemaphore* waitSem, ISemaphore* signalSem);

}  // namespace rhi
}  // namespace te
