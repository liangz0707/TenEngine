/**
 * @file backend_vulkan.hpp
 * @brief Vulkan backend factory; only declare when TE_RHI_VULKAN=1.
 * No Vulkan types here so device.cpp can include without volk.
 */
#ifndef TE_RHI_BACKEND_VULKAN_HPP
#define TE_RHI_BACKEND_VULKAN_HPP

#include "te/rhi/device.hpp"

namespace te {
namespace rhi {

/** Create Vulkan-backed IDevice. Returns nullptr on failure. */
IDevice* CreateDeviceVulkan();

/** Destroy Vulkan device; only call for device returned by CreateDeviceVulkan. */
void DestroyDeviceVulkan(IDevice* device);

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_BACKEND_VULKAN_HPP
