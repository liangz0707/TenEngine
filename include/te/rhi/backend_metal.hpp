/** @file backend_metal.hpp
 *  Metal backend factory: CreateDeviceMetal, DestroyDeviceMetal.
 */
#pragma once

#include <te/rhi/device.hpp>

namespace te {
namespace rhi {

struct IDevice;
IDevice* CreateDeviceMetal();
void DestroyDeviceMetal(IDevice* device);

}  // namespace rhi
}  // namespace te
