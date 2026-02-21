# 006-Input Module ABI

- **Contract**: [006-input-public-api.md](./006-input-public-api.md) (capabilities and types description)
- **This file**: 006-Input external ABI explicit table.
- **CMake Target Name**: **`te_input`** (project name `te_input`). Downstream should use **`te_input`** in `target_link_libraries`.
- **Naming**: Member methods and free functions use **snake_case**; all methods have complete function signatures in the description column.
- **Namespace**: te_input (header path te_input/).

## ABI Table

Column definitions: **Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description**

### Types and Enums

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 006-Input | te_input | - | using | KeyCode type | te_input/KeyCode.hpp | KeyCode | `using KeyCode = std::uint32_t;` Abstract key/axis code, device-agnostic |
| 006-Input | te_input | - | using | DeviceId type | te_input/DeviceId.hpp | DeviceId | `using DeviceId = std::uint32_t;` Logical device ID; for gamepads: connection-order index 0, 1, 2, ... |
| 006-Input | te_input | - | using | WindowHandle type | te_input/KeyboardMouse.hpp | WindowHandle | `using WindowHandle = void*;` Opaque window handle from 003-Application contract |
| 006-Input | te_input | - | enum class | Device kind | te_input/DeviceKind.hpp | DeviceKind | `enum class DeviceKind { Keyboard, Mouse, Gamepad, Touch };` Device kind for binding table |
| 006-Input | te_input | - | enum class | Touch phase | te_input/Touch.hpp | TouchPhase | `enum class TouchPhase { Begin, Move, End };` Touch phase (contract-declared) |

### Structs

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 006-Input | te_input | - | struct | Action identifier | te_input/ActionId.hpp | ActionId | `struct ActionId { std::string name_; explicit ActionId(std::string name); static ActionId from_name(std::string_view name); bool operator==(const ActionId&) const; bool operator!=(const ActionId&) const; };` Action identifier: value type, comparable |
| 006-Input | te_input | - | struct | Axis identifier | te_input/AxisId.hpp | AxisId | `struct AxisId { std::string name_; explicit AxisId(std::string name); static AxisId from_name(std::string_view name); bool operator==(const AxisId&) const; bool operator!=(const AxisId&) const; };` Axis identifier: value type, comparable |
| 006-Input | te_input | - | struct | Mouse state | te_input/MouseState.hpp | MouseState | `struct MouseState { float x{0.f}; float y{0.f}; float dx{0.f}; float dy{0.f}; unsigned buttons{0}; };` Position, delta, buttons |
| 006-Input | te_input | - | struct | Touch state | te_input/Touch.hpp | TouchState | `struct TouchState { int touch_id{0}; float x{0.f}; float y{0.f}; TouchPhase phase{TouchPhase::End}; };` Touch point state |
| 006-Input | te_input | - | struct | Binding entry | te_input/BindingTable.hpp | BindingEntry | `struct BindingEntry { DeviceKind kind{}; KeyCode code{}; };` One binding: (DeviceKind, KeyCode) |

### BindingTable Class

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 006-Input | te_input | BindingTable | class | Binding table | te_input/BindingTable.hpp | BindingTable::BindingTable | `BindingTable() = default;` Default constructor |
| 006-Input | te_input | BindingTable | member | Add action binding | te_input/BindingTable.hpp | BindingTable::add_binding | `void add_binding(ActionId action, DeviceKind kind, KeyCode code);` |
| 006-Input | te_input | BindingTable | member | Add axis binding | te_input/BindingTable.hpp | BindingTable::add_axis_binding | `void add_axis_binding(AxisId axis, DeviceKind kind, KeyCode code);` |
| 006-Input | te_input | BindingTable | member | Remove action binding | te_input/BindingTable.hpp | BindingTable::remove_binding | `void remove_binding(ActionId action, DeviceKind kind, KeyCode code);` |
| 006-Input | te_input | BindingTable | member | Remove axis binding | te_input/BindingTable.hpp | BindingTable::remove_axis_binding | `void remove_axis_binding(AxisId axis, DeviceKind kind, KeyCode code);` |
| 006-Input | te_input | BindingTable | member | Remove all for action | te_input/BindingTable.hpp | BindingTable::remove_all_for_action | `void remove_all_for_action(ActionId action);` |
| 006-Input | te_input | BindingTable | member | Remove all for axis | te_input/BindingTable.hpp | BindingTable::remove_all_for_axis | `void remove_all_for_axis(AxisId axis);` |
| 006-Input | te_input | BindingTable | member | Get bindings for action | te_input/BindingTable.hpp | BindingTable::get_bindings_for_action | `std::vector<BindingEntry> get_bindings_for_action(ActionId action) const;` |
| 006-Input | te_input | BindingTable | member | Get bindings for axis | te_input/BindingTable.hpp | BindingTable::get_bindings_for_axis | `std::vector<BindingEntry> get_bindings_for_axis(AxisId axis) const;` |

### InputAbstraction Class

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 006-Input | te_input | InputAbstraction | class | Input abstraction | te_input/Abstraction.hpp | InputAbstraction::InputAbstraction | `explicit InputAbstraction(BindingTable const* table);` Constructor with binding table |
| 006-Input | te_input | InputAbstraction | member | Get action state | te_input/Abstraction.hpp | InputAbstraction::get_action_state | `bool get_action_state(ActionId action) const;` Returns true if any bound key/button is down |
| 006-Input | te_input | InputAbstraction | member | Get axis value | te_input/Abstraction.hpp | InputAbstraction::get_axis_value | `float get_axis_value(AxisId axis) const;` Returns current-frame axis value |
| 006-Input | te_input | InputAbstraction | member | Tick | te_input/Abstraction.hpp | InputAbstraction::tick | `void tick();` Call once per frame to update state from event queue |

### KeyboardMouse Class

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 006-Input | te_input | KeyboardMouse | class | Keyboard and mouse | te_input/KeyboardMouse.hpp | KeyboardMouse::KeyboardMouse | `KeyboardMouse() = default;` Default constructor |
| 006-Input | te_input | KeyboardMouse | member | Get key | te_input/KeyboardMouse.hpp | KeyboardMouse::get_key | `bool get_key(KeyCode code) const;` Get key state (current frame pressed) |
| 006-Input | te_input | KeyboardMouse | member | Get mouse position | te_input/KeyboardMouse.hpp | KeyboardMouse::get_mouse_position | `MouseState get_mouse_position() const;` Returns MouseState with x, y |
| 006-Input | te_input | KeyboardMouse | member | Get mouse delta | te_input/KeyboardMouse.hpp | KeyboardMouse::get_mouse_delta | `MouseState get_mouse_delta() const;` Returns MouseState with dx, dy |
| 006-Input | te_input | KeyboardMouse | member | Set capture | te_input/KeyboardMouse.hpp | KeyboardMouse::set_capture | `void set_capture();` Set mouse capture |
| 006-Input | te_input | KeyboardMouse | member | Focus | te_input/KeyboardMouse.hpp | KeyboardMouse::focus | `void focus(WindowHandle window);` Set focus window |
| 006-Input | te_input | KeyboardMouse | member | Tick | te_input/KeyboardMouse.hpp | KeyboardMouse::tick | `void tick();` Call once per frame to update state |

### Gamepad Class

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 006-Input | te_input | Gamepad | class | Gamepad input | te_input/Gamepad.hpp | Gamepad::Gamepad | `Gamepad() = default;` Default constructor |
| 006-Input | te_input | Gamepad | member | Get gamepad count | te_input/Gamepad.hpp | Gamepad::get_gamepad_count | `int get_gamepad_count() const;` Returns connected gamepad count |
| 006-Input | te_input | Gamepad | member | Get button | te_input/Gamepad.hpp | Gamepad::get_button | `bool get_button(DeviceId device, KeyCode button) const;` Returns button state |
| 006-Input | te_input | Gamepad | member | Get axis | te_input/Gamepad.hpp | Gamepad::get_axis | `float get_axis(DeviceId device, KeyCode axis) const;` Returns axis value |
| 006-Input | te_input | Gamepad | member | Set vibration | te_input/Gamepad.hpp | Gamepad::set_vibration | `void set_vibration(DeviceId device, float left, float right);` Set vibration |
| 006-Input | te_input | Gamepad | member | Tick | te_input/Gamepad.hpp | Gamepad::tick | `void tick();` Call once per frame to update state |

### TouchInput Class

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 006-Input | te_input | TouchInput | class | Touch input | te_input/Touch.hpp | TouchInput::TouchInput | `TouchInput() = default;` Default constructor |
| 006-Input | te_input | TouchInput | member | Get touch count | te_input/Touch.hpp | TouchInput::get_touch_count | `int get_touch_count() const;` Returns active touch point count |
| 006-Input | te_input | TouchInput | member | Get touch | te_input/Touch.hpp | TouchInput::get_touch | `TouchState get_touch(int touch_id) const;` Get touch point state by ID |
| 006-Input | te_input | TouchInput | member | Tick | te_input/Touch.hpp | TouchInput::tick | `void tick();` Call once per frame to update state |

### MapDevice Class

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 006-Input | te_input | MapDevice | class | Device mapping | te_input/MapDevice.hpp | MapDevice::map_physical_device | `void map_physical_device(void* platform_handle_or_index, DeviceKind kind, DeviceId id);` Map physical device to logical ID |

### LoadConfig Class

| Module | Namespace | Class | Export Form | Interface Description | Header | Symbol | Description |
|--------|-----------|-------|-------------|----------------------|--------|--------|-------------|
| 006-Input | te_input | LoadConfig | class | Config loader | te_input/LoadConfig.hpp | LoadConfig::LoadConfig | `explicit LoadConfig(BindingTable& table);` Constructor with binding table |
| 006-Input | te_input | LoadConfig | member | Load from file | te_input/LoadConfig.hpp | LoadConfig::load_config | `bool load_config(char const* path);` Returns true on success |
| 006-Input | te_input | LoadConfig | member | Load from memory | te_input/LoadConfig.hpp | LoadConfig::load_config_from_memory | `bool load_config_from_memory(void const* data, std::size_t size);` Returns true on success |

*Source: User Story US-input-001 (Input Polling and Events).*

*Version: 3.0.0 (2026-02-22 code-aligned update)*

*Change Notes:*
- *Namespace: Updated to `te_input` (as implemented in code)*
- *Types: KeyCode, DeviceId, DeviceKind, ActionId, AxisId, TouchPhase, TouchState, MouseState, BindingEntry, WindowHandle*
- *Classes: InputAbstraction, BindingTable, KeyboardMouse, Gamepad, TouchInput, MapDevice, LoadConfig (as implemented)*
- *Removed: IInput interface (not in code)*
- *Method naming: snake_case (as implemented)*

## Change Log

| Date | Change Description |
|------|-------------------|
| T0 Initial | 006-Input initial ABI |
| 2026-02-06 | Redesign: IInput interface, ProcessEvents method, state query enhancements |
| 2026-02-22 | Code-aligned update: namespace `te_input`, actual types and classes as implemented, snake_case methods, removed IInput interface |
