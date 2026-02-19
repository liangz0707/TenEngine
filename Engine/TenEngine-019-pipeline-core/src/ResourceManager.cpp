/**
 * @file ResourceManager.cpp
 * @brief Implementation of TransientResourcePool and ResourceBarrierBuilder.
 */

#include <te/pipelinecore/ResourceManager.h>

#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <mutex>
#include <unordered_set>

namespace te::pipelinecore {

// === TransientResourcePool::Impl ===

struct TextureEntry {
  TransientTextureDesc desc;
  rhi::ITexture* texture{nullptr};
  ResourceLifetimeInfo lifetime;
  bool created{false};
};

struct BufferEntry {
  TransientBufferDesc desc;
  rhi::IBuffer* buffer{nullptr};
  ResourceLifetimeInfo lifetime;
  bool created{false};
};

struct PassBarriers {
  std::vector<ResourceBarrier> barriers;
};

struct TransientResourcePool::Impl {
  rhi::IDevice* device{nullptr};
  CreateTextureCallback createTextureCb;
  CreateBufferCallback createBufferCb;

  uint64_t nextHandleId{1};
  uint32_t currentPass{0};
  bool isCompiled{false};

  std::unordered_map<uint64_t, TextureEntry> textures;
  std::unordered_map<uint64_t, BufferEntry> buffers;

  std::vector<PassBarriers> passBarriers;
  std::vector<ResourceBarrier> allBarriers;

  size_t totalMemoryUsed{0};

  // Default dimensions for transient textures
  uint32_t defaultWidth_{800};
  uint32_t defaultHeight_{600};

  void Reset() {
    // Destroy all RHI resources
    for (auto& pair : textures) {
      if (pair.second.texture && device) {
        device->DestroyTexture(pair.second.texture);
      }
    }
    for (auto& pair : buffers) {
      if (pair.second.buffer && device) {
        device->DestroyBuffer(pair.second.buffer);
      }
    }

    textures.clear();
    buffers.clear();
    passBarriers.clear();
    allBarriers.clear();
    totalMemoryUsed = 0;
    nextHandleId = 1;
    currentPass = 0;
    isCompiled = false;
  }

  TextureEntry* FindTexture(uint64_t id) {
    auto it = textures.find(id);
    return it != textures.end() ? &it->second : nullptr;
  }

  BufferEntry* FindBuffer(uint64_t id) {
    auto it = buffers.find(id);
    return it != buffers.end() ? &it->second : nullptr;
  }

  rhi::ITexture* CreateTexture(TextureEntry& entry) {
    if (entry.texture) return entry.texture;

    if (createTextureCb) {
      entry.texture = createTextureCb(entry.desc);
    } else if (device) {
      rhi::TextureDesc rhiDesc{};
      rhiDesc.width = entry.desc.width;
      rhiDesc.height = entry.desc.height;
      rhiDesc.depth = entry.desc.depth;
      rhiDesc.format = entry.desc.format;
      entry.texture = device->CreateTexture(rhiDesc);
    }

    if (entry.texture) {
      entry.created = true;
      // Approximate memory usage
      size_t pixelSize = 4; // Assume 4 bytes per pixel
      totalMemoryUsed += entry.desc.width * entry.desc.height * entry.desc.depth * pixelSize;
    }

    return entry.texture;
  }

  rhi::IBuffer* CreateBuffer(BufferEntry& entry) {
    if (entry.buffer) return entry.buffer;

    if (createBufferCb) {
      entry.buffer = createBufferCb(entry.desc);
    } else if (device) {
      rhi::BufferDesc rhiDesc{};
      rhiDesc.size = entry.desc.size;
      rhiDesc.usage = entry.desc.usage;
      entry.buffer = device->CreateBuffer(rhiDesc);
    }

    if (entry.buffer) {
      entry.created = true;
      totalMemoryUsed += entry.desc.size;
    }

    return entry.buffer;
  }
};

// === TransientResourcePool ===

TransientResourcePool::TransientResourcePool()
  : impl_(std::make_unique<Impl>()) {
}

TransientResourcePool::~TransientResourcePool() {
  if (impl_) {
    impl_->Reset();
  }
}

TransientResourcePool::TransientResourcePool(TransientResourcePool&& other) noexcept
  : impl_(std::move(other.impl_)) {
}

TransientResourcePool& TransientResourcePool::operator=(TransientResourcePool&& other) noexcept {
  if (this != &other) {
    impl_ = std::move(other.impl_);
  }
  return *this;
}

void TransientResourcePool::SetDevice(rhi::IDevice* device) {
  impl_->device = device;
}

void TransientResourcePool::SetCreateCallbacks(
    CreateTextureCallback textureCb,
    CreateBufferCallback bufferCb) {
  impl_->createTextureCb = std::move(textureCb);
  impl_->createBufferCb = std::move(bufferCb);
}

void TransientResourcePool::BeginFrame() {
  impl_->Reset();
}

void TransientResourcePool::EndFrame() {
  impl_->Reset();
}

TransientResourceHandle TransientResourcePool::DeclareTransientTexture(
    TransientTextureDesc const& desc) {
  uint64_t id = impl_->nextHandleId++;

  TextureEntry entry{};
  entry.desc = desc;
  entry.lifetime.handle = {id, TransientResourceType::Texture};
  entry.lifetime.firstUsePass = 0xFFFFFFFF;
  entry.lifetime.lastUsePass = 0;
  entry.lifetime.createPass = 0xFFFFFFFF;
  entry.lifetime.releasePass = 0xFFFFFFFF;
  entry.lifetime.isUsed = false;
  entry.texture = nullptr;
  entry.created = false;

  impl_->textures[id] = std::move(entry);

  return {id, TransientResourceType::Texture};
}

TransientResourceHandle TransientResourcePool::DeclareTransientBuffer(
    TransientBufferDesc const& desc) {
  uint64_t id = impl_->nextHandleId++;

  BufferEntry entry{};
  entry.desc = desc;
  entry.lifetime.handle = {id, TransientResourceType::Buffer};
  entry.lifetime.firstUsePass = 0xFFFFFFFF;
  entry.lifetime.lastUsePass = 0;
  entry.lifetime.createPass = 0xFFFFFFFF;
  entry.lifetime.releasePass = 0xFFFFFFFF;
  entry.lifetime.isUsed = false;
  entry.buffer = nullptr;
  entry.created = false;

  impl_->buffers[id] = std::move(entry);

  return {id, TransientResourceType::Buffer};
}

TransientTextureDesc const* TransientResourcePool::GetTextureDesc(
    TransientResourceHandle handle) const {
  auto* entry = impl_->FindTexture(handle.id);
  return entry ? &entry->desc : nullptr;
}

TransientBufferDesc const* TransientResourcePool::GetBufferDesc(
    TransientResourceHandle handle) const {
  auto* entry = impl_->FindBuffer(handle.id);
  return entry ? &entry->desc : nullptr;
}

void TransientResourcePool::MarkResourceRead(TransientResourceHandle handle, uint32_t passIndex) {
  if (handle.IsTexture()) {
    auto* entry = impl_->FindTexture(handle.id);
    if (entry) {
      entry->lifetime.firstUsePass = std::min(entry->lifetime.firstUsePass, passIndex);
      entry->lifetime.lastUsePass = std::max(entry->lifetime.lastUsePass, passIndex);
      entry->lifetime.isUsed = true;
    }
  } else {
    auto* entry = impl_->FindBuffer(handle.id);
    if (entry) {
      entry->lifetime.firstUsePass = std::min(entry->lifetime.firstUsePass, passIndex);
      entry->lifetime.lastUsePass = std::max(entry->lifetime.lastUsePass, passIndex);
      entry->lifetime.isUsed = true;
    }
  }
}

void TransientResourcePool::MarkResourceWrite(TransientResourceHandle handle, uint32_t passIndex) {
  // Write implies read for lifetime tracking
  MarkResourceRead(handle, passIndex);
}

void TransientResourcePool::ReleaseAfterPass(TransientResourceHandle handle, uint32_t passIndex) {
  if (handle.IsTexture()) {
    auto* entry = impl_->FindTexture(handle.id);
    if (entry) {
      entry->lifetime.releasePass = passIndex;
    }
  } else {
    auto* entry = impl_->FindBuffer(handle.id);
    if (entry) {
      entry->lifetime.releasePass = passIndex;
    }
  }
}

void TransientResourcePool::Compile() {
  impl_->allBarriers.clear();
  impl_->passBarriers.clear();

  // Find max pass index
  uint32_t maxPass = 0;
  for (auto const& pair : impl_->textures) {
    maxPass = std::max(maxPass, pair.second.lifetime.lastUsePass);
  }
  for (auto const& pair : impl_->buffers) {
    maxPass = std::max(maxPass, pair.second.lifetime.lastUsePass);
  }

  // Initialize pass barriers
  impl_->passBarriers.resize(maxPass + 1);

  // Generate barriers based on state transitions
  // For each resource, insert barriers when state changes between passes
  // This is a simplified version - full version would track actual state per pass

  // Group barriers by pass
  for (auto const& pair : impl_->textures) {
    auto const& lifetime = pair.second.lifetime;
    if (!lifetime.isUsed) continue;

    // Initial transition to first use state
    if (lifetime.firstUsePass <= maxPass) {
      ResourceBarrier barrier{};
      barrier.resource = lifetime.handle;
      barrier.srcState = pair.second.desc.initialState;
      // Assume first use is render target for textures
      barrier.dstState = rhi::ResourceState::RenderTarget;
      barrier.beforePass = lifetime.firstUsePass;

      if (barrier.beforePass < impl_->passBarriers.size()) {
        impl_->passBarriers[barrier.beforePass].barriers.push_back(barrier);
        impl_->allBarriers.push_back(barrier);
      }
    }
  }

  for (auto const& pair : impl_->buffers) {
    auto const& lifetime = pair.second.lifetime;
    if (!lifetime.isUsed) continue;

    // Initial transition
    if (lifetime.firstUsePass <= maxPass) {
      ResourceBarrier barrier{};
      barrier.resource = lifetime.handle;
      barrier.srcState = pair.second.desc.initialState;
      // Assume first use is uniform buffer
      barrier.dstState = rhi::ResourceState::ShaderResource;
      barrier.beforePass = lifetime.firstUsePass;

      if (barrier.beforePass < impl_->passBarriers.size()) {
        impl_->passBarriers[barrier.beforePass].barriers.push_back(barrier);
        impl_->allBarriers.push_back(barrier);
      }
    }
  }

  // Sort barriers by pass
  std::sort(impl_->allBarriers.begin(), impl_->allBarriers.end(),
    [](ResourceBarrier const& a, ResourceBarrier const& b) {
      return a.beforePass < b.beforePass;
    });

  impl_->isCompiled = true;
}

std::vector<ResourceBarrier> const& TransientResourcePool::GetBarriersForPass(
    uint32_t passIndex) const {
  static std::vector<ResourceBarrier> empty;
  if (passIndex >= impl_->passBarriers.size()) {
    return empty;
  }
  return impl_->passBarriers[passIndex].barriers;
}

std::vector<ResourceBarrier> const& TransientResourcePool::GetAllBarriers() const {
  return impl_->allBarriers;
}

rhi::ITexture* TransientResourcePool::GetOrCreateTexture(TransientResourceHandle handle) {
  auto* entry = impl_->FindTexture(handle.id);
  if (!entry) return nullptr;
  return impl_->CreateTexture(*entry);
}

rhi::IBuffer* TransientResourcePool::GetOrCreateBuffer(TransientResourceHandle handle) {
  auto* entry = impl_->FindBuffer(handle.id);
  if (!entry) return nullptr;
  return impl_->CreateBuffer(*entry);
}

rhi::ITexture* TransientResourcePool::GetTexture(TransientResourceHandle handle) const {
  auto* entry = impl_->FindTexture(handle.id);
  return entry ? entry->texture : nullptr;
}

rhi::IBuffer* TransientResourcePool::GetBuffer(TransientResourceHandle handle) const {
  auto* entry = impl_->FindBuffer(handle.id);
  return entry ? entry->buffer : nullptr;
}

bool TransientResourcePool::IsResourceCreated(TransientResourceHandle handle) const {
  if (handle.IsTexture()) {
    auto* entry = impl_->FindTexture(handle.id);
    return entry && entry->created;
  } else {
    auto* entry = impl_->FindBuffer(handle.id);
    return entry && entry->created;
  }
}

void TransientResourcePool::InsertBarriersForPass(uint32_t passIndex, rhi::ICommandList* cmd) {
  if (!cmd || passIndex >= impl_->passBarriers.size()) return;

  auto const& barriers = impl_->passBarriers[passIndex].barriers;
  for (auto const& barrier : barriers) {
    if (barrier.resource.IsTexture()) {
      rhi::ITexture* texture = GetTexture(barrier.resource);
      if (texture) {
        rhi::TextureBarrier rhiBarrier{};
        rhiBarrier.texture = texture;
        rhiBarrier.mipLevel = 0;
        rhiBarrier.arrayLayer = 0;
        rhiBarrier.srcState = barrier.srcState;
        rhiBarrier.dstState = barrier.dstState;
        cmd->ResourceBarrier(nullptr, 0, &rhiBarrier, 1);
      }
    } else {
      rhi::IBuffer* buffer = GetBuffer(barrier.resource);
      if (buffer) {
        rhi::BufferBarrier rhiBarrier{};
        rhiBarrier.buffer = buffer;
        rhiBarrier.offset = 0;
        rhiBarrier.size = 0; // Whole buffer
        rhiBarrier.srcState = barrier.srcState;
        rhiBarrier.dstState = barrier.dstState;
        cmd->ResourceBarrier(&rhiBarrier, 1, nullptr, 0);
      }
    }
  }
}

ResourceLifetimeInfo const* TransientResourcePool::GetLifetimeInfo(
    TransientResourceHandle handle) const {
  if (handle.IsTexture()) {
    auto* entry = impl_->FindTexture(handle.id);
    return entry ? &entry->lifetime : nullptr;
  } else {
    auto* entry = impl_->FindBuffer(handle.id);
    return entry ? &entry->lifetime : nullptr;
  }
}

size_t TransientResourcePool::GetAllocatedTextureCount() const {
  return impl_->textures.size();
}

size_t TransientResourcePool::GetAllocatedBufferSize() const {
  return impl_->buffers.size();
}

size_t TransientResourcePool::GetTotalMemoryUsed() const {
  return impl_->totalMemoryUsed;
}

void TransientResourcePool::SetDefaultDimensions(uint32_t width, uint32_t height) {
  impl_->defaultWidth_ = width;
  impl_->defaultHeight_ = height;
}

rhi::ITexture* TransientResourcePool::CreateTextureFromAttachment(
    uint32_t width,
    uint32_t height,
    uint32_t format,
    rhi::ResourceState initialState) {
  
  // Use default dimensions if zero
  uint32_t w = (width > 0) ? width : impl_->defaultWidth_;
  uint32_t h = (height > 0) ? height : impl_->defaultHeight_;

  TransientTextureDesc desc{};
  desc.width = w;
  desc.height = h;
  desc.depth = 1;
  desc.format = format;
  desc.initialState = initialState;

  auto handle = DeclareTransientTexture(desc);
  return GetOrCreateTexture(handle);
}

// === ResourceBarrierBuilder::Impl ===

struct ResourceBarrierBuilder::Impl {
  std::vector<ResourceBarrier> pending;
};

// === ResourceBarrierBuilder ===

ResourceBarrierBuilder::ResourceBarrierBuilder()
  : impl_(std::make_unique<Impl>()) {
}

ResourceBarrierBuilder::~ResourceBarrierBuilder() = default;

void ResourceBarrierBuilder::AddTextureTransition(
    TransientResourceHandle handle,
    rhi::ResourceState srcState,
    rhi::ResourceState dstState,
    uint32_t beforePass) {
  ResourceBarrier barrier{};
  barrier.resource = handle;
  barrier.srcState = srcState;
  barrier.dstState = dstState;
  barrier.beforePass = beforePass;
  impl_->pending.push_back(barrier);
}

void ResourceBarrierBuilder::AddBufferTransition(
    TransientResourceHandle handle,
    rhi::ResourceState srcState,
    rhi::ResourceState dstState,
    uint32_t beforePass) {
  ResourceBarrier barrier{};
  barrier.resource = handle;
  barrier.srcState = srcState;
  barrier.dstState = dstState;
  barrier.beforePass = beforePass;
  impl_->pending.push_back(barrier);
}

std::vector<ResourceBarrier> ResourceBarrierBuilder::Build() {
  // Sort by pass index
  std::sort(impl_->pending.begin(), impl_->pending.end(),
    [](ResourceBarrier const& a, ResourceBarrier const& b) {
      return a.beforePass < b.beforePass;
    });

  // Remove duplicates (same resource, same pass, same states)
  auto last = std::unique(impl_->pending.begin(), impl_->pending.end(),
    [](ResourceBarrier const& a, ResourceBarrier const& b) {
      return a.resource.id == b.resource.id &&
             a.beforePass == b.beforePass &&
             a.srcState == b.srcState &&
             a.dstState == b.dstState;
    });
  impl_->pending.erase(last, impl_->pending.end());

  return impl_->pending;
}

void ResourceBarrierBuilder::Clear() {
  impl_->pending.clear();
}

// === Free Functions ===

TransientResourcePool* CreateTransientResourcePool() {
  return new TransientResourcePool();
}

void DestroyTransientResourcePool(TransientResourcePool* pool) {
  delete pool;
}

ResourceBarrierBuilder* CreateResourceBarrierBuilder() {
  return new ResourceBarrierBuilder();
}

void DestroyResourceBarrierBuilder(ResourceBarrierBuilder* builder) {
  delete builder;
}

}  // namespace te::pipelinecore
