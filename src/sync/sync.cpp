/**
 * @file sync.cpp
 * @brief RHI sync implementation (stub).
 */
#include "te/rhi/sync.hpp"

namespace te {
namespace rhi {

void Wait(IFence* f) {
  if (f) f->Wait();
}

void Signal(IFence* f) {
  if (f) f->Signal();
}

}  // namespace rhi
}  // namespace te
