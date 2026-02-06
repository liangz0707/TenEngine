/**
 * @file test_alloc.cpp
 * @brief Unit tests for Alloc/Free per contract: success, alignment, nullptr, double-free no-op.
 */

#include "te/core/alloc.h"
#include <cassert>
#include <cstddef>
#include <cstring>

using namespace te::core;

int main() {
  // Success: allocate and free
  void* p = Alloc(64, 8);
  assert(p != nullptr);
  std::memset(p, 0, 64);
  Free(p);

  // size==0 returns nullptr
  assert(Alloc(0, 8) == nullptr);

  // Invalid alignment (non power-of-two) returns nullptr
  assert(Alloc(16, 3) == nullptr);
  assert(Alloc(16, 0) == nullptr);

  // Free(nullptr) is no-op
  Free(nullptr);

  // Double-free is no-op (contract: safe to ignore)
  void* q = Alloc(32, 16);
  assert(q != nullptr);
  Free(q);
  Free(q);  // second free is no-op

  // DefaultAllocator
  DefaultAllocator alloc;
  void* r = alloc.Alloc(8, 8);
  assert(r != nullptr);
  alloc.Free(r);
  alloc.Free(nullptr);

  // GetDefaultAllocator: non-null, Alloc/Free via returned allocator matches global semantics
  Allocator* def = GetDefaultAllocator();
  assert(def != nullptr);
  void* s = def->Alloc(16, 8);
  assert(s != nullptr);
  def->Free(s);
  def->Free(nullptr);
  assert(def->Alloc(0, 8) == nullptr);

  return 0;
}
