/**
 * @file queue.hpp
 * @brief RHI queue abstraction (contract: specs/_contracts/008-rhi-public-api.md §2).
 */
#ifndef TE_RHI_QUEUE_HPP
#define TE_RHI_QUEUE_HPP

#include "te/rhi/types.hpp"

namespace te {
namespace rhi {

/** Queue handle; returned by IDevice::GetQueue. Non-owning, lifetime tied to IDevice. */
struct IQueue {
  virtual ~IQueue() = default;
};

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_QUEUE_HPP
