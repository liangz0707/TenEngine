/**
 * @file registry_detail.hpp
 * @brief Internal Registry access for Lifecycle. Not part of public API.
 */
#ifndef TE_SUBSYSTEMS_DETAIL_REGISTRY_DETAIL_HPP
#define TE_SUBSYSTEMS_DETAIL_REGISTRY_DETAIL_HPP

#include "te/subsystems/descriptor.hpp"
#include "te/subsystems/subsystem.hpp"

#include <utility>
#include <vector>

namespace te {
namespace subsystems {
namespace detail {

/** (descriptor, instance ptr) for lifecycle ordering. */
using SubsystemEntry = std::pair<SubsystemDescriptor, ISubsystem*>;

/** Get all registered entries for lifecycle iteration. */
std::vector<SubsystemEntry> GetEntriesForLifecycle();

}  // namespace detail
}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_DETAIL_REGISTRY_DETAIL_HPP
