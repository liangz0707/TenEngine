/** @file CoreMemory.hpp
 *  Internal: wrap 001-Core Alloc/Free per specs/_contracts/001-core-public-api.md
 */
#ifndef TE_OBJECT_DETAIL_CORE_MEMORY_HPP
#define TE_OBJECT_DETAIL_CORE_MEMORY_HPP

#include <cstddef>

namespace te {
namespace object {
namespace detail {

void* Alloc(std::size_t size, std::size_t alignment);
void Free(void* ptr);

}  // namespace detail
}  // namespace object
}  // namespace te

#endif  // TE_OBJECT_DETAIL_CORE_MEMORY_HPP
