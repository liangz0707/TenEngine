/**
 * @file queue.hpp
 * @brief RHI queue abstraction (contract: specs/_contracts/008-rhi-public-api.md §2).
 */
#ifndef TE_RHI_QUEUE_HPP
#define TE_RHI_QUEUE_HPP

#include "te/rhi/types.hpp"

namespace te {
namespace rhi {

struct ICommandList;
struct IFence;
struct ISemaphore;

/** Queue handle; returned by IDevice::GetQueue. Non-owning, lifetime tied to IDevice. */
struct IQueue {
  virtual ~IQueue() = default;
  virtual void Submit(ICommandList* cmdList, IFence* signalFence = nullptr,
                     ISemaphore* waitSemaphore = nullptr, ISemaphore* signalSemaphore = nullptr) = 0;
  virtual void WaitIdle() = 0;
};

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_QUEUE_HPP
