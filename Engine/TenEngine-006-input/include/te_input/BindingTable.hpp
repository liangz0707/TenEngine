#ifndef TE_INPUT_BINDING_TABLE_HPP
#define TE_INPUT_BINDING_TABLE_HPP

#include "te_input/ActionId.hpp"
#include "te_input/AxisId.hpp"
#include "te_input/DeviceKind.hpp"
#include "te_input/KeyCode.hpp"
#include <utility>
#include <vector>

namespace te_input {

/// One binding: (DeviceKind, KeyCode). Contract-declared only.
struct BindingEntry {
    DeviceKind kind{};
    KeyCode code{};
};

/// Binding table: action/axis to (DeviceKind, KeyCode) mappings; multi-bind allowed (contract-declared only).
class BindingTable {
public:
    BindingTable() = default;

    void add_binding(ActionId action, DeviceKind kind, KeyCode code);
    void add_axis_binding(AxisId axis, DeviceKind kind, KeyCode code);
    void remove_binding(ActionId action, DeviceKind kind, KeyCode code);
    void remove_axis_binding(AxisId axis, DeviceKind kind, KeyCode code);
    void remove_all_for_action(ActionId action);
    void remove_all_for_axis(AxisId axis);

    std::vector<BindingEntry> get_bindings_for_action(ActionId action) const;
    std::vector<BindingEntry> get_bindings_for_axis(AxisId axis) const;

private:
    struct ActionKey {
        ActionId action;
        DeviceKind kind;
        KeyCode code;
        bool operator==(const ActionKey& o) const {
            return action == o.action && kind == o.kind && code == o.code;
        }
    };
    struct AxisKey {
        AxisId axis;
        DeviceKind kind;
        KeyCode code;
        bool operator==(const AxisKey& o) const {
            return axis == o.axis && kind == o.kind && code == o.code;
        }
    };
    std::vector<ActionKey> action_bindings_;
    std::vector<AxisKey> axis_bindings_;
};

} // namespace te_input

#endif
