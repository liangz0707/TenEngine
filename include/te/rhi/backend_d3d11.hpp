/**
 * @file backend_d3d11.hpp
 * @brief D3D11 backend factory; only declare when TE_RHI_D3D11=1.
 * No D3D11 types here so device.cpp can include without d3d11.h.
 */
#ifndef TE_RHI_BACKEND_D3D11_HPP
#define TE_RHI_BACKEND_D3D11_HPP

#include "te/rhi/device.hpp"

namespace te {
namespace rhi {

/** Create D3D11-backed IDevice. Returns nullptr on failure. Windows only. */
IDevice* CreateDeviceD3D11();

/** Destroy D3D11 device; only call for device returned by CreateDeviceD3D11. */
void DestroyDeviceD3D11(IDevice* device);

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_BACKEND_D3D11_HPP
