/**
 * @file discovery.cpp
 * @brief Discovery implementation (contract: 007-subsystems-public-api.md).
 * Uses 001-Core module_load (LoadLibrary, GetSymbol) per contract.
 */
#include "te/subsystems/discovery.hpp"
#include "te/core/module_load.h"

namespace te {
namespace subsystems {

namespace {
/** Plugin export convention: void fn(Registry*). */
using RegisterFn = void (*)(Registry*);
constexpr char const* kRegisterSymbol = "te_subsystems_register";
}  // namespace

bool Discovery::ScanPlugins(Registry&) {
    /* Contract: ScanPlugins scans loaded plugins. Core does not expose "list loaded modules".
     * Minimal impl: no-op; return true. Caller may use RegisterFromPlugin per loaded module. */
    return true;
}

bool Discovery::RegisterFromPlugin(Registry& reg, void* moduleHandle) {
    if (!moduleHandle)
        return false;
    auto* fn = reinterpret_cast<RegisterFn>(
        te::core::GetSymbol(static_cast<te::core::ModuleHandle>(moduleHandle), kRegisterSymbol));
    if (!fn)
        return false;
    fn(&reg);
    return true;
}

}  // namespace subsystems
}  // namespace te
