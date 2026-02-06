#ifndef TE_INPUT_DEVICE_ID_HPP
#define TE_INPUT_DEVICE_ID_HPP

#include <cstdint>

namespace te_input {

/// Logical device ID (contract-declared).
/// For gamepads: connection-order index 0, 1, 2, ...
/// Reconnection may change the index; behaviour is implementation-defined and documented.
using DeviceId = std::uint32_t;

} // namespace te_input

#endif
