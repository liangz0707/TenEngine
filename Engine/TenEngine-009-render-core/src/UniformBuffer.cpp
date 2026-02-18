/**
 * @file UniformBuffer.cpp
 * @brief Implementation of IUniformBuffer and IUniformLayout.
 */

#include <te/rendercore/uniform_buffer.hpp>
#include <te/rendercore/uniform_layout.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>

#include <cstring>
#include <memory>
#include <vector>
#include <unordered_map>

namespace te::rendercore {

// === Uniform Layout Implementation ===

class UniformLayoutImpl : public IUniformLayout {
public:
  std::vector<UniformMember> members;
  std::unordered_map<std::string, size_t> nameToIndex;
  size_t totalSize{0};

  size_t GetOffset(char const* name) const override {
    if (!name) return 0;
    auto it = nameToIndex.find(name);
    if (it != nameToIndex.end() && it->second < members.size()) {
      return members[it->second].offset;
    }
    return 0;
  }

  size_t GetTotalSize() const override {
    return totalSize;
  }
};

// std140 alignment rules
static uint32_t GetStd140Alignment(UniformMemberType type) {
  switch (type) {
    case UniformMemberType::Float: return 4;
    case UniformMemberType::Float2: return 8;
    case UniformMemberType::Float3: return 16; // vec3 aligned to vec4
    case UniformMemberType::Float4: return 16;
    case UniformMemberType::Mat3: return 16; // column-major, 3 vec4s
    case UniformMemberType::Mat4: return 16; // column-major, 4 vec4s
    case UniformMemberType::Int: return 4;
    case UniformMemberType::Int2: return 8;
    case UniformMemberType::Int3: return 16;
    case UniformMemberType::Int4: return 16;
    default: return 4;
  }
}

static uint32_t GetStd140Size(UniformMemberType type) {
  switch (type) {
    case UniformMemberType::Float: return 4;
    case UniformMemberType::Float2: return 8;
    case UniformMemberType::Float3: return 12;
    case UniformMemberType::Float4: return 16;
    case UniformMemberType::Mat3: return 48; // 3 * 16
    case UniformMemberType::Mat4: return 64; // 4 * 16
    case UniformMemberType::Int: return 4;
    case UniformMemberType::Int2: return 8;
    case UniformMemberType::Int3: return 12;
    case UniformMemberType::Int4: return 16;
    default: return 4;
  }
}

IUniformLayout* CreateUniformLayoutInternal(UniformLayoutDesc const& desc) {
  if (!desc.members || desc.memberCount == 0) return nullptr;

  auto* layout = new UniformLayoutImpl();
  layout->members.resize(desc.memberCount);

  uint32_t currentOffset = 0;

  for (uint32_t i = 0; i < desc.memberCount; ++i) {
    layout->members[i] = desc.members[i];
    
    // Align offset
    uint32_t alignment = GetStd140Alignment(desc.members[i].type);
    currentOffset = (currentOffset + alignment - 1) & ~(alignment - 1);
    
    layout->members[i].offset = currentOffset;
    layout->members[i].size = GetStd140Size(desc.members[i].type);
    
    layout->nameToIndex[desc.members[i].name] = i;
    
    currentOffset += layout->members[i].size;
  }

  // Round up to 16-byte boundary
  layout->totalSize = (currentOffset + 15) & ~15;

  if (desc.totalSize > 0) {
    layout->totalSize = desc.totalSize;
  }

  return layout;
}

void ReleaseUniformLayoutInternal(IUniformLayout* layout) {
  delete static_cast<UniformLayoutImpl*>(layout);
}

// === Uniform Buffer Implementation ===

class UniformBufferImpl : public IUniformBuffer {
public:
  rhi::IDevice* device{nullptr};
  IUniformLayout* layout{nullptr};
  std::vector<rhi::IBuffer*> buffers;  // One per frame slot
  std::vector<uint8_t> cpuData;
  size_t bufferSize{0};
  FrameSlotId currentSlot{0};
  uint32_t framesInFlight{2};

  ~UniformBufferImpl() override {
    for (auto* buf : buffers) {
      if (buf && device) {
        device->DestroyBuffer(buf);
      }
    }
    if (layout) {
      ReleaseUniformLayoutInternal(layout);
    }
  }

  void Update(void const* data, size_t size) override {
    if (!data || size == 0) return;
    size_t copySize = std::min(size, bufferSize);
    if (copySize > cpuData.size()) {
      cpuData.resize(copySize);
    }
    std::memcpy(cpuData.data(), data, copySize);

    // Update GPU buffer for current slot
    if (currentSlot < buffers.size() && buffers[currentSlot] && device) {
      device->UpdateBuffer(buffers[currentSlot], 0, cpuData.data(), copySize);
    }
  }

  void Bind(rhi::ICommandList* cmd, uint32_t slot) override {
    if (!cmd || currentSlot >= buffers.size() || !buffers[currentSlot]) return;
    cmd->SetUniformBuffer(slot, buffers[currentSlot], 0);
  }

  size_t GetRingBufferOffset(FrameSlotId slot) const override {
    (void)slot;
    return 0; // Simple implementation: each slot has its own buffer
  }

  void SetCurrentFrameSlot(FrameSlotId slot) override {
    currentSlot = slot;
  }

  rhi::IBuffer* GetBuffer() override {
    if (currentSlot < buffers.size()) {
      return buffers[currentSlot];
    }
    return nullptr;
  }

  bool Initialize(IUniformLayout const* layoutDesc, rhi::IDevice* dev, uint32_t frames) {
    if (!layoutDesc || !dev) return false;

    device = dev;
    framesInFlight = frames;
    bufferSize = layoutDesc->GetTotalSize();

    if (bufferSize == 0) return false;

    cpuData.resize(bufferSize, 0);

    // Create one buffer per frame slot
    buffers.resize(framesInFlight);
    for (uint32_t i = 0; i < framesInFlight; ++i) {
      rhi::BufferDesc desc{};
      desc.size = bufferSize;
      desc.usage = static_cast<uint32_t>(rhi::BufferUsage::Uniform) | 
                   static_cast<uint32_t>(rhi::BufferUsage::CopyDst);
      buffers[i] = device->CreateBuffer(desc);
      if (!buffers[i]) return false;
    }

    return true;
  }
};

IUniformBuffer* CreateUniformBuffer(IUniformLayout const* layout, rhi::IDevice* device) {
  if (!layout || !device) return nullptr;

  auto* ub = new UniformBufferImpl();
  if (!ub->Initialize(layout, device, 2)) { // Default 2 frames in flight
    delete ub;
    return nullptr;
  }
  return ub;
}

void ReleaseUniformBuffer(IUniformBuffer* buffer) {
  delete static_cast<UniformBufferImpl*>(buffer);
}

// Override the inline stubs
#undef CreateUniformLayout
#undef ReleaseUniformLayout

IUniformLayout* CreateUniformLayout(UniformLayoutDesc const& desc) {
  return CreateUniformLayoutInternal(desc);
}

void ReleaseUniformLayout(IUniformLayout* layout) {
  ReleaseUniformLayoutInternal(layout);
}

}  // namespace te::rendercore
