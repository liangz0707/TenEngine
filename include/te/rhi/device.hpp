/**
 * @file device.hpp
 * @brief RHI device abstraction (contract: specs/_contracts/008-rhi-public-api.md §1, §3, §4).
 */
#ifndef TE_RHI_DEVICE_HPP
#define TE_RHI_DEVICE_HPP

#include "te/rhi/queue.hpp"
#include "te/rhi/types.hpp"

namespace te {
namespace rhi {

struct ICommandList;  // forward; defined in command_list.hpp

/** Graphics device abstraction; creates queues, resources, PSO. Lifetime until DestroyDevice. */
struct IDevice {
  virtual ~IDevice() = default;

  virtual IQueue* GetQueue(QueueType type, uint32_t index) = 0;
  virtual DeviceFeatures const& GetFeatures() const = 0;
  virtual ICommandList* CreateCommandList() = 0;
  virtual void DestroyCommandList(ICommandList* cmd) = 0;
};

/** Set default backend; used when CreateDevice() is called with no args. */
void SelectBackend(Backend b);

/** Get currently selected backend. */
Backend GetSelectedBackend();

/** Create device for given backend. Returns nullptr on failure. Does not depend on window/context. */
IDevice* CreateDevice(Backend backend);

/** Create device using GetSelectedBackend(). */
IDevice* CreateDevice();

/** Destroy device; nullptr is no-op. */
void DestroyDevice(IDevice* device);

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_DEVICE_HPP
