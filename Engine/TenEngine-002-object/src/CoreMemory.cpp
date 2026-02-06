/**
 * Core memory integration: use only 001-Core contract API (Alloc/Free).
 * Contract: specs/_contracts/001-core-public-api.md
 */

#include <te/object/detail/CoreMemory.hpp>
#include <te/core/alloc.h>

namespace te {
namespace object {
namespace detail {

void* Alloc(std::size_t size, std::size_t alignment) {
  return core::Alloc(size, alignment);
}

void Free(void* ptr) {
  core::Free(ptr);
}

}  // namespace detail
}  // namespace object
}  // namespace te
