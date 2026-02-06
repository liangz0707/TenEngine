/**
 * @file alloc.h
 * @brief Memory allocation API (contract: specs/_contracts/001-core-public-api.md).
 * Only types and functions declared in the contract are exposed.
 */
#ifndef TE_CORE_ALLOC_H
#define TE_CORE_ALLOC_H

#include <cstddef>

namespace te {
namespace core {

/** Abstract allocator interface per contract capability 1. */
struct Allocator {
  virtual void* Alloc(std::size_t size, std::size_t alignment) = 0;
  virtual void Free(void* ptr) = 0;
  virtual ~Allocator() = default;
};

/** Default heap allocator; semantics per contract. */
class DefaultAllocator : public Allocator {
 public:
  void* Alloc(std::size_t size, std::size_t alignment) override;
  void Free(void* ptr) override;
};

/** Global Alloc: at least \a size bytes, aligned to \a alignment. Returns nullptr on failure, size==0, or invalid alignment. */
void* Alloc(std::size_t size, std::size_t alignment);

/** Global Free: no-op for nullptr or double-free. */
void Free(void* ptr);

/** Aligned allocation (semantically equivalent to Alloc but more explicit). Returns nullptr on failure. */
void* AllocAligned(std::size_t size, std::size_t alignment);

/** Reallocate memory (optional). Returns nullptr on failure. If ptr is nullptr, equivalent to Alloc. */
void* Realloc(void* ptr, std::size_t newSize);

/** Memory statistics structure (optional). */
struct MemoryStats {
  std::size_t allocated_bytes = 0;
  std::size_t peak_bytes = 0;
  std::size_t allocation_count = 0;
};

/** Get memory statistics (optional). Returns current memory usage stats. */
MemoryStats GetMemoryStats();

/** Return default heap allocator; caller does not own the pointer. Thread-safe. */
Allocator* GetDefaultAllocator();

}  // namespace core
}  // namespace te

#endif  // TE_CORE_ALLOC_H
