/**
 * @file test_engine.cpp
 * @brief Unit tests for Init/Shutdown per contract (001-core-ABI.md).
 */

#include "te/core/engine.h"
#include <cassert>

using namespace te::core;

int main() {
  assert(Init(nullptr) == true);
  assert(Init(nullptr) == true);  // idempotent
  Shutdown();
  assert(Init(nullptr) == true);
  Shutdown();
  return 0;
}
