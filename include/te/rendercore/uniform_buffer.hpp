#pragma once
// 009-RenderCore UniformBuffer API (te::rendercore)
// ABI: specs/_contracts/009-rendercore-ABI.md
// Direct 008-RHI integration: te::rhi::IDevice*, te::rhi::ICommandList*

#include <te/rendercore/types.hpp>
#include <te/rendercore/uniform_layout.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/command_list.hpp>
#include <cstddef>

namespace te {
namespace rendercore {

class IUniformBuffer {
public:
    virtual ~IUniformBuffer() = default;
    virtual void Update(void const* data, size_t size) = 0;
    virtual void Bind(te::rhi::ICommandList* cmd, uint32_t slot) = 0;
    virtual size_t GetRingBufferOffset(FrameSlotId slot) const = 0;
    virtual void SetCurrentFrameSlot(FrameSlotId slot) = 0;
};

IUniformBuffer* CreateUniformBuffer(IUniformLayout const* layout, te::rhi::IDevice* device);
void ReleaseUniformBuffer(IUniformBuffer* buffer);

} // namespace rendercore
} // namespace te
