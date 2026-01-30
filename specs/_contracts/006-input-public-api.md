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

## API 雏形（本切片：ActionId / AxisId / BindingTable）

**来源**：plan 006-input-abstraction-001。技术栈 C++17、CMake；本切片仅暴露以下类型与 API。

### 类型（跨边界）

| 类型名 | 语义 | 说明 |
|--------|------|------|
| `te_input::ActionId` | 动作标识 | 值类型，`from_name(std::string_view)` 构造；支持 `operator==`/`operator!=`、可拷贝。无中央注册表。 |
| `te_input::AxisId` | 轴标识 | 同上，用于逻辑轴。 |
| `te_input::DeviceKind` | 设备种类 | 枚举：`Keyboard`, `Mouse`, `Gamepad`, `Touch`。 |
| `te_input::KeyCode` | 抽象键/轴码 | `std::uint32_t`，与设备解耦。 |
| `te_input::BindingEntry` | 单条绑定 | `struct { DeviceKind kind; KeyCode code; }`。 |
| `te_input::BindingTable` | 绑定表 | 动作/轴 到 (DeviceKind, KeyCode) 的映射集合；同一 ActionId/AxisId 允许多绑定。 |

### 函数签名（C++17，仅本切片）

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

## 调用顺序与约束

- 须在 Core、Application 可用之后使用；与 Application 事件泵的对接须明确（事件源由 Application 提供、Input 消费或转发）。
- 多设备与多平台行为由实现定义并文档化；下游（UICore、Editor、XR）仅依赖本契约抽象。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 006-Input 模块规格与依赖表新增契约 |
| 2026-01-29 | API 雏形：由 plan 006-input-abstraction-001 同步（ActionId、AxisId、BindingTable、DeviceKind、KeyCode、BindingEntry） |