/**
 * @file alloc.cpp
 * @brief Implementation of Alloc/Free, DefaultAllocator, GetDefaultAllocator per contract (001-core-ABI.md).
 * Double-free is no-op per contract.
 */

#include <mutex>
#include <unordered_set>
#include <cstdlib>
#include <cstddef>
#include "te/core/alloc.h"

#if defined(_WIN32) || defined(_WIN64)
#include <malloc.h>
#endif

namespace te {
namespace core {

namespace {

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

static std::mutex g_freed_mutex;
static std::unordered_set<void*> g_freed;

void AlignedFreeImpl(void* ptr) {
  if (!ptr) return;
  {
    std::lock_guard<std::mutex> lock(g_freed_mutex);
    if (g_freed.count(ptr) != 0) return;
    g_freed.insert(ptr);
  }
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

Allocator* GetDefaultAllocator() {
  static DefaultAllocator s_default;
  return &s_default;
}

}  // namespace core
}  // namespace te
