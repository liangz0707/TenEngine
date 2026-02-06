# 契约：006-Input 模块对外 API

## 适用模块

- **实现方**：006-Input（L1；输入抽象、键鼠/手柄/触摸、原始输入、事件与映射）
- **对应规格**：`docs/module-specs/006-input.md`
- **依赖**：001-Core、003-Application

## 消费者

- 017-UICore、024-Editor、027-XR

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **KeyCode** | 键盘键码枚举（与物理设备解耦） | 编译时定义 |
| **MouseButton** | 鼠标按钮枚举（Left/Right/Middle等） | 编译时定义 |
| **GamepadButton** | 手柄按钮枚举（A/B/X/Y/Shoulder等） | 编译时定义 |
| **GamepadAxis** | 手柄轴枚举（LeftStickX/Y、RightStickX/Y、Trigger等） | 编译时定义 |
| **TouchPhase** | 触摸阶段枚举（Begin/Move/End/Cancel） | 编译时定义 |
| **TouchState** | 触摸点状态（ID、位置、阶段） | 每帧或按查询 |
| **ActionId** | 动作ID（uint32_t），用于输入抽象 | 注册后直至取消 |
| **AxisId** | 轴ID（uint32_t），用于输入抽象 | 注册后直至取消 |
| **ActionState / AxisValue** | 动作/轴抽象后的状态与数值 | 每帧或按查询 |
| **绑定表 / 配置** | 动作/轴与物理设备映射；可序列化或从配置加载 | 按实现约定 |

### 能力（提供方保证）

#### 1. 事件处理

| 能力 | 说明 |
|------|------|
| **ProcessEvents** | 从Application的EventQueue消费事件并更新输入状态；应在主循环中每帧调用 |

#### 2. 键盘输入

| 能力 | 说明 |
|------|------|
| **GetKey** | 获取键状态（当前帧按下） |
| **GetKeyDown** | 获取键按下状态（本帧刚按下，上一帧未按下） |
| **GetKeyUp** | 获取键释放状态（本帧刚释放，上一帧按下） |

#### 3. 鼠标输入

| 能力 | 说明 |
|------|------|
| **GetMouseButton** | 获取鼠标按钮状态（当前帧按下） |
| **GetMouseButtonDown** | 获取鼠标按钮按下状态（本帧刚按下） |
| **GetMouseButtonUp** | 获取鼠标按钮释放状态（本帧刚释放） |
| **GetMousePosition** | 获取鼠标位置（X、Y坐标） |
| **GetMouseDelta** | 获取鼠标增量（相对于上一帧的移动量） |
| **SetMouseCapture** | 设置鼠标捕获（捕获鼠标输入即使鼠标在窗口外） |
| **SetFocusWindow** | 设置焦点窗口（输入事件关联到此窗口） |

#### 4. 手柄输入

| 能力 | 说明 |
|------|------|
| **GetGamepadCount** | 获取连接的手柄数量 |
| **GetGamepadButton** | 获取手柄按钮状态 |
| **GetGamepadAxis** | 获取手柄轴值（范围[-1.0, 1.0]或[0.0, 1.0]用于扳机） |
| **SetGamepadVibration** | 设置手柄震动（可选功能） |

#### 5. 触摸输入

| 能力 | 说明 |
|------|------|
| **GetTouchCount** | 获取活动触摸点数量 |
| **GetTouch** | 获取触摸点状态（ID、位置、阶段）；与平台触摸事件对接 |

#### 6. 输入抽象（Action/Axis）

| 能力 | 说明 |
|------|------|
| **GetActionState** | 获取动作状态（映射自绑定的输入设备） |
| **GetAxisValue** | 获取轴值（映射自绑定的输入设备） |
| **RegisterAction** | 注册动作ID和名称 |
| **RegisterAxis** | 注册轴ID和名称 |
| **BindActionToKey** | 将动作绑定到键盘键 |
| **BindAxisToKey** | 将轴绑定到键盘键（带缩放因子） |
| **BindAxisToGamepadAxis** | 将轴绑定到手柄轴（带缩放因子） |
| **LoadBindingConfig** | 从配置文件加载绑定配置 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前版本：**2.0.0**（重新设计后的版本）

## 约束

1. **初始化顺序**：须在 Core、Application 可用之后使用；通过 Application 的 EventQueue 消费事件
2. **事件处理**：ProcessEvents() 应在主循环中每帧调用，从 Application 的 EventQueue 消费事件
3. **线程安全**：状态查询方法应该是线程安全的（使用 mutex 保护）
4. **多设备**：多设备与多平台行为由实现定义并文档化
5. **焦点管理**：输入与焦点协同：Editor 或 UI 获得焦点时，游戏逻辑可不接收输入（由实现约定）

## 命名空间与头文件

- **命名空间**：`te::input`
- **主要头文件**：
  - `te/input/Input.h`：IInput接口（整合键盘、鼠标、手柄、触摸、输入抽象）、CreateInput工厂函数
  - `te/input/InputTypes.h`：输入类型定义（KeyCode、MouseButton、GamepadButton、GamepadAxis、TouchPhase、TouchState、ActionId、AxisId）

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 006-Input 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
| 2026-02-06 | **重新设计**：接口整合（移除IInputService单例，改为IInput接口和CreateInput工厂函数）、事件处理改进（新增ProcessEvents方法从EventQueue消费事件）、状态查询增强（新增GetKeyDown/GetKeyUp、GetMouseButtonDown/GetMouseButtonUp方法）、命名规范统一（PascalCase命名，对齐Application和Core模块风格）、注释风格统一（per contract标注）；版本升级至2.0.0 |
