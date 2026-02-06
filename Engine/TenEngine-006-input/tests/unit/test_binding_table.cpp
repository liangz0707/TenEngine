#include "te_input/BindingTable.hpp"
#include "te_input/ActionId.hpp"
#include "te_input/AxisId.hpp"
#include "te_input/DeviceKind.hpp"
#include "te_input/KeyCode.hpp"

int main() {
    te_input::BindingTable table;

    auto jump = te_input::ActionId::from_name("jump");
    auto move = te_input::AxisId::from_name("move");

    table.add_binding(jump, te_input::DeviceKind::Keyboard, 32u);
    table.add_binding(jump, te_input::DeviceKind::Gamepad, 0u);
    table.add_axis_binding(move, te_input::DeviceKind::Keyboard, 87u);
    table.add_axis_binding(move, te_input::DeviceKind::Keyboard, 83u);

    auto jump_bindings = table.get_bindings_for_action(jump);
    if (jump_bindings.size() != 2u) return 1;
    auto move_bindings = table.get_bindings_for_axis(move);
    if (move_bindings.size() != 2u) return 2;

    table.remove_binding(jump, te_input::DeviceKind::Keyboard, 32u);
    jump_bindings = table.get_bindings_for_action(jump);
    if (jump_bindings.size() != 1u) return 3;

    table.remove_all_for_axis(move);
    move_bindings = table.get_bindings_for_axis(move);
    if (!move_bindings.empty()) return 4;

    auto empty = te_input::ActionId::from_name("none");
    auto empty_bindings = table.get_bindings_for_action(empty);
    if (!empty_bindings.empty()) return 5;

    return 0;
}
