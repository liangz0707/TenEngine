/** @file sync.hpp
 *  008-RHI ABI: IFence, ISemaphore, Wait, Signal.
 */
#pragma once

#include <te/rhi/types.hpp>

namespace te {
namespace rhi {

struct IFence {
  virtual void Wait() = 0;
  virtual void Signal() = 0;
  virtual void Reset() = 0;
  virtual ~IFence() = default;
};

struct ISemaphore {
  virtual ~ISemaphore() = default;
};

inline void Wait(IFence* f) {
  if (f) f->Wait();
}

inline void Signal(IFence* f) {
  if (f) f->Signal();
}

}  // namespace rhi
}  // namespace te
