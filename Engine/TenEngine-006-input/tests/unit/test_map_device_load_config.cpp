#include "te_input/MapDevice.hpp"
#include "te_input/LoadConfig.hpp"
#include "te_input/Abstraction.hpp"
#include "te_input/BindingTable.hpp"
#include "te_input/ActionId.hpp"
#include "te_input/AxisId.hpp"
#include "te_input/DeviceKind.hpp"
#include "te_input/KeyCode.hpp"

int main() {
    // MapDevice: map_physical_device per contract
    te_input::MapDevice map_dev;
    map_dev.map_physical_device(reinterpret_cast<void*>(1), te_input::DeviceKind::Gamepad, 0u);
    map_dev.map_physical_device(reinterpret_cast<void*>(2), te_input::DeviceKind::Gamepad, 1u);

    // LoadConfig: load_config_from_memory fills BindingTable (config format: implementation-defined)
    te_input::BindingTable table;
    te_input::LoadConfig loader(table);
    const char config[] = "action jump 0 32\naxis move 0 87\n";
    if (!loader.load_config_from_memory(config, sizeof(config) - 1)) return 1;
    auto jump_bindings = table.get_bindings_for_action(te_input::ActionId::from_name("jump"));
    if (jump_bindings.size() != 1u) return 2;
    auto move_bindings = table.get_bindings_for_axis(te_input::AxisId::from_name("move"));
    if (move_bindings.size() != 1u) return 3;

    // InputAbstraction: get_action_state, get_axis_value (contract)
    te_input::InputAbstraction abstraction(&table);
    if (abstraction.get_action_state(te_input::ActionId::from_name("jump"))) return 4; // not pressed yet
    if (abstraction.get_axis_value(te_input::AxisId::from_name("move")) != 0.f) return 5;
    abstraction.tick(); // no-op stub; state can be set by integration layer
    return 0;
}
