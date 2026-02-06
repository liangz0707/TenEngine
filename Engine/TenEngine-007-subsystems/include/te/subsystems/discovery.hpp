/**
 * @file discovery.hpp
 * @brief Discovery: ScanPlugins, RegisterFromPlugin (contract: 007-subsystems-public-api.md).
 * Only contract-declared types and API.
 */
#ifndef TE_SUBSYSTEMS_DISCOVERY_HPP
#define TE_SUBSYSTEMS_DISCOVERY_HPP

#include "te/subsystems/registry.hpp"

namespace te {
namespace subsystems {

/**
 * Discovery: scan plugins and register subsystems; works with Core ModuleLoad.
 * moduleHandle: Core module handle (e.g. LoadLibrary return).
 */
class Discovery {
public:
    static bool ScanPlugins(Registry& reg);
    static bool RegisterFromPlugin(Registry& reg, void* moduleHandle);
};

}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_DISCOVERY_HPP
