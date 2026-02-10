// 009-RenderCore UniformBuffer Implementation (ABI: te/rendercore/uniform_buffer.hpp)

#include <te/rendercore/uniform_buffer.hpp>
#include <te/rendercore/uniform_layout.hpp>
#include <te/rendercore/resource_desc.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/command_list.hpp>
#include <cstring>
#include <new>
#include <vector>

namespace te {
namespace rendercore {

namespace {
constexpr FrameSlotId kRingBufferSlots = 3;
}

class UniformBufferImpl final : public IUniformBuffer {
 public:
  UniformBufferImpl(IUniformLayout const* layout, te::rhi::IDevice* device)
      : layout_(layout), device_(device) {
    totalSize_ = layout_ ? layout_->GetTotalSize() : 0;
    if (totalSize_ == 0)
      totalSize_ = 256;  // Fallback to minimum block size.

    storage_.resize(static_cast<size_t>(kRingBufferSlots) * totalSize_, 0u);

    if (device_) {
      te::rhi::BufferDesc desc{};
      desc.size = storage_.size();
      desc.usage = static_cast<uint32_t>(te::rhi::BufferUsage::Uniform);
      buffer_ = device_->CreateBuffer(desc);
    }
  }

  ~UniformBufferImpl() override {
    if (device_ && buffer_) {
      device_->DestroyBuffer(buffer_);
    }
  }

  void Update(void const* data, size_t size) override {
    if (!data || size == 0 || storage_.empty())
      return;

    size_t copySize = size > totalSize_ ? totalSize_ : size;
    size_t offset = static_cast<size_t>(currentSlot_) * totalSize_;
    uint8_t* dst = storage_.data() + offset;
    std::memcpy(dst, data, copySize);
    if (device_ && buffer_)
      device_->UpdateBuffer(buffer_, offset, dst, copySize);
  }

  void Bind(te::rhi::ICommandList* cmd, uint32_t slot) override {
    if (!cmd || !buffer_) return;
    size_t offset = GetRingBufferOffset(currentSlot_);
    cmd->SetUniformBuffer(slot, buffer_, offset);
  }

  size_t GetRingBufferOffset(FrameSlotId slot) const override {
    if (slot >= kRingBufferSlots)
      return 0;
    return static_cast<size_t>(slot) * totalSize_;
  }

  void SetCurrentFrameSlot(FrameSlotId slot) override {
    currentSlot_ = slot % kRingBufferSlots;
  }

 private:
  IUniformLayout const* layout_;
  te::rhi::IDevice* device_;
  te::rhi::IBuffer* buffer_ = nullptr;
  std::vector<uint8_t> storage_;
  size_t totalSize_ = 0;
  FrameSlotId currentSlot_ = 0;
};

IUniformBuffer* CreateUniformBuffer(IUniformLayout const* layout, te::rhi::IDevice* device) {
  if (layout == nullptr)
    return nullptr;

  UniformBufferImpl* impl = new (std::nothrow) UniformBufferImpl(layout, device);
  return impl;
}

void ReleaseUniformBuffer(IUniformBuffer* buffer) {
  delete buffer;
}

}  // namespace rendercore
}  // namespace te
