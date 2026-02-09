/** @file DeviceResources.h
 *  030-DeviceResourceManager internal: Device resources structure.
 */
#pragma once

#include <te/deviceresource/CommandListPool.h>
#include <te/deviceresource/StagingBufferManager.h>
#include <te/rhi/device.hpp>
#include <memory>

namespace te {
namespace deviceresource {
namespace internal {

/**
 * Per-device resource managers.
 */
struct DeviceResources {
  std::unique_ptr<CommandListPool> commandListPool;
  std::unique_ptr<StagingBufferManager> stagingBufferManager;
  
  // Default constructor (required for std::unordered_map)
  DeviceResources() = default;
  
  // Constructor with device
  DeviceResources(rhi::IDevice* device)
      : commandListPool(std::make_unique<CommandListPool>(device)),
        stagingBufferManager(std::make_unique<StagingBufferManager>(device)) {
  }
  
  // Move constructor
  DeviceResources(DeviceResources&&) = default;
  
  // Move assignment
  DeviceResources& operator=(DeviceResources&&) = default;
  
  // Delete copy constructor and assignment
  DeviceResources(DeviceResources const&) = delete;
  DeviceResources& operator=(DeviceResources const&) = delete;
};

}  // namespace internal
}  // namespace deviceresource
}  // namespace te
