/**
 * @file test_module_load.cpp
 * @brief Unit tests for LoadLibrary, UnloadLibrary, GetSymbol, ModuleInit/Shutdown per contract capability 7.
 */

#include "te/core/module_load.h"
#include <cassert>

using namespace te::core;

int main() {
  // ModuleInit/Shutdown callbacks (no actual DLL needed)
  static int init_count = 0, shutdown_count = 0;
  init_count = shutdown_count = 0;
  RegisterModuleInit([]() { ++init_count; });
  RegisterModuleShutdown([]() { ++shutdown_count; });
  RunModuleInit();
  assert(init_count == 1);
  RunModuleShutdown();
  assert(shutdown_count == 1);

  // LoadLibrary with invalid path returns nullptr
  ModuleHandle h = LoadLibrary("");
  assert(h == nullptr);
  h = LoadLibrary("nonexistent_dll_12345.dll");
  assert(h == nullptr);

  // UnloadLibrary(nullptr) is no-op
  UnloadLibrary(nullptr);

  // GetSymbol(nullptr, "x") returns nullptr
  assert(GetSymbol(nullptr, "x") == nullptr);
  assert(GetSymbol(nullptr, "") == nullptr);

  return 0;
}
