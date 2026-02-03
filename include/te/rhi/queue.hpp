/** @file queue.hpp
 *  008-RHI ABI: IQueue (Submit, WaitIdle).
 */
#pragma once

#include <te/rhi/types.hpp>

namespace te {
namespace rhi {

struct IQueue {
  virtual void Submit(ICommandList* cmdList,
                      IFence*       signalFence       = nullptr,
                      ISemaphore*   waitSemaphore     = nullptr,
                      ISemaphore*   signalSemaphore   = nullptr) = 0;
  virtual void WaitIdle() = 0;
  virtual ~IQueue() = default;
};

}  // namespace rhi
}  // namespace te
