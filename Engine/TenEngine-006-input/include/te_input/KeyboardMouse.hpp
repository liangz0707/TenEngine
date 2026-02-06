#ifndef TE_INPUT_KEYBOARD_MOUSE_HPP
#define TE_INPUT_KEYBOARD_MOUSE_HPP

#include "te_input/KeyCode.hpp"
#include "te_input/MouseState.hpp"
#include <unordered_map>

namespace te_input {

/// Opaque window handle from 003-Application contract; use when linked with Application.
using WindowHandle = void*;

/// Keyboard and mouse input (contract-declared).
/// State is per-frame; call tick() from Application TickCallback to update from events.
class KeyboardMouse {
public:
    KeyboardMouse() = default;

    bool get_key(KeyCode code) const;
    MouseState get_mouse_position() const;
    MouseState get_mouse_delta() const;
    void set_capture();
    void focus(WindowHandle window);

    /// Call once per frame from Application TickCallback to update key/mouse state from events.
    void tick();

private:
    MouseState mouse_position_{};
    MouseState mouse_delta_{};
    std::unordered_map<KeyCode, bool> key_state_;
};

} // namespace te_input

#endif
