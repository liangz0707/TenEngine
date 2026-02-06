#include "te_input/Gamepad.hpp"
#include <cstdint>

namespace te_input {

namespace {
std::uint64_t key(DeviceId device, KeyCode code) {
    return (static_cast<std::uint64_t>(device) << 32u) | code;
}
} // namespace

int Gamepad::get_gamepad_count() const {
    return static_cast<int>(connected_.size());
}

bool Gamepad::get_button(DeviceId device, KeyCode button) const {
    auto it = button_state_.find(key(device, button));
    return it != button_state_.end() && it->second;
}

float Gamepad::get_axis(DeviceId device, KeyCode axis) const {
    auto it = axis_state_.find(key(device, axis));
    return it != axis_state_.end() ? it->second : 0.f;
}

void Gamepad::set_vibration(DeviceId device, float left, float right) {
    (void)device;
    (void)left;
    (void)right;
    // Optional; no-op when not supported (implementation-defined).
}

void Gamepad::tick() {
    // Per-frame update from platform gamepad API; connection order defines DeviceId.
}

} // namespace te_input
