#ifndef TE_INPUT_ABSTRACTION_HPP
#define TE_INPUT_ABSTRACTION_HPP

#include "te_input/ActionId.hpp"
#include "te_input/AxisId.hpp"
#include "te_input/BindingTable.hpp"
#include "te_input/KeyCode.hpp"
#include <cstdint>
#include <unordered_map>

namespace te_input {

/// Action/axis query abstraction (contract-declared).
/// Returns current-frame ActionState (bool) and AxisValue (float) from BindingTable and raw state.
/// Call tick() from Application TickCallback to update state from events (implementation-defined event feed).
class InputAbstraction {
public:
    explicit InputAbstraction(BindingTable const* table);

    /// Returns current-frame action state: true if any bound key/button is down.
    bool get_action_state(ActionId action) const;

    /// Returns current-frame axis value; multiple bindings: first bound value (implementation-defined).
    float get_axis_value(AxisId axis) const;

    /// Call once per frame from Application TickCallback to update state from internal event queue.
    void tick();

private:
    void set_key_state(KeyCode code, bool pressed);
    void set_axis_state(KeyCode code, float value);

    BindingTable const* table_{nullptr};
    std::unordered_map<std::uint32_t, bool> key_state_;
    std::unordered_map<std::uint32_t, float> axis_state_;
};

} // namespace te_input

#endif
