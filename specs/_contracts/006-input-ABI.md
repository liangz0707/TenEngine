# 006-Input 模块 ABI

- **契约**：[006-input-public-api.md](./006-input-public-api.md)（能力与类型描述）
- **本文件**：006-Input 对外 ABI 显式表。
- **CMake Target 名称**：**`te_input`**（project name `te_input`）。下游在 `target_link_libraries` 中应使用 **`te_input`**。
- **命名**：成员方法与自由函数采用**首字母大写的驼峰**（PascalCase）；所有方法在说明列给出**完整函数签名**。
- **命名空间**：te::input（头文件路径 te/input/）。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 事件处理接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 006-Input | te::input | IInput | 抽象接口 | 处理事件 | te/input/Input.h | IInput::ProcessEvents | `void ProcessEvents(te::application::EventQueue& eventQueue);` 从Application的EventQueue消费事件并更新输入状态（非const引用因为Pop需要修改队列） |

### 键盘输入接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 006-Input | te::input | IInput | 抽象接口 | 键状态 | te/input/Input.h | IInput::GetKey | `bool GetKey(KeyCode code) const;` 当前帧按下为 true |
| 006-Input | te::input | IInput | 抽象接口 | 键按下 | te/input/Input.h | IInput::GetKeyDown | `bool GetKeyDown(KeyCode code) const;` 本帧刚按下（上一帧未按下） |
| 006-Input | te::input | IInput | 抽象接口 | 键释放 | te/input/Input.h | IInput::GetKeyUp | `bool GetKeyUp(KeyCode code) const;` 本帧刚释放（上一帧按下） |

### 鼠标输入接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 006-Input | te::input | IInput | 抽象接口 | 鼠标按钮状态 | te/input/Input.h | IInput::GetMouseButton | `bool GetMouseButton(MouseButton button) const;` 当前帧按下为 true |
| 006-Input | te::input | IInput | 抽象接口 | 鼠标按钮按下 | te/input/Input.h | IInput::GetMouseButtonDown | `bool GetMouseButtonDown(MouseButton button) const;` 本帧刚按下 |
| 006-Input | te::input | IInput | 抽象接口 | 鼠标按钮释放 | te/input/Input.h | IInput::GetMouseButtonUp | `bool GetMouseButtonUp(MouseButton button) const;` 本帧刚释放 |
| 006-Input | te::input | IInput | 抽象接口 | 鼠标位置 | te/input/Input.h | IInput::GetMousePosition | `void GetMousePosition(int32_t* x, int32_t* y) const;` 返回本帧鼠标位置 |
| 006-Input | te::input | IInput | 抽象接口 | 鼠标增量 | te/input/Input.h | IInput::GetMouseDelta | `void GetMouseDelta(int32_t* dx, int32_t* dy) const;` 返回本帧鼠标移动增量 |
| 006-Input | te::input | IInput | 抽象接口 | 鼠标捕获 | te/input/Input.h | IInput::SetMouseCapture | `void SetMouseCapture(bool capture);` 设置鼠标捕获（与Application窗口对接） |
| 006-Input | te::input | IInput | 抽象接口 | 焦点窗口 | te/input/Input.h | IInput::SetFocusWindow | `void SetFocusWindow(te::application::WindowId windowId);` 设置焦点窗口（输入事件关联到此窗口） |

### 手柄输入接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 006-Input | te::input | IInput | 抽象接口 | 手柄数量 | te/input/Input.h | IInput::GetGamepadCount | `uint32_t GetGamepadCount() const;` 返回连接的手柄数量 |
| 006-Input | te::input | IInput | 抽象接口 | 手柄按钮 | te/input/Input.h | IInput::GetGamepadButton | `bool GetGamepadButton(uint32_t deviceId, GamepadButton button) const;` 返回手柄按钮状态 |
| 006-Input | te::input | IInput | 抽象接口 | 手柄轴 | te/input/Input.h | IInput::GetGamepadAxis | `float GetGamepadAxis(uint32_t deviceId, GamepadAxis axis) const;` 返回手柄轴值 [-1.0, 1.0]（或[0.0, 1.0]用于扳机） |
| 006-Input | te::input | IInput | 抽象接口 | 手柄震动 | te/input/Input.h | IInput::SetGamepadVibration | `void SetGamepadVibration(uint32_t deviceId, float leftMotor, float rightMotor);` 设置手柄震动（可选功能） |

### 触摸输入接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 006-Input | te::input | IInput | 抽象接口 | 触摸数量 | te/input/Input.h | IInput::GetTouchCount | `uint32_t GetTouchCount() const;` 返回活动触摸点数量 |
| 006-Input | te::input | IInput | 抽象接口 | 触摸状态 | te/input/Input.h | IInput::GetTouch | `void GetTouch(uint32_t index, TouchState* out) const;` 获取触摸点状态（ID、位置、阶段） |

### 输入抽象接口（Action/Axis）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 006-Input | te::input | IInput | 抽象接口 | 动作状态 | te/input/Input.h | IInput::GetActionState | `bool GetActionState(ActionId actionId) const;` 返回动作状态（映射自绑定的输入设备） |
| 006-Input | te::input | IInput | 抽象接口 | 轴值 | te/input/Input.h | IInput::GetAxisValue | `float GetAxisValue(AxisId axisId) const;` 返回轴值（映射自绑定的输入设备） |
| 006-Input | te::input | IInput | 抽象接口 | 注册动作 | te/input/Input.h | IInput::RegisterAction | `void RegisterAction(ActionId actionId, char const* name);` 注册动作ID和名称 |
| 006-Input | te::input | IInput | 抽象接口 | 注册轴 | te/input/Input.h | IInput::RegisterAxis | `void RegisterAxis(AxisId axisId, char const* name);` 注册轴ID和名称 |
| 006-Input | te::input | IInput | 抽象接口 | 绑定动作到键 | te/input/Input.h | IInput::BindActionToKey | `void BindActionToKey(ActionId actionId, KeyCode key);` 将动作绑定到键盘键 |
| 006-Input | te::input | IInput | 抽象接口 | 绑定轴到键 | te/input/Input.h | IInput::BindAxisToKey | `void BindAxisToKey(AxisId axisId, KeyCode key, float scale);` 将轴绑定到键盘键（带缩放因子） |
| 006-Input | te::input | IInput | 抽象接口 | 绑定轴到手柄轴 | te/input/Input.h | IInput::BindAxisToGamepadAxis | `void BindAxisToGamepadAxis(AxisId axisId, uint32_t deviceId, GamepadAxis axis, float scale);` 将轴绑定到手柄轴（带缩放因子） |
| 006-Input | te::input | IInput | 抽象接口 | 加载绑定配置 | te/input/Input.h | IInput::LoadBindingConfig | `bool LoadBindingConfig(char const* configPath);` 从配置文件加载绑定配置 |

### 工厂函数

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 006-Input | te::input | — | 自由函数 | 创建输入实例 | te/input/Input.h | CreateInput | `IInput* CreateInput();` 创建输入实例，失败返回 nullptr；调用方负责释放或由引擎管理 |
| 006-Input | te::input | — | 自由函数 | 创建输入实例（带Application） | te/input/Input.h | CreateInput | `IInput* CreateInput(te::application::IApplication* application);` 创建输入实例并关联Application（可选自动事件处理），失败返回 nullptr |

### 类型定义

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 006-Input | te::input | — | 枚举 | 键码 | te/input/InputTypes.h | KeyCode | `enum class KeyCode : uint32_t { A, B, C, ..., Space, Enter, Escape, ... };` 与物理设备解耦的键码枚举 |
| 006-Input | te::input | — | 枚举 | 鼠标按钮 | te/input/InputTypes.h | MouseButton | `enum class MouseButton : uint8_t { Left, Right, Middle, Button4, Button5 };` 鼠标按钮枚举 |
| 006-Input | te::input | — | 枚举 | 手柄按钮 | te/input/InputTypes.h | GamepadButton | `enum class GamepadButton : uint8_t { A, B, X, Y, LeftShoulder, RightShoulder, Back, Start, ... };` 手柄按钮枚举 |
| 006-Input | te::input | — | 枚举 | 手柄轴 | te/input/InputTypes.h | GamepadAxis | `enum class GamepadAxis : uint8_t { LeftStickX, LeftStickY, RightStickX, RightStickY, LeftTrigger, RightTrigger };` 手柄轴枚举 |
| 006-Input | te::input | — | 枚举 | 触摸阶段 | te/input/InputTypes.h | TouchPhase | `enum class TouchPhase : uint8_t { Begin, Move, End, Cancel };` 触摸阶段枚举 |
| 006-Input | te::input | — | struct | 触摸状态 | te/input/InputTypes.h | TouchState | `struct TouchState { uint32_t touchId; float x; float y; TouchPhase phase; };` 触摸点状态结构 |
| 006-Input | te::input | — | typedef | 动作ID | te/input/InputTypes.h | ActionId | `using ActionId = uint32_t;` 动作ID类型 |
| 006-Input | te::input | — | typedef | 轴ID | te/input/InputTypes.h | AxisId | `using AxisId = uint32_t;` 轴ID类型 |

*来源：用户故事 US-input-001（输入轮询与事件）。*

*版本：2.0.0（2026-02-06重新设计，对齐Application模块设计风格）*

*变更说明：*
- *接口整合：移除IInputService单例模式，改为IInput接口和CreateInput工厂函数；整合所有输入功能到单一接口*
- *事件处理：新增ProcessEvents方法，直接从Application的EventQueue消费事件*
- *状态查询：新增GetKeyDown/GetKeyUp、GetMouseButtonDown/GetMouseButtonUp方法（基于上一帧状态）*
- *命名规范：统一使用PascalCase命名，对齐Application和Core模块风格*
- *注释风格：统一注释风格对齐Core模块（使用per contract标注）*
- *输入抽象：简化Action/Axis绑定实现，保留核心功能但减少复杂性*
