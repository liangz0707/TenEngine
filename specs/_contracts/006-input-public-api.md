# 契约：006-Input 模块对外 API

## 适用模块

- **实现方**：**006-Input**（T0 输入抽象与设备映射）
- **对应规格**：`docs/module-specs/006-input.md`
- **依赖**：001-Core（001-core-public-api）, 003-Application（003-application-public-api）

## 消费者（T0 下游）

- 017-UICore, 024-Editor, 027-XR。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ActionState / AxisValue | 动作/轴抽象后的状态与数值 | 每帧或按查询 |
| KeyCode / MouseState | 键盘键、鼠标位置/增量/按钮、捕获与焦点 | 每帧或按查询 |
| GamepadState | 手柄按钮/摇杆/扳机、多手柄 ID、震动 | 每帧或按查询 |
| TouchState | 触摸点 ID、位置、阶段（Begin/Move/End） | 每帧或按查询 |
| 绑定表 / 配置 | 动作/轴与物理设备映射；可序列化或从配置加载 | 按实现约定 |

## 能力列表（提供方保证）

1. **输入抽象**：ActionId、AxisId、BindingTable、MapDevice、LoadConfig；与物理设备解耦。
2. **键鼠**：GetKey、GetMousePosition、GetMouseDelta、SetCapture、Focus。
3. **手柄**：GetGamepadCount、GetButton、GetAxis、SetVibration、DeviceId。
4. **触摸**：GetTouchCount、GetTouch(id)、Phase；与平台触摸事件对接。

## API 雏形（完整模块：plan 006-input-fullversion-001）

**来源**：plan 006-input-fullversion-001。技术栈 C++17、CMake；本 feature 实现完整模块，暴露以下类型与 API。

### 类型（跨边界）

| 类型名 | 语义 | 说明 |
|--------|------|------|
| `te_input::ActionId` | 动作标识 | 值类型，`from_name(std::string_view)` 构造；支持 `operator==`/`operator!=`、可拷贝。无中央注册表。 |
| `te_input::AxisId` | 轴标识 | 同上，用于逻辑轴。 |
| `te_input::DeviceKind` | 设备种类 | 枚举：`Keyboard`, `Mouse`, `Gamepad`, `Touch`。 |
| `te_input::KeyCode` | 抽象键/轴码 | `std::uint32_t`，与设备解耦。 |
| `te_input::BindingEntry` | 单条绑定 | `struct { DeviceKind kind; KeyCode code; }`。 |
| `te_input::BindingTable` | 绑定表 | 动作/轴 到 (DeviceKind, KeyCode) 的映射集合；同一 ActionId/AxisId 允许多绑定。 |
| `te_input::ActionState` | 动作状态 | 语义为 `bool`：按下 true，否则 false；每帧或按查询。 |
| `te_input::AxisValue` | 轴数值 | 语义为 `float`；每帧或按查询。 |
| `te_input::DeviceId` | 逻辑设备 ID | 手柄为连接顺序索引（0, 1, 2…），重连后可能变化，由实现文档化。 |
| `te_input::MouseState` | 鼠标状态 | 位置、增量、按钮等；具体字段由实现定义并文档化；每帧或按查询。 |
| `te_input::GamepadState` | 手柄状态 | 按钮/摇杆/扳机、DeviceId、震动等；具体字段由实现定义并文档化；每帧或按查询。 |
| `te_input::TouchPhase` | 触摸阶段 | 枚举：`Begin`, `Move`, `End`。 |
| `te_input::TouchState` | 触摸点状态 | 触摸点 ID（从 Begin 到 End 跨帧稳定）、位置、Phase；每帧或按查询。 |

*注：WindowHandle 来自 003-Application 契约，作为 Focus 参数使用。*

### 函数签名（C++17，完整模块）

**ActionId**（`include/te_input/ActionId.hpp`）

- `static ActionId ActionId::from_name(std::string_view name);`
- `bool operator==(const ActionId& a, const ActionId& b);`
- `bool operator!=(const ActionId& a, const ActionId& b);`

**AxisId**（`include/te_input/AxisId.hpp`）

- `static AxisId AxisId::from_name(std::string_view name);`
- `bool operator==(const AxisId& a, const AxisId& b);`
- `bool operator!=(const AxisId& a, const AxisId& b);`

**BindingTable**（`include/te_input/BindingTable.hpp`）

- `void BindingTable::add_binding(ActionId action, DeviceKind kind, KeyCode code);`
- `void BindingTable::add_axis_binding(AxisId axis, DeviceKind kind, KeyCode code);`
- `void BindingTable::remove_binding(ActionId action, DeviceKind kind, KeyCode code);`
- `void BindingTable::remove_axis_binding(AxisId axis, DeviceKind kind, KeyCode code);`
- `void BindingTable::remove_all_for_action(ActionId action);`
- `void BindingTable::remove_all_for_axis(AxisId axis);`
- `std::vector<BindingEntry> BindingTable::get_bindings_for_action(ActionId action) const;`
- `std::vector<BindingEntry> BindingTable::get_bindings_for_axis(AxisId axis) const;`

**MapDevice**（`include/te_input/MapDevice.hpp` 或等价）

- 物理设备 → 逻辑 DeviceId/DeviceKind 的映射；具体签名由实现定义并文档化（例如 `void map_physical_device(void* platform_handle_or_index, DeviceKind kind, DeviceId id);` 或等价）。

**LoadConfig**（`include/te_input/LoadConfig.hpp` 或等价）

- 从路径加载：`bool load_config(char const* path);` 或 `bool load_binding_config(char const* path);`
- 从内存加载：`bool load_config_from_memory(void const* data, size_t size);` 或等价  
- 配置格式由实现定义并文档化。

**动作/轴查询**（`include/te_input/Abstraction.hpp`）

- `bool get_action_state(ActionId action) const;` — 返回当前帧 ActionState（true=按下）。
- `float get_axis_value(AxisId axis) const;` — 返回当前帧 AxisValue。

**KeyboardMouse**（`include/te_input/KeyboardMouse.hpp` 或等价）

- `bool get_key(KeyCode code) const;`
- `MouseState get_mouse_position() const;` 或 `void get_mouse_position(float* x, float* y) const;`
- `MouseState get_mouse_delta() const;` 或 `void get_mouse_delta(float* dx, float* dy) const;`
- `void set_capture();` — 鼠标捕获到当前应用主窗口或 Application 提供的活动窗口。
- `void focus(WindowHandle window);` — 设置键盘/鼠标焦点到指定窗口；WindowHandle 来自 003-Application 契约。

**Gamepad**（`include/te_input/Gamepad.hpp` 或等价）

- `int get_gamepad_count() const;`
- `bool get_button(DeviceId device, KeyCode button) const;` 或等价按钮码类型。
- `float get_axis(DeviceId device, KeyCode axis) const;` 或等价轴码类型。
- `void set_vibration(DeviceId device, float left, float right);` — 可选；不支持时 no-op 或由实现文档化。

**Touch**（`include/te_input/Touch.hpp` 或等价）

- `int get_touch_count() const;`
- `TouchState get_touch(int touch_id) const;` 或 `bool get_touch(int touch_id, TouchState* out) const;`
- TouchPhase 为 TouchState 的一部分（Begin/Move/End）。

## 调用顺序与约束

- 须在 Core、Application 可用之后使用；Input 通过 Application 的 RegisterTickCallback 注册，在每帧 TickCallback 内从 Application 拉取本帧事件并更新状态；下游 GetKey/GetMouse/GetActionState 等返回本帧已更新状态。
- 多设备与多平台行为由实现定义并文档化；下游（UICore、Editor、XR）仅依赖本契约抽象。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 006-Input 模块规格与依赖表新增契约 |
| 2026-01-29 | API 雏形：由 plan 006-input-abstraction-001 同步（ActionId、AxisId、BindingTable、DeviceKind、KeyCode、BindingEntry） |
| 2026-01-29 | API 雏形：由 plan 006-input-fullversion-001 同步（完整模块：Abstraction、KeyboardMouse、Gamepad、Touch、MapDevice、LoadConfig） |
| 2026-01-29 | API 雏形：动作/轴查询 头文件明确为 include/te_input/Abstraction.hpp（与 plan/tasks 一致） |