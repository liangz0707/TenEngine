/**
 * @file registry_state.hpp
 * @brief Internal state for Registry/Lifecycle coordination. Not part of public API.
 */
#ifndef TE_SUBSYSTEMS_DETAIL_REGISTRY_STATE_HPP
#define TE_SUBSYSTEMS_DETAIL_REGISTRY_STATE_HPP

namespace te {
namespace subsystems {
namespace detail {

bool IsShutdown();
void SetShutdown(bool value);

}  // namespace detail
}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_DETAIL_REGISTRY_STATE_HPP
