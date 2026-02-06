#ifndef TE_INPUT_GAMEPAD_HPP
#define TE_INPUT_GAMEPAD_HPP

#include "te_input/DeviceId.hpp"
#include "te_input/KeyCode.hpp"
#include <unordered_map>
#include <vector>

namespace te_input {

/// Gamepad input: multi-controller, buttons/axes, vibration (contract-declared).
/// DeviceId is connection-order index 0, 1, 2, ...; may change on reconnection (implementation-defined).
class Gamepad {
public:
    Gamepad() = default;

    int get_gamepad_count() const;
    bool get_button(DeviceId device, KeyCode button) const;
    float get_axis(DeviceId device, KeyCode axis) const;
    void set_vibration(DeviceId device, float left, float right);

    void tick();

private:
    std::vector<bool> connected_;
    std::unordered_map<std::uint64_t, bool> button_state_;
    std::unordered_map<std::uint64_t, float> axis_state_;
};

} // namespace te_input

#endif
