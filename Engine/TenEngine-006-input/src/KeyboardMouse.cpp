#include "te_input/KeyboardMouse.hpp"

namespace te_input {

bool KeyboardMouse::get_key(KeyCode code) const {
    auto it = key_state_.find(code);
    return it != key_state_.end() && it->second;
}

MouseState KeyboardMouse::get_mouse_position() const {
    return mouse_position_;
}

MouseState KeyboardMouse::get_mouse_delta() const {
    return mouse_delta_;
}

void KeyboardMouse::set_capture() {
    // Capture mouse to current application main window; implementation-defined (e.g. platform API when linked with Application).
}

void KeyboardMouse::focus(WindowHandle window) {
    (void)window;
    // Set keyboard/mouse focus to given window; implementation-defined when linked with Application.
}

void KeyboardMouse::tick() {
    // Per-frame update from Application events; event feed is implementation-defined.
}

} // namespace te_input
