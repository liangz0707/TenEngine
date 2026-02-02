/** @file backend_vulkan.hpp
 *  Vulkan backend factory: CreateDeviceVulkan, DestroyDeviceVulkan.
 */
#pragma once

#include <te/rhi/device.hpp>

namespace te {
namespace rhi {

struct IDevice;
IDevice* CreateDeviceVulkan();
void DestroyDeviceVulkan(IDevice* device);

}  // namespace rhi
}  // namespace te
