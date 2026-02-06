#include "te_input/BindingTable.hpp"
#include <algorithm>

namespace te_input {

void BindingTable::add_binding(ActionId action, DeviceKind kind, KeyCode code) {
    action_bindings_.push_back({action, kind, code});
}

void BindingTable::add_axis_binding(AxisId axis, DeviceKind kind, KeyCode code) {
    axis_bindings_.push_back({axis, kind, code});
}

void BindingTable::remove_binding(ActionId action, DeviceKind kind, KeyCode code) {
    ActionKey key{action, kind, code};
    auto it = std::find_if(action_bindings_.begin(), action_bindings_.end(),
        [&key](const ActionKey& e) { return e == key; });
    if (it != action_bindings_.end())
        action_bindings_.erase(it);
}

void BindingTable::remove_axis_binding(AxisId axis, DeviceKind kind, KeyCode code) {
    AxisKey key{axis, kind, code};
    auto it = std::find_if(axis_bindings_.begin(), axis_bindings_.end(),
        [&key](const AxisKey& e) { return e == key; });
    if (it != axis_bindings_.end())
        axis_bindings_.erase(it);
}

void BindingTable::remove_all_for_action(ActionId action) {
    action_bindings_.erase(
        std::remove_if(action_bindings_.begin(), action_bindings_.end(),
            [&action](const ActionKey& e) { return e.action == action; }),
        action_bindings_.end());
}

void BindingTable::remove_all_for_axis(AxisId axis) {
    axis_bindings_.erase(
        std::remove_if(axis_bindings_.begin(), axis_bindings_.end(),
            [&axis](const AxisKey& e) { return e.axis == axis; }),
        axis_bindings_.end());
}

std::vector<BindingEntry> BindingTable::get_bindings_for_action(ActionId action) const {
    std::vector<BindingEntry> out;
    for (const auto& e : action_bindings_)
        if (e.action == action)
            out.push_back({e.kind, e.code});
    return out;
}

std::vector<BindingEntry> BindingTable::get_bindings_for_axis(AxisId axis) const {
    std::vector<BindingEntry> out;
    for (const auto& e : axis_bindings_)
        if (e.axis == axis)
            out.push_back({e.kind, e.code});
    return out;
}

} // namespace te_input
