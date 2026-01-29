/** Fallback Alloc/Free when 001-Core is not linked. Contract: use Core Alloc/Free when Core is initialized. */

#include "te/object/detail/CoreMemory.hpp"
#include <cstdlib>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
#include <malloc.h>
#endif

namespace te {
namespace object {
namespace detail {

namespace {

bool isPowerOfTwo(std::size_t n) {
    return n != 0 && (n & (n - 1)) == 0;
}

}  // namespace

void* Alloc(std::size_t size, std::size_t alignment) {
    if (size == 0 || !isPowerOfTwo(alignment)) return nullptr;
#if defined(_WIN32) || defined(_WIN64)
    void* p = _aligned_malloc(size, alignment);
#else
    void* p = nullptr;
    if (posix_memalign(&p, alignment, size) != 0) p = nullptr;
#endif
    return p;
}

void Free(void* ptr) {
    if (!ptr) return;
#if defined(_WIN32) || defined(_WIN64)
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}

}  // namespace detail
}  // namespace object
}  // namespace te
