/** @file backend_d3d11.hpp
 *  D3D11 backend factory: CreateDeviceD3D11, DestroyDeviceD3D11.
 */
#pragma once

#include <te/rhi/device.hpp>

namespace te {
namespace rhi {

struct IDevice;
IDevice* CreateDeviceD3D11();
void DestroyDeviceD3D11(IDevice* device);

}  // namespace rhi
}  // namespace te
