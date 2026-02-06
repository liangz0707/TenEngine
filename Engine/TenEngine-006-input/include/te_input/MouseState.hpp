#ifndef TE_INPUT_MOUSE_STATE_HPP
#define TE_INPUT_MOUSE_STATE_HPP

namespace te_input {

/// Mouse state: position, delta, buttons (contract-declared; fields implementation-defined).
/// Valid per-frame or on query after tick.
struct MouseState {
    float x{0.f};
    float y{0.f};
    float dx{0.f};
    float dy{0.f};
    /// Button bits: implementation-defined (e.g. bit0=left, bit1=right, bit2=middle).
    unsigned buttons{0};
};

} // namespace te_input

#endif
