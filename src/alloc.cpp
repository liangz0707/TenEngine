/**
 * @file alloc.cpp
 * @brief Implementation of Alloc/Free and DefaultAllocator per contract (001-core-public-api.md).
 * Uses aligned_alloc (POSIX) or _aligned_malloc (Windows). Comments in English.
 */

#include "te/core/alloc.h"
#include <cstdlib>
#include <cstddef>

#if defined(_WIN32) || defined(_WIN64)
#include <malloc.h>
#endif

namespace te {
namespace core {

namespace {

// Returns true if alignment is a power of two and valid for aligned allocation.
bool IsValidAlignment(std::size_t alignment) {
  return alignment != 0 && (alignment & (alignment - 1)) == 0;
}

void* AlignedAllocImpl(std::size_t size, std::size_t alignment) {
  if (size == 0 || !IsValidAlignment(alignment)) return nullptr;
#if defined(_WIN32) || defined(_WIN64)
  return _aligned_malloc(size, alignment);
#else
  return std::aligned_alloc(alignment, size);
#endif
}

void AlignedFreeImpl(void* ptr) {
  if (!ptr) return;
#if defined(_WIN32) || defined(_WIN64)
  _aligned_free(ptr);
#else
  std::free(ptr);
#endif
}

}  // namespace

void* DefaultAllocator::Alloc(std::size_t size, std::size_t alignment) {
  return AlignedAllocImpl(size, alignment);
}

void DefaultAllocator::Free(void* ptr) {
  AlignedFreeImpl(ptr);
}

void* Alloc(std::size_t size, std::size_t alignment) {
  return AlignedAllocImpl(size, alignment);
}

void Free(void* ptr) {
  AlignedFreeImpl(ptr);
}

}  // namespace core
}  // namespace te
