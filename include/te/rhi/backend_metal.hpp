/**
 * @file backend_metal.hpp
 * @brief Metal backend factory; only declare when TE_RHI_METAL=1.
 * No Metal types here so device.cpp can include without Metal headers.
 */
#ifndef TE_RHI_BACKEND_METAL_HPP
#define TE_RHI_BACKEND_METAL_HPP

#include "te/rhi/device.hpp"

namespace te {
namespace rhi {

/** Create Metal-backed IDevice. Returns nullptr on failure. macOS/iOS only. */
IDevice* CreateDeviceMetal();

/** Destroy Metal device; only call for device returned by CreateDeviceMetal. */
void DestroyDeviceMetal(IDevice* device);

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_BACKEND_METAL_HPP
