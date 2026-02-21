# Contract: 006-Input Module Public API

## Applicable Modules

- **Implementer**: 006-Input (L1; input abstraction, keyboard/mouse/gamepad/touch, raw input, events and mapping)
- **Corresponding Spec**: `docs/module-specs/006-input.md`
- **Dependencies**: 001-Core, 003-Application

## Consumers

- 017-UICore, 024-Editor, 027-XR

## Capability List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifetime |
|------|-----------|----------|
| **KeyCode** | Abstract key/axis code, device-agnostic (`using KeyCode = std::uint32_t;`) | Compile-time definition |
| **DeviceKind** | Device kind enumeration (Keyboard, Mouse, Gamepad, Touch) | Compile-time definition |
| **DeviceId** | Logical device ID (`using DeviceId = std::uint32_t;`); for gamepads: connection-order index 0, 1, 2, ... | Registration until unregister |
| **ActionId** | Action identifier (value type with name string) | Registration until unregister |
| **AxisId** | Axis identifier (value type with name string) | Registration until unregister |
| **TouchPhase** | Touch phase enumeration (Begin, Move, End) | Per-frame or on query |
| **TouchState** | Touch point state (ID, position, phase) | Per-frame or on query |
| **MouseState** | Mouse state (position, delta, buttons) | Per-frame or on query |
| **BindingEntry** | One binding: (DeviceKind, KeyCode) | Binding table lifetime |
| **BindingTable** | Action/axis to (DeviceKind, KeyCode) mappings; multi-bind allowed | Application lifetime |
| **WindowHandle** | Opaque window handle from 003-Application (`using WindowHandle = void*;`) | Application lifetime |

### Capabilities (Provider Guarantees)

#### 1. Input Abstraction (Action/Axis)

| Capability | Description |
|------------|-------------|
| **InputAbstraction** | Action/axis query abstraction; returns current-frame ActionState (bool) and AxisValue (float) from BindingTable and raw state |
| **get_action_state** | Returns current-frame action state: true if any bound key/button is down |
| **get_axis_value** | Returns current-frame axis value; multiple bindings: first bound value (implementation-defined) |
| **tick** | Call once per frame from Application TickCallback to update state from internal event queue |

#### 2. Keyboard and Mouse Input

| Capability | Description |
|------------|-------------|
| **KeyboardMouse** | Keyboard and mouse input; state is per-frame |
| **get_key** | Get key state (current frame pressed) |
| **get_mouse_position** | Get mouse position (MouseState with x, y) |
| **get_mouse_delta** | Get mouse delta (MouseState with dx, dy) |
| **set_capture** | Set mouse capture |
| **focus** | Set focus window (WindowHandle) |
| **tick** | Call once per frame to update key/mouse state from events |

#### 3. Gamepad Input

| Capability | Description |
|------------|-------------|
| **Gamepad** | Gamepad input: multi-controller, buttons/axes, vibration |
| **get_gamepad_count** | Get connected gamepad count |
| **get_button** | Get gamepad button state (DeviceId, KeyCode) |
| **get_axis** | Get gamepad axis value (DeviceId, KeyCode) |
| **set_vibration** | Set gamepad vibration (DeviceId, left, right) |
| **tick** | Call once per frame to update gamepad state |

#### 4. Touch Input

| Capability | Description |
|------------|-------------|
| **TouchInput** | Touch input: multi-touch, get_touch_count / get_touch(id) |
| **get_touch_count** | Get active touch point count |
| **get_touch** | Get touch point state (TouchState) by touch_id |
| **tick** | Call once per frame to update touch state |

#### 5. Binding Configuration

| Capability | Description |
|------------|-------------|
| **BindingTable** | Binding table: action/axis to (DeviceKind, KeyCode) mappings; multi-bind allowed |
| **add_binding** | Add action binding (ActionId, DeviceKind, KeyCode) |
| **add_axis_binding** | Add axis binding (AxisId, DeviceKind, KeyCode) |
| **remove_binding** | Remove action binding |
| **remove_axis_binding** | Remove axis binding |
| **remove_all_for_action** | Remove all bindings for an action |
| **remove_all_for_axis** | Remove all bindings for an axis |
| **get_bindings_for_action** | Get all bindings for an action |
| **get_bindings_for_axis** | Get all bindings for an axis |

#### 6. Device Mapping

| Capability | Description |
|------------|-------------|
| **MapDevice** | Maps physical devices to logical DeviceId/DeviceKind |
| **map_physical_device** | Map a physical device (platform_handle_or_index, DeviceKind, DeviceId) |

#### 7. Configuration Loading

| Capability | Description |
|------------|-------------|
| **LoadConfig** | Load binding config from path or memory into a BindingTable |
| **load_config** | Load from file path; returns true on success |
| **load_config_from_memory** | Load from memory; returns true on success |

## Version / ABI

- Follows Constitution: Public API versioned; breaking changes increment MAJOR.
- Current version: **3.0.0** (code-aligned update)

## Constraints

1. **Initialization Order**: Must be used after Core and Application are available
2. **Event Processing**: tick() should be called once per frame from Application TickCallback
3. **Thread Safety**: State query methods should be thread-safe (protected by mutex)
4. **Multi-Device**: Multi-device and multi-platform behavior is implementation-defined and documented
5. **Focus Management**: Input and focus coordination: when Editor or UI gains focus, game logic may not receive input (implementation-defined)

## Namespace and Header Files

- **Namespace**: `te_input`
- **Header Files**:
  - `te_input/Abstraction.hpp`: InputAbstraction class
  - `te_input/ActionId.hpp`: ActionId struct
  - `te_input/AxisId.hpp`: AxisId struct
  - `te_input/BindingTable.hpp`: BindingTable class, BindingEntry struct
  - `te_input/DeviceId.hpp`: DeviceId type
  - `te_input/DeviceKind.hpp`: DeviceKind enum
  - `te_input/Gamepad.hpp`: Gamepad class
  - `te_input/KeyboardMouse.hpp`: KeyboardMouse class, WindowHandle type
  - `te_input/KeyCode.hpp`: KeyCode type
  - `te_input/LoadConfig.hpp`: LoadConfig class
  - `te_input/MapDevice.hpp`: MapDevice class
  - `te_input/MouseState.hpp`: MouseState struct
  - `te_input/Touch.hpp`: TouchInput class, TouchPhase enum, TouchState struct

## Change Log

| Date | Change Description |
|------|-------------------|
| T0 Initial | 006-Input contract |
| 2026-02-05 | Unified directory; capability list in table format |
| 2026-02-06 | Redesign: consolidated interfaces, improved event handling, enhanced state query, unified naming conventions |
| 2026-02-22 | Code-aligned update: namespace `te_input`, actual types and classes as implemented (InputAbstraction, BindingTable, Gamepad, KeyboardMouse, TouchInput, MapDevice, LoadConfig), removed IInput interface (not in code), added WindowHandle type |
