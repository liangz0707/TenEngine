# US-input-001：输入轮询与事件（键盘、鼠标、触摸、游戏手柄）

- **标题**：应用在主循环中**轮询输入**或接收**输入事件**；支持键盘、鼠标、触摸、游戏手柄；与 Application 消息泵或独立轮询对接，供游戏逻辑、UI、Editor 消费。
- **编号**：US-input-001

---

## 1. 角色/触发

- **角色**：游戏逻辑、UI、Editor
- **触发**：每帧或事件驱动下需要获取**键盘**、**鼠标**、**触摸**、**游戏手柄**等输入状态或事件；不阻塞主循环；与平台（Win/Linux/macOS/Android/iOS）解耦。

---

## 2. 端到端流程

1. **Application** 在主循环中调用 **pollEvents()** 或由窗口系统推送事件；**Input** 模块接收原始输入并转换为引擎抽象（KeyCode、MouseButton、TouchPhase、GamepadAxis 等）。
2. 游戏逻辑/UI/Editor 通过 **Input** 接口查询当前帧按键状态（getKey、getMouseButton、getTouchCount）或订阅事件（onKeyDown、onMouseMove、onTouch 等）。
3. 输入与**焦点**协同：Editor 或 UI 获得焦点时，游戏逻辑可不接收输入（由实现约定）。
4. 多设备与多平台行为由实现定义并文档化；下游（UICore、Editor、XR）仅依赖本契约抽象。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 006-Input | 输入设备抽象、轮询/事件 API、KeyCode/MouseButton/Touch/Gamepad 枚举、getKey/getMouseButton/getTouch、onKeyDown/onMouseMove 等 |
| 003-Application | 窗口与消息泵、pollEvents、与平台事件源对接 |
| 017-UICore / 024-Editor | 消费输入、焦点管理 |

---

## 4. 每模块职责与 I/O

### 006-Input

- **职责**：提供 **pollEvents** 或 **processEvent**；**getKey**、**getMouseButton**、**getMousePosition**、**getTouchCount**、**getTouch**、**getGamepadAxis** 等查询接口；可选 **onKeyDown**、**onMouseMove**、**onTouch** 等事件回调；KeyCode、MouseButton、TouchPhase、GamepadAxis 等枚举；与 Application 窗口/消息泵对接。
- **输入**：平台原始事件或轮询结果。
- **输出**：抽象按键/鼠标/触摸/手柄状态与事件；下游据此驱动逻辑/UI/Editor。

---

## 5. 派生 ABI（与契约对齐）

- **006-input-ABI**：pollEvents、getKey、getMouseButton、getMousePosition、getTouchCount、getTouch、getGamepadAxis；KeyCode、MouseButton、TouchPhase 等枚举；可选 onKeyDown、onMouseMove、onTouch 回调。详见 `specs/_contracts/006-input-ABI.md`。

---

## 6. 验收要点

- 主循环中可轮询或接收键盘、鼠标、触摸、游戏手柄输入；API 与平台解耦。
- 游戏逻辑、UI、Editor 可通过统一 Input 接口获取当前帧状态或事件。
