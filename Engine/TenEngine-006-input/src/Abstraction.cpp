#include "te_input/Abstraction.hpp"
#include "te_input/BindingTable.hpp"

namespace te_input {

InputAbstraction::InputAbstraction(BindingTable const* table) : table_(table) {}

bool InputAbstraction::get_action_state(ActionId action) const {
    if (!table_) return false;
    auto entries = table_->get_bindings_for_action(action);
    for (auto const& e : entries) {
        auto it = key_state_.find(e.code);
        if (it != key_state_.end() && it->second)
            return true;
    }
    return false;
}

float InputAbstraction::get_axis_value(AxisId axis) const {
    if (!table_) return 0.f;
    auto entries = table_->get_bindings_for_axis(axis);
    for (auto const& e : entries) {
        auto it = axis_state_.find(e.code);
        if (it != axis_state_.end())
            return it->second;
    }
    return 0.f;
}

void InputAbstraction::set_key_state(KeyCode code, bool pressed) {
    key_state_[code] = pressed;
}

void InputAbstraction::set_axis_state(KeyCode code, float value) {
    axis_state_[code] = value;
}

void InputAbstraction::tick() {
    // Per-frame update from events. Event feed is implementation-defined (e.g. Application pushes events before calling tick).
    // Stub: no event queue processing here; state is set by tests or by a later integration layer.
}

} // namespace te_input
