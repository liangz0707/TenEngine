#ifndef TE_INPUT_DEVICE_KIND_HPP
#define TE_INPUT_DEVICE_KIND_HPP

namespace te_input {

/// Device kind for binding table (contract-declared only).
enum class DeviceKind {
    Keyboard,
    Mouse,
    Gamepad,
    Touch,
};

} // namespace te_input

#endif
