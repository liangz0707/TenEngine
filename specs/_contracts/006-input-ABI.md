# 006-Input 模块 ABI

- **契约**：[006-input-public-api.md](./006-input-public-api.md)（能力与类型描述）
- **本文件**：006-Input 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 006-Input | TenEngine::input | IInputService | 抽象接口/单例 | 输入服务入口 | TenEngine/input/InputService.h | GetInputService | `IInputService* GetInputService();` 由 Application 或 SubsystemRegistry 提供；调用方不拥有指针 |
| 006-Input | TenEngine::input | IInputService | 抽象接口 | 键状态 | TenEngine/input/InputService.h | IInputService::GetKey | `bool GetKey(KeyCode code) const;` 本帧按下为 true |
| 006-Input | TenEngine::input | IInputService | 抽象接口 | 鼠标位置与增量 | TenEngine/input/InputService.h | IInputService::GetMousePosition, GetMouseDelta | `void GetMousePosition(int* x, int* y) const;` `void GetMouseDelta(int* dx, int* dy) const;` 返回本帧位置/增量 |
| 006-Input | TenEngine::input | IInputService | 抽象接口 | 鼠标捕获与焦点 | TenEngine/input/InputService.h | IInputService::SetCapture, SetFocusWindow | `void SetCapture(bool capture);` `void SetFocusWindow(void* window_handle);` 与 Application 窗口对接 |
| 006-Input | TenEngine::input | IInputService | 抽象接口 | 手柄数量与状态 | TenEngine/input/InputService.h | IInputService::GetGamepadCount, GetButton, GetAxis | `uint32_t GetGamepadCount() const;` `bool GetButton(uint32_t device_id, GamepadButton button) const;` `float GetAxis(uint32_t device_id, GamepadAxis axis) const;` SetVibration 可选 |
| 006-Input | TenEngine::input | IInputService | 抽象接口 | 触摸 | TenEngine/input/InputService.h | IInputService::GetTouchCount, GetTouch | `uint32_t GetTouchCount() const;` `void GetTouch(uint32_t index, TouchState* out) const;` Phase(Begin/Move/End)；与平台触摸对接 |
| 006-Input | TenEngine::input | — | 枚举/类型 | 键码、鼠标按钮 | TenEngine/input/InputTypes.h | KeyCode, MouseButton | 与物理设备解耦的键码与按钮枚举 |
| 006-Input | TenEngine::input | — | 抽象/配置 | 动作/轴与绑定 | TenEngine/input/InputBinding.h | IInputBinding::GetActionState, GetAxisValue | `bool GetActionState(ActionId id) const;` `float GetAxisValue(AxisId id) const;` ActionId/AxisId、BindingTable、MapDevice、LoadConfig |
