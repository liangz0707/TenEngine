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

}  // namespace core
}  // namespace te

#endif  // TE_CORE_ALLOC_H
