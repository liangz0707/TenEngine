#include "te_input/ActionId.hpp"
#include "te_input/AxisId.hpp"
#include "te_input/BindingTable.hpp"
#include "te_input/DeviceKind.hpp"
#include "te_input/KeyCode.hpp"

static int run_action_axis_id_tests() {
    auto a1 = te_input::ActionId::from_name("jump");
    auto a2 = te_input::ActionId::from_name("jump");
    auto a3 = te_input::ActionId::from_name("fire");
    if (a1 != a2) return 1;
    if (a1 == a3) return 2;

    auto x1 = te_input::AxisId::from_name("move");
    auto x2 = te_input::AxisId::from_name("move");
    auto x3 = te_input::AxisId::from_name("look");
    if (x1 != x2) return 3;
    if (x1 == x3) return 4;

    return 0;
}

static int run_binding_table_tests() {
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

int main() {
    int r = run_action_axis_id_tests();
    if (r != 0) return r;
    return run_binding_table_tests();
}
