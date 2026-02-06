/**
 * @file InputTypes.h
 * @brief Input type definitions (contract: specs/_contracts/006-input-ABI.md).
 * Only types declared in the contract are exposed.
 */
#ifndef TE_INPUT_INPUT_TYPES_H
#define TE_INPUT_INPUT_TYPES_H

#include <cstdint>

namespace te {
namespace input {

/**
 * @brief Key code enumeration per contract.
 * Device-agnostic key codes, decoupled from physical devices.
 */
enum class KeyCode : uint32_t {
    // Letters
    A = 0, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    // Numbers
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    
    // Modifiers
    LeftShift, RightShift,
    LeftControl, RightControl,
    LeftAlt, RightAlt,
    LeftSuper, RightSuper,  // Windows/Command key
    
    // Special keys
    Space,
    Enter, Return = Enter,
    Escape,
    Tab,
    Backspace,
    Delete,
    Insert,
    Home,
    End,
    PageUp,
    PageDown,
    
    // Arrow keys
    Up,
    Down,
    Left,
    Right,
    
    // Numpad
    Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
    Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
    NumpadEnter,
    NumpadAdd,
    NumpadSubtract,
    NumpadMultiply,
    NumpadDivide,
    NumpadDecimal,
    
    // Other
    Semicolon,      // ;
    Equals,         // =
    Comma,          // ,
    Minus,          // -
    Period,         // .
    Slash,          // /
    Grave,          // `
    LeftBracket,    // [
    Backslash,      // \
    RightBracket,   // ]
    Apostrophe,     // '
    
    // Count (must be last)
    Count
};

/**
 * @brief Mouse button enumeration per contract.
 */
enum class MouseButton : uint8_t {
    Left = 0,
    Right,
    Middle,
    Button4,
    Button5,
    Count
};

/**
 * @brief Gamepad button enumeration per contract.
 */
enum class GamepadButton : uint8_t {
    A = 0,
    B,
    X,
    Y,
    LeftShoulder,   // LB / L1
    RightShoulder,  // RB / R1
    Back,           // Select / View
    Start,          // Menu / Options
    LeftStick,      // L3
    RightStick,     // R3
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,
    Count
};

/**
 * @brief Gamepad axis enumeration per contract.
 */
enum class GamepadAxis : uint8_t {
    LeftStickX = 0,
    LeftStickY,
    RightStickX,
    RightStickY,
    LeftTrigger,    // LT / L2
    RightTrigger,   // RT / R2
    Count
};

/**
 * @brief Touch phase enumeration per contract.
 */
enum class TouchPhase : uint8_t {
    Begin,  // Touch started
    Move,   // Touch moved
    End,    // Touch ended
    Cancel  // Touch cancelled
};

/**
 * @brief Touch state structure per contract.
 */
struct TouchState {
    /** Touch point ID (stable from Begin to End). */
    uint32_t touchId = 0;
    
    /** Touch X position. */
    float x = 0.0f;
    
    /** Touch Y position. */
    float y = 0.0f;
    
    /** Touch phase. */
    TouchPhase phase = TouchPhase::End;
};

/**
 * @brief Action ID type per contract.
 * Used for input abstraction (action/axis binding).
 */
using ActionId = uint32_t;

/**
 * @brief Axis ID type per contract.
 * Used for input abstraction (action/axis binding).
 */
using AxisId = uint32_t;

}  // namespace input
}  // namespace te

#endif  // TE_INPUT_INPUT_TYPES_H
