#ifndef TE_INPUT_KEY_CODE_HPP
#define TE_INPUT_KEY_CODE_HPP

#include <cstdint>

namespace te_input {

/// Abstract key/axis code, device-agnostic (contract-declared only).
using KeyCode = std::uint32_t;

} // namespace te_input

#endif
