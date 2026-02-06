#include "te_input/KeyboardMouse.hpp"
#include "te_input/KeyCode.hpp"
#include "te_input/MouseState.hpp"

int main() {
    te_input::KeyboardMouse km;
    if (km.get_key(32u)) return 1;
    te_input::MouseState pos = km.get_mouse_position();
    if (pos.x != 0.f || pos.y != 0.f) return 2;
    te_input::MouseState delta = km.get_mouse_delta();
    if (delta.dx != 0.f || delta.dy != 0.f) return 3;
    km.set_capture();
    km.focus(nullptr);
    km.tick();
    return 0;
}
