/** @file StagingBufferManager.h
 *  030-DeviceResourceManager internal: Staging buffer manager for GPU data upload.
 */
#pragma once

#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <mutex>
#include <vector>
#include <cstddef>

namespace te {
namespace deviceresource {

/**
 * Staging buffer manager for efficient GPU data upload.
 * Manages multiple staging buffers of different sizes.
 * Thread-safe.
 */
class StagingBufferManager {
 public:
  explicit StagingBufferManager(rhi::IDevice* device);
  ~StagingBufferManager();

  /**
   * Allocate staging buffer of at least the requested size.
   * Returns nullptr if allocation fails.
   */
  rhi::IBuffer* Allocate(size_t size);

  /**
   * Release staging buffer back to the pool.
   */
  void Release(rhi::IBuffer* buffer);

  /**
   * Clear all staging buffers.
   */
  void Clear();

 private:
  rhi::IDevice* device_;
  std::mutex mutex_;
  size_t maxTotalSize_;  // Maximum total size (default 64MB)
  size_t currentTotalSize_;
  
  struct BufferEntry {
    rhi::IBuffer* buffer;
    size_t size;
    bool inUse;
  };
  std::vector<BufferEntry> buffers_;
};

}  // namespace deviceresource
}  // namespace te
