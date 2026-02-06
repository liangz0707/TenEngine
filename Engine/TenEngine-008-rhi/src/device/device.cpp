/** @file device.cpp
 *  Device factory: SelectBackend, GetSelectedBackend, CreateDevice, DestroyDevice.
 *  Dispatches per TE_RHI_VULKAN / TE_RHI_D3D12 / TE_RHI_D3D11 / TE_RHI_METAL.
 */
#include <te/rhi/device.hpp>
#include <te/rhi/types.hpp>

#if defined(TE_RHI_VULKAN)
#include <te/rhi/backend_vulkan.hpp>
#endif
#if defined(TE_RHI_D3D12) && defined(_WIN32)
#include <te/rhi/backend_d3d12.hpp>
#endif
#if defined(TE_RHI_D3D11) && defined(_WIN32)
#include <te/rhi/backend_d3d11.hpp>
#endif
#if defined(TE_RHI_METAL) && (defined(__APPLE__) && defined(__MACH__))
#include <te/rhi/backend_metal.hpp>
#endif

namespace te {
namespace rhi {

namespace {
Backend g_selected_backend = Backend::Vulkan;
}  // namespace

void SelectBackend(Backend b) {
  g_selected_backend = b;
}

Backend GetSelectedBackend() {
  return g_selected_backend;
}

IDevice* CreateDevice(Backend backend) {
  switch (backend) {
#if defined(TE_RHI_VULKAN)
  case Backend::Vulkan:
    return CreateDeviceVulkan();
#endif
#if defined(TE_RHI_D3D12) && defined(_WIN32)
  case Backend::D3D12:
    return CreateDeviceD3D12();
#endif
#if defined(TE_RHI_D3D11) && defined(_WIN32)
  case Backend::D3D11:
    return CreateDeviceD3D11();
#endif
#if defined(TE_RHI_METAL) && (defined(__APPLE__) && defined(__MACH__))
  case Backend::Metal:
    return CreateDeviceMetal();
#endif
  default:
    return nullptr;
  }
}

IDevice* CreateDevice() {
  return CreateDevice(GetSelectedBackend());
}

void DestroyDevice(IDevice* device) {
  if (!device) return;
  switch (device->GetBackend()) {
#if defined(TE_RHI_VULKAN)
  case Backend::Vulkan:
    DestroyDeviceVulkan(device);
    return;
#endif
#if defined(TE_RHI_D3D12) && defined(_WIN32)
  case Backend::D3D12:
    DestroyDeviceD3D12(device);
    return;
#endif
#if defined(TE_RHI_D3D11) && defined(_WIN32)
  case Backend::D3D11:
    DestroyDeviceD3D11(device);
    return;
#endif
#if defined(TE_RHI_METAL) && (defined(__APPLE__) && defined(__MACH__))
  case Backend::Metal:
    DestroyDeviceMetal(device);
    return;
#endif
  default:
    break;
  }
}

}  // namespace rhi
}  // namespace te
