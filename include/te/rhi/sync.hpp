/**
 * @file sync.hpp
 * @brief RHI synchronization (contract: specs/_contracts/008-rhi-public-api.md section 7).
 */
#ifndef TE_RHI_SYNC_HPP
#define TE_RHI_SYNC_HPP

#include "te/rhi/types.hpp"

namespace te {
namespace rhi {

struct IFence {
  virtual ~IFence() = default;
};

struct ISemaphore {
  virtual ~ISemaphore() = default;
};

void Wait(IFence* f);
void Signal(IFence* f);

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_SYNC_HPP
