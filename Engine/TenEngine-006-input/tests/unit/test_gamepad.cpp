#include "te_input/Gamepad.hpp"
#include "te_input/DeviceId.hpp"
#include "te_input/KeyCode.hpp"

int main() {
    te_input::Gamepad gp;
    if (gp.get_gamepad_count() != 0) return 1;
    if (gp.get_button(0u, 0u)) return 2;
    if (gp.get_axis(0u, 0u) != 0.f) return 3;
    gp.set_vibration(0u, 0.f, 0.f);
    gp.tick();
    return 0;
}
