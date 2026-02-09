/** @file StagingBufferManager.cpp
 *  030-DeviceResourceManager implementation: Staging buffer manager.
 */
#include <te/deviceresource/StagingBufferManager.h>
#include <te/rhi/device.hpp>
#include <te/core/log.h>
#include <algorithm>

namespace te {
namespace deviceresource {

StagingBufferManager::StagingBufferManager(rhi::IDevice* device)
    : device_(device), maxTotalSize_(64 * 1024 * 1024), currentTotalSize_(0) {
  if (!device_) {
    te::core::Log(te::core::LogLevel::Error, "StagingBufferManager: Invalid device");
  }
}

StagingBufferManager::~StagingBufferManager() {
  Clear();
}

rhi::IBuffer* StagingBufferManager::Allocate(size_t size) {
  if (!device_ || size == 0) {
    return nullptr;
  }

  std::lock_guard<std::mutex> lock(mutex_);

  // Try to find an available buffer that's large enough
  for (auto& entry : buffers_) {
    if (!entry.inUse && entry.size >= size) {
      entry.inUse = true;
      return entry.buffer;
    }
  }

  // Check if we can create a new buffer
  if (currentTotalSize_ + size > maxTotalSize_) {
    te::core::Log(te::core::LogLevel::Warn, "StagingBufferManager: Cannot allocate buffer, max size exceeded");
    return nullptr;
  }

  // Create new staging buffer
  rhi::BufferDesc desc{};
  desc.size = size;
  desc.usage = static_cast<uint32_t>(rhi::BufferUsage::CopySrc) | 
               static_cast<uint32_t>(rhi::BufferUsage::CopyDst);
  
  rhi::IBuffer* buffer = device_->CreateBuffer(desc);
  if (buffer) {
    BufferEntry entry;
    entry.buffer = buffer;
    entry.size = size;
    entry.inUse = true;
    buffers_.push_back(entry);
    currentTotalSize_ += size;
  }
  
  return buffer;
}

void StagingBufferManager::Release(rhi::IBuffer* buffer) {
  if (!buffer) {
    return;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  
  for (auto& entry : buffers_) {
    if (entry.buffer == buffer) {
      entry.inUse = false;
      return;
    }
  }
  
  // Buffer not found in pool, destroy it
  if (device_) {
    device_->DestroyBuffer(buffer);
  }
}

void StagingBufferManager::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  for (auto& entry : buffers_) {
    if (device_ && entry.buffer) {
      device_->DestroyBuffer(entry.buffer);
    }
  }
  
  buffers_.clear();
  currentTotalSize_ = 0;
}

}  // namespace deviceresource
}  // namespace te
