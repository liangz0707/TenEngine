/**
 * @file test_check.cpp
 * @brief Unit tests for CheckWarning, CheckError per contract (001-core-ABI.md).
 */

#include "te/core/check.h"
#include <cassert>

int main() {
  CheckWarning(1 == 1);
  CheckWarning(1 == 0, "custom warning message");
  CheckError(1 == 1);
  return 0;
}
