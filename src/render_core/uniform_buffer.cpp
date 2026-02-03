// 009-RenderCore UniformBuffer (te::rendercore)
// Direct 008-RHI calls: CreateBuffer, UpdateBuffer, SetUniformBuffer, DestroyBuffer

#include <te/rendercore/uniform_buffer.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/command_list.hpp>
#include <cstddef>
#include <new>

namespace te {
namespace rendercore {

constexpr uint32_t kRingBufferSlots = 3;

struct UniformBufferAdapter : public IUniformBuffer {
    IUniformLayout const* layout_ = nullptr;
    te::rhi::IDevice* device_ = nullptr;
    te::rhi::IBuffer* buffer_ = nullptr;
    size_t slotSize_ = 0;
    uint32_t currentSlot_ = 0;

    ~UniformBufferAdapter() override {
        if (device_ && buffer_) {
            device_->DestroyBuffer(buffer_);
            buffer_ = nullptr;
        }
    }

    void Update(void const* src, size_t sz) override {
        if (!buffer_ || !device_ || !src) return;
        size_t copySize = (sz < slotSize_) ? sz : slotSize_;
        size_t offset = currentSlot_ * slotSize_;
        device_->UpdateBuffer(buffer_, offset, src, copySize);
    }

    void Bind(te::rhi::ICommandList* cmd, uint32_t slot) override {
        if (!cmd || !buffer_) return;
        size_t offset = currentSlot_ * slotSize_;
        cmd->SetUniformBuffer(slot, buffer_, offset);
    }

    size_t GetRingBufferOffset(FrameSlotId slot) const override {
        if (slot >= kRingBufferSlots) return 0;
        return slot * slotSize_;
    }

    void SetCurrentFrameSlot(FrameSlotId slot) override {
        if (slot < kRingBufferSlots) currentSlot_ = slot;
    }
};

IUniformBuffer* CreateUniformBuffer(IUniformLayout const* layout, te::rhi::IDevice* device) {
    if (!layout || !device) return nullptr;
    size_t slotSize = layout->GetTotalSize();
    if (slotSize == 0) slotSize = 256;

    te::rhi::BufferDesc desc{};
    desc.size = slotSize * kRingBufferSlots;
    desc.usage = static_cast<uint32_t>(te::rhi::BufferUsage::Uniform);

    te::rhi::IBuffer* buf = device->CreateBuffer(desc);
    if (!buf) return nullptr;

    UniformBufferAdapter* impl = new (std::nothrow) UniformBufferAdapter{};
    if (!impl) {
        device->DestroyBuffer(buf);
        return nullptr;
    }
    impl->layout_ = layout;
    impl->device_ = device;
    impl->buffer_ = buf;
    impl->slotSize_ = slotSize;
    impl->currentSlot_ = 0;
    return impl;
}

void ReleaseUniformBuffer(IUniformBuffer* buffer) {
    delete buffer;
}

} // namespace rendercore
} // namespace te
