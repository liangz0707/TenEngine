/** @file backend_d3d12.hpp
 *  D3D12 backend factory: CreateDeviceD3D12, DestroyDeviceD3D12.
 */
#pragma once

#include <te/rhi/device.hpp>

namespace te {
namespace rhi {

struct IDevice;
IDevice* CreateDeviceD3D12();
void DestroyDeviceD3D12(IDevice* device);

}  // namespace rhi
}  // namespace te
