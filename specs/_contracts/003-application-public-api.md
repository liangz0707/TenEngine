# 契约：003-Application 模块对外 API

## 适用模块

- **实现方**：003-Application（L0；应用生命周期、窗口管理、事件处理、主循环）
- **对应规格**：`docs/module-specs/003-application.md`
- **依赖**：001-Core

## 消费者

- 006-Input、017-UICore、024-Editor、008-RHI

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **WindowHandle** | 窗口句柄（不透明类型）、与平台原生句柄对接 | 创建后直至销毁 |
| **WindowId** | 窗口ID（uint32_t或类似），用于多窗口管理 | 创建后直至销毁 |
| **DisplayInfo** | 显示器信息（分辨率、DPI、刷新率、位置） | 查询时有效 |
| **Event** | 来自平台消息泵的事件（窗口事件、输入事件、系统事件） | 单次泵取或队列内 |
| **DeltaTime** | 帧间隔时间（float，秒） | 每帧更新 |
| **TickCallback** | 主循环Tick回调函数类型 | 注册后直至取消注册 |
| **RunMode** | 运行模式枚举（Editor/Game/Headless） | 应用生命周期内 |
| **TimeStepMode** | 时间步模式枚举（Fixed/Variable/Mixed） | 可动态切换 |
| **WindowDesc** | 窗口创建描述结构（标题、尺寸、属性等），含IsValid()验证方法 | 创建时使用 |
| **EventType** | 事件类型枚举或ID | 事件生命周期内 |
| **IWindowPlatform** | 平台窗口抽象接口 | 平台实现生命周期内 |
| **IEventPumpPlatform** | 平台事件泵抽象接口 | 平台实现生命周期内 |
| **PlatformEvent** | 平台特定事件结构（不透明） | 平台事件处理时 |

### 能力（提供方保证）

#### 1. 应用生命周期

| 能力 | 说明 |
|------|------|
| **Initialize** | 初始化应用，解析参数，准备子系统；返回bool表示成功/失败 |
| **Run** | 进入主循环，直到退出；阻塞调用 |
| **Pause** | 暂停应用运行（停止Tick，但保持窗口和资源） |
| **Resume** | 恢复应用运行 |
| **RequestExit** | 请求退出，设置退出码；下一帧主循环退出 |
| **GetExitCode** | 获取退出码 |
| **GetRunMode** | 获取当前运行模式（Editor/Game/Headless） |
| **SetRunMode** | 切换运行模式（可选，某些模式切换可能需要重启） |

#### 2. 窗口管理

| 能力 | 说明 |
|------|------|
| **CreateWindow** | 创建窗口，返回WindowId；支持WindowDesc配置 |
| **DestroyWindow** | 销毁窗口 |
| **GetMainWindow** | 获取主窗口ID |
| **SetWindowTitle** | 设置窗口标题 |
| **SetWindowSize** | 设置窗口尺寸（宽、高） |
| **SetWindowPosition** | 设置窗口位置（x、y） |
| **SetFullscreen** | 切换全屏/窗口化 |
| **GetNativeHandle** | 获取平台原生句柄（HWND/X11 Window/NSWindow等） |
| **GetDisplayInfo** | 获取指定显示器的信息 |
| **EnumerateDisplays** | 枚举所有显示器 |
| **SetWindowCallback** | 设置窗口事件回调（创建/销毁/调整大小/焦点等） |
| **多窗口支持** | 可选功能，支持创建和管理多个窗口 |
| **WindowDesc::IsValid** | 验证窗口描述有效性（新增） |

#### 3. 事件系统

| 能力 | 说明 |
|------|------|
| **PumpEvents** | 轮询并处理平台消息；应在主循环中每帧调用 |
| **PushEvent** | 手动推送事件到队列（可选，用于自定义事件） |
| **GetEventQueue (const)** | 获取事件队列（const版本，用于只读访问） |
| **GetEventQueue (non-const)** | 获取事件队列（非const版本，供Input模块消费事件，因为Pop需要修改队列） |
| **事件类型** | 支持窗口事件、输入事件、系统事件等 |
| **简化设计** | 事件队列直接暴露，减少复杂的订阅机制；保留简单的窗口事件回调注册 |
| **EventQueue::Clear** | 清空事件队列（新增） |

#### 4. 主循环

| 能力 | 说明 |
|------|------|
| **GetDeltaTime** | 获取上一帧的DeltaTime（秒） |
| **GetTotalTime** | 获取总运行时间（秒） |
| **GetFrameCount** | 获取帧计数 |
| **SetTargetFPS** | 设置目标帧率（0表示不限制） |
| **SetTimeStepMode** | 设置时间步模式（Fixed/Variable/Mixed），Variable为默认 |
| **RegisterTickCallback** | 注册Tick回调（支持优先级，不强制阶段划分） |
| **UnregisterTickCallback** | 取消Tick回调注册 |
| **简化设计** | Tick回调支持优先级排序，更灵活，不强制Early/Update/Late阶段 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化（MAJOR.MINOR.PATCH）；破坏性变更递增 MAJOR。
- 当前版本：**2.0.0**（重新设计后的版本）

## 约束

1. **初始化顺序**：须在Core初始化之后使用；窗口创建与事件泵依赖平台抽象（Core）
2. **线程安全**：窗口操作必须在主线程执行（平台限制）；事件泵应在主循环中调用
3. **生命周期**：Run()为阻塞调用，直到RequestExit()被调用或窗口关闭
4. **事件处理**：PumpEvents()必须在主循环中每帧调用，否则窗口可能无响应
5. **多窗口**：多窗口支持为可选功能，单窗口场景必须支持
6. **时间步**：主循环必须支持至少一种时间步模式（Variable为默认）

## 命名空间与头文件

- **命名空间**：`te::application`
- **主要头文件**：
  - `te/application/Application.h`：IApplication接口（整合窗口、事件、主循环）、RunParams、RunMode、InitParams
  - `te/application/Window.h`：窗口相关类型、WindowDesc（含IsValid验证）、WindowId、DisplayInfo、WindowEvent、WindowCallback
  - `te/application/Event.h`：事件相关类型、Event、EventType、EventQueue（含Clear方法）
  - `te/application/MainLoop.h`：主循环相关类型、TimeStepMode、TickCallback、TickCallbackId
  - `te/application/Platform.h`：平台抽象接口、IWindowPlatform、IEventPumpPlatform、CreateWindowPlatform、CreateEventPumpPlatform

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 003-Application 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
| 2026-02-06 | **重新设计**：增强窗口管理（多窗口、窗口属性、窗口回调）、改进事件系统（事件队列、订阅机制、事件过滤）、完善主循环（多种时间步模式、帧率控制、Tick阶段）、明确生命周期管理（运行模式、暂停/恢复）；版本升级至2.0.0 |
| 2026-02-06 | **接口整合优化**：将窗口、事件、主循环功能整合到IApplication接口中，简化事件系统（直接暴露事件队列，移除复杂订阅），优化主循环（简化Tick阶段为优先级），与Core模块设计风格对齐；版本保持2.0.0 |
| 2026-02-06 | **平台抽象层重新设计**：新增IWindowPlatform和IEventPumpPlatform平台抽象接口，清晰的平台抽象层设计；优化WindowDesc（添加IsValid验证方法），改进EventQueue（添加Clear方法），统一注释风格对齐Core模块；版本保持2.0.0 |
| 2026-02-06 | **事件队列接口增强**：新增非const版本的GetEventQueue()方法，供Input模块消费事件（因为EventQueue::Pop需要修改队列状态）；版本保持2.0.0 |
