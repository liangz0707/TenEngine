/**
 * @file module_load.h
 * @brief LoadLibrary, UnloadLibrary, GetSymbol, ModuleInit/Shutdown (contract: 001-core-public-api.md).
 * Only contract-declared types and API.
 */
#ifndef TE_CORE_MODULE_LOAD_H
#define TE_CORE_MODULE_LOAD_H

namespace te {
namespace core {

/** Opaque module handle from LoadLibrary. */
using ModuleHandle = void*;

/** Load dynamic library; returns nullptr on failure. */
ModuleHandle LoadLibrary(char const* path);

/** Unload dynamic library; no-op for nullptr. */
void UnloadLibrary(ModuleHandle handle);

/** Get symbol address; returns nullptr if not found. */
void* GetSymbol(ModuleHandle handle, char const* name);

/** Module init callback type. */
using ModuleInitFn = void (*)();

/** Module shutdown callback type. */
using ModuleShutdownFn = void (*)();

/** Register init callback (called in registration order on RunModuleInit). */
void RegisterModuleInit(ModuleInitFn fn);

/** Register shutdown callback (called in reverse order on RunModuleShutdown). */
void RegisterModuleShutdown(ModuleShutdownFn fn);

/** Run all registered init callbacks. Call after loading modules. */
void RunModuleInit();

/** Run all registered shutdown callbacks. Call before unloading. */
void RunModuleShutdown();

}  // namespace core
}  // namespace te

#endif  // TE_CORE_MODULE_LOAD_H
