/**
 * @file test_containers.cpp
 * @brief Unit tests for Array, Map, String, UniquePtr, SharedPtr per contract capability 6.
 */

#include "te/core/containers.h"
#include <cassert>

using namespace te::core;

int main() {
  Array<int> arr;
  arr.push_back(1);
  arr.push_back(2);
  assert(arr.size() == 2 && arr[0] == 1 && arr[1] == 2);

  Map<String, int> map;
  map["a"] = 1;
  map["b"] = 2;
  assert(map.size() == 2 && map["a"] == 1 && map["b"] == 2);

  String s = "hello";
  assert(s.size() == 5 && s[0] == 'h');

  UniquePtr<int> up(new int(42));
  assert(up && *up == 42);
  up.reset();
  assert(!up);

  SharedPtr<int> sp = std::make_shared<int>(99);
  assert(sp && *sp == 99);
  SharedPtr<int> sp2 = sp;
  assert(sp.use_count() == 2);

  return 0;
}
