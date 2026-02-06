/**
 * @file lifecycle.hpp
 * @brief Lifecycle: InitializeAll, StartAll, StopAll, ShutdownAll (contract: 007-subsystems-public-api.md).
 * Only contract-declared types and API.
 */
#ifndef TE_SUBSYSTEMS_LIFECYCLE_HPP
#define TE_SUBSYSTEMS_LIFECYCLE_HPP

#include "te/subsystems/registry.hpp"

namespace te {
namespace subsystems {

/**
 * Lifecycle: dependency topology + same-layer Priority.
 * InitializeAll returns false on cycle; main-loop single-threaded only.
 */
class Lifecycle {
public:
    static bool InitializeAll(Registry const& reg);
    static void StartAll(Registry const& reg);
    static void StopAll(Registry const& reg);
    static void ShutdownAll(Registry& reg);
};

}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_LIFECYCLE_HPP
