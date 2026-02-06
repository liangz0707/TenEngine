/**
 * @file module_load.cpp
 * @brief Implementation of LoadLibrary, UnloadLibrary, GetSymbol, ModuleInit/Shutdown per contract (001-core-public-api.md).
 * Uses dlopen/LoadLibrary/dyld. Comments in English.
 */

#include "te/core/module_load.h"
#include <vector>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#ifdef LoadLibrary
#undef LoadLibrary
#endif
#define TE_LOAD_LIBRARY(path) ::LoadLibraryA(path)
#define TE_GET_SYMBOL(handle, name) reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(handle), name))
#define TE_UNLOAD_LIBRARY(handle) ::FreeLibrary(static_cast<HMODULE>(handle))
#elif defined(__APPLE__)
#include <dlfcn.h>
#define TE_LOAD_LIBRARY(path) dlopen(path, RTLD_LAZY)
#define TE_GET_SYMBOL(handle, name) dlsym(handle, name)
#define TE_UNLOAD_LIBRARY(handle) dlclose(handle)
#else
#include <dlfcn.h>
#define TE_LOAD_LIBRARY(path) dlopen(path, RTLD_LAZY)
#define TE_GET_SYMBOL(handle, name) dlsym(handle, name)
#define TE_UNLOAD_LIBRARY(handle) dlclose(handle)
#endif

namespace te {
namespace core {

namespace {

std::vector<ModuleInitFn> g_init_fns;
std::vector<ModuleShutdownFn> g_shutdown_fns;

}  // namespace

ModuleHandle LoadLibrary(char const* path) {
  if (!path || path[0] == '\0') return nullptr;
  return TE_LOAD_LIBRARY(path);
}

void UnloadLibrary(ModuleHandle handle) {
  if (!handle) return;
  TE_UNLOAD_LIBRARY(handle);
}

void* GetSymbol(ModuleHandle handle, char const* name) {
  if (!handle || !name || name[0] == '\0') return nullptr;
  return TE_GET_SYMBOL(handle, name);
}

void RegisterModuleInit(ModuleInitFn fn) {
  if (fn) g_init_fns.push_back(fn);
}

void RegisterModuleShutdown(ModuleShutdownFn fn) {
  if (fn) g_shutdown_fns.push_back(fn);
}

void RunModuleInit() {
  for (auto fn : g_init_fns) fn();
}

void RunModuleShutdown() {
  for (auto it = g_shutdown_fns.rbegin(); it != g_shutdown_fns.rend(); ++it) (*it)();
}

}  // namespace core
}  // namespace te
