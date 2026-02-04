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

## 调用顺序与约束

- 须在 Core、Application 可用之后使用；Input 通过 Application 的 RegisterTickCallback 注册，在每帧 TickCallback 内从 Application 拉取本帧事件并更新状态；下游 GetKey/GetMouse/GetActionState 等返回本帧已更新状态。
- 多设备与多平台行为由实现定义并文档化；下游（UICore、Editor、XR）仅依赖本契约抽象。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 006-Input 模块规格与依赖表新增契约 |
| 2026-01-29 | 契约更新由 plan 006-input-fullversion-001 同步 |