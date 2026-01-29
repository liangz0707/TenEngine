/** Memory API per 001-core-public-api (Alloc/Free). 002-Object uses only these for heap allocation. */

#ifndef TE_OBJECT_DETAIL_CORE_MEMORY_HPP
#define TE_OBJECT_DETAIL_CORE_MEMORY_HPP

#include <cstddef>

namespace te {
namespace object {
namespace detail {

/** Alloc(size, alignment) per 001-core contract; use Core implementation when linked. */
void* Alloc(std::size_t size, std::size_t alignment);

/** Free(ptr) per 001-core contract; use Core implementation when linked. */
void Free(void* ptr);

}  // namespace detail
}  // namespace object
}  // namespace te

#endif
