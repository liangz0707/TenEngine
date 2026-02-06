/** @file sync.cpp
 *  Sync module: Wait/Signal are inline in te/rhi/sync.hpp.
 *  This file ensures the sync translation unit exists for the build.
 */
#include <te/rhi/sync.hpp>

namespace te {
namespace rhi {
// Wait(IFence*), Signal(IFence*) are defined inline in sync.hpp.
}  // namespace rhi
}  // namespace te
