/**
 * @file backend_d3d12.hpp
 * @brief D3D12 backend factory; only declare when TE_RHI_D3D12=1.
 * No D3D12 types here so device.cpp can include without d3d12.h.
 */
#ifndef TE_RHI_BACKEND_D3D12_HPP
#define TE_RHI_BACKEND_D3D12_HPP

#include "te/rhi/device.hpp"

namespace te {
namespace rhi {

/** Create D3D12-backed IDevice. Returns nullptr on failure. Windows only. */
IDevice* CreateDeviceD3D12();

/** Destroy D3D12 device; only call for device returned by CreateDeviceD3D12. */
void DestroyDeviceD3D12(IDevice* device);

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_BACKEND_D3D12_HPP
