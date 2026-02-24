/** @file uniform_buffer.hpp
 *  009-RenderCore ABI: IUniformBuffer, CreateUniformBuffer, ReleaseUniformBuffer.
 */
#pragma once

#include <te/rendercore/types.hpp>
#include <te/rendercore/uniform_layout.hpp>
#include <cstddef>

namespace te {
namespace rhi {
struct IDevice;
struct ICommandList;
struct IBuffer;
}  // namespace rhi
}  // namespace te

namespace te {
namespace rendercore {

struct IUniformBuffer {
  virtual void Update(void const* data, size_t size) = 0;
  virtual void Bind(te::rhi::ICommandList* cmd, uint32_t slot) = 0;
  virtual size_t GetRingBufferOffset(FrameSlotId slot) const = 0;
  virtual void SetCurrentFrameSlot(FrameSlotId slot) = 0;
  /** Underlying RHI buffer for descriptor set updates. */
  virtual te::rhi::IBuffer* GetBuffer() = 0;
  virtual ~IUniformBuffer() = default;
};

IUniformBuffer* CreateUniformBuffer(IUniformLayout const* layout, te::rhi::IDevice* device);

void ReleaseUniformBuffer(IUniformBuffer* buffer);

}  // namespace rendercore
}  // namespace te
