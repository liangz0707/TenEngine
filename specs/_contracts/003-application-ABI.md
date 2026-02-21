# 003-Application 模块 ABI

- **契约**：[003-application-public-api.md](./003-application-public-api.md)（能力与类型描述）
- **本文件**：003-Application 对外 ABI 显式表。
- **CMake Target 名称**：**`te_application`**（project name `te_application`）。下游在 `target_link_libraries` 中应使用 **`te_application`**。
- **命名**：成员方法与自由函数采用**首字母大写的驼峰**（PascalCase）；所有方法在说明列给出**完整函数签名**。
- **命名空间**：te::application（头文件路径 te/application/）。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 应用生命周期接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 003-Application | te::application | IApplication | 抽象接口 | 应用生命周期与主循环 | te/application/Application.h | IApplication::Initialize | `bool Initialize(InitParams const* params = nullptr);` 初始化应用，解析参数，准备子系统；失败返回false |
| 003-Application | te::application | IApplication | 抽象接口 | 进入主循环 | te/application/Application.h | IApplication::Run | `void Run(RunParams const& args);` 进入主循环直至退出；阻塞调用 |
| 003-Application | te::application | IApplication | 抽象接口 | 暂停应用 | te/application/Application.h | IApplication::Pause | `void Pause();` 暂停应用运行（停止Tick，但保持窗口和资源） |
| 003-Application | te::application | IApplication | 抽象接口 | 恢复应用 | te/application/Application.h | IApplication::Resume | `void Resume();` 恢复应用运行 |
| 003-Application | te::application | IApplication | 抽象接口 | 请求退出 | te/application/Application.h | IApplication::RequestExit | `void RequestExit(int exitCode = 0);` 请求退出，设置退出码；下一帧主循环退出 |
| 003-Application | te::application | IApplication | 抽象接口 | 获取退出码 | te/application/Application.h | IApplication::GetExitCode | `int GetExitCode() const;` 获取退出码 |
| 003-Application | te::application | IApplication | 抽象接口 | 获取运行模式 | te/application/Application.h | IApplication::GetRunMode | `RunMode GetRunMode() const;` 获取当前运行模式（Editor/Game/Headless） |
| 003-Application | te::application | IApplication | 抽象接口 | 设置运行模式 | te/application/Application.h | IApplication::SetRunMode | `bool SetRunMode(RunMode mode);` 切换运行模式；某些模式切换可能需要重启，返回false表示不支持 |
| 003-Application | te::application | — | 自由函数 | 创建应用实例 | te/application/Application.h | CreateApplication | `IApplication* CreateApplication();` 失败返回 nullptr；调用方负责释放或由引擎管理 |
| 003-Application | te::application | — | struct | 初始化参数 | te/application/Application.h | InitParams | 可选参数：命令行参数、配置文件路径等；下游按需填充 |
| 003-Application | te::application | — | struct | 运行参数 | te/application/Application.h | RunParams | 窗口标题、宽高、每帧回调 TickCallback、runMode；下游填充 |
| 003-Application | te::application | — | 枚举 | 运行模式 | te/application/Application.h | RunMode | `enum class RunMode { Editor, Game, Headless };` 编辑器启动 vs 游戏启动 vs 无头模式 |

### 窗口管理接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 003-Application | te::application | IApplication | 抽象接口 | 创建窗口 | te/application/Window.h | IApplication::CreateWindow | `WindowId CreateWindow(WindowDesc const& desc);` 创建窗口，返回窗口ID；失败返回InvalidWindowId |
| 003-Application | te::application | IApplication | 抽象接口 | 销毁窗口 | te/application/Window.h | IApplication::DestroyWindow | `void DestroyWindow(WindowId windowId);` 销毁窗口 |
| 003-Application | te::application | IApplication | 抽象接口 | 获取主窗口 | te/application/Window.h | IApplication::GetMainWindow | `WindowId GetMainWindow() const;` 获取主窗口ID |
| 003-Application | te::application | IApplication | 抽象接口 | 设置窗口标题 | te/application/Window.h | IApplication::SetWindowTitle | `void SetWindowTitle(WindowId windowId, char const* title);` 设置窗口标题 |
| 003-Application | te::application | IApplication | 抽象接口 | 设置窗口尺寸 | te/application/Window.h | IApplication::SetWindowSize | `void SetWindowSize(WindowId windowId, uint32_t width, uint32_t height);` 设置窗口尺寸 |
| 003-Application | te::application | IApplication | 抽象接口 | 设置窗口位置 | te/application/Window.h | IApplication::SetWindowPosition | `void SetWindowPosition(WindowId windowId, int32_t x, int32_t y);` 设置窗口位置 |
| 003-Application | te::application | IApplication | 抽象接口 | 切换全屏 | te/application/Window.h | IApplication::SetFullscreen | `void SetFullscreen(WindowId windowId, bool fullscreen);` 切换全屏/窗口化 |
| 003-Application | te::application | IApplication | 抽象接口 | 获取原生句柄 | te/application/Window.h | IApplication::GetNativeHandle | `void* GetNativeHandle(WindowId windowId) const;` 获取平台原生句柄（HWND/X11 Window/NSWindow等） |
| 003-Application | te::application | IApplication | 抽象接口 | 获取显示器信息 | te/application/Window.h | IApplication::GetDisplayInfo | `DisplayInfo GetDisplayInfo(uint32_t displayIndex) const;` 获取指定显示器的信息 |
| 003-Application | te::application | IApplication | 抽象接口 | 枚举显示器 | te/application/Window.h | IApplication::EnumerateDisplays | `uint32_t EnumerateDisplays(DisplayInfo* displays, uint32_t maxCount) const;` 枚举所有显示器，返回实际数量 |
| 003-Application | te::application | IApplication | 抽象接口 | 设置窗口回调 | te/application/Window.h | IApplication::SetWindowCallback | `void SetWindowCallback(WindowId windowId, WindowCallback callback);` 设置窗口事件回调 |
| 003-Application | te::application | — | struct | 窗口描述 | te/application/Window.h | WindowDesc | 窗口创建参数：标题、尺寸、位置、属性（可调整大小、边框、置顶等）；含IsValid()验证方法 |
| 003-Application | te::application | — | typedef | 窗口ID | te/application/Window.h | WindowId | `typedef uint32_t WindowId;` 窗口ID类型；InvalidWindowId = 0 |
| 003-Application | te::application | — | struct | 显示器信息 | te/application/Window.h | DisplayInfo | 分辨率、DPI、刷新率、位置等；含默认值 |
| 003-Application | te::application | — | enum | 窗口事件类型 | te/application/Window.h | WindowEventType | `enum class WindowEventType { Created, Destroyed, Resized, Moved, Focused, Unfocused, Closed, Minimized, Maximized, Restored };` 窗口事件类型枚举 |
| 003-Application | te::application | — | struct | 窗口事件 | te/application/Window.h | WindowEvent | 窗口事件结构：类型、窗口ID、事件数据（联合体：resized、moved等） |
| 003-Application | te::application | — | 回调类型 | 窗口事件回调 | te/application/Window.h | WindowCallback | `void (*WindowCallback)(WindowId windowId, void const* event);` 窗口事件回调函数类型；event参数应转换为WindowEvent const* |

### 事件系统接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 003-Application | te::application | IApplication | 抽象接口 | 轮询事件 | te/application/Event.h | IApplication::PumpEvents | `void PumpEvents();` 轮询并处理平台消息；应在主循环中每帧调用 |
| 003-Application | te::application | IApplication | 抽象接口 | 推送事件 | te/application/Event.h | IApplication::PushEvent | `void PushEvent(Event const& event);` 手动推送事件到队列（可选） |
| 003-Application | te::application | IApplication | 抽象接口 | 获取事件队列（const） | te/application/Event.h | IApplication::GetEventQueue | `EventQueue const& GetEventQueue() const;` 获取事件队列（const版本，用于只读访问） |
| 003-Application | te::application | IApplication | 抽象接口 | 获取事件队列（非const） | te/application/Event.h | IApplication::GetEventQueue | `EventQueue& GetEventQueue();` 获取事件队列（非const版本，供Input模块消费事件，因为Pop需要修改队列） |
| 003-Application | te::application | — | enum | 事件类型 | te/application/Event.h | EventType | `enum class EventType { WindowCreated, WindowDestroyed, WindowResized, WindowMoved, WindowFocused, WindowClosed, KeyDown, KeyUp, MouseMove, MouseButtonDown, MouseButtonUp, ... };` 事件类型枚举 |
| 003-Application | te::application | — | struct | 事件 | te/application/Event.h | Event | 事件结构：类型、时间戳、窗口ID、事件数据（联合体） |
| 003-Application | te::application | EventQueue | 类 | 事件队列 | te/application/Event.h | EventQueue::Pop, Push, Empty, Size, Clear | `bool Pop(Event& event);` `void Push(Event const& event);` `bool Empty() const;` `std::size_t Size() const;` `void Clear();` 线程安全的事件队列，供Input模块消费 |

### 主循环接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 003-Application | te::application | IApplication | 抽象接口 | 获取DeltaTime | te/application/MainLoop.h | IApplication::GetDeltaTime | `float GetDeltaTime() const;` 获取上一帧的DeltaTime（秒） |
| 003-Application | te::application | IApplication | 抽象接口 | 获取总时间 | te/application/MainLoop.h | IApplication::GetTotalTime | `float GetTotalTime() const;` 获取总运行时间（秒） |
| 003-Application | te::application | IApplication | 抽象接口 | 获取帧计数 | te/application/MainLoop.h | IApplication::GetFrameCount | `uint64_t GetFrameCount() const;` 获取帧计数 |
| 003-Application | te::application | IApplication | 抽象接口 | 设置目标FPS | te/application/MainLoop.h | IApplication::SetTargetFPS | `void SetTargetFPS(uint32_t fps);` 设置目标帧率（0表示不限制） |
| 003-Application | te::application | IApplication | 抽象接口 | 设置时间步模式 | te/application/MainLoop.h | IApplication::SetTimeStepMode | `void SetTimeStepMode(TimeStepMode mode);` 设置时间步模式（Fixed/Variable/Mixed） |
| 003-Application | te::application | IApplication | 抽象接口 | 注册Tick回调 | te/application/MainLoop.h | IApplication::RegisterTickCallback | `TickCallbackId RegisterTickCallback(TickCallback callback, int32_t priority = 0);` 注册Tick回调，支持优先级排序，返回回调ID |
| 003-Application | te::application | IApplication | 抽象接口 | 取消Tick回调 | te/application/MainLoop.h | IApplication::UnregisterTickCallback | `void UnregisterTickCallback(TickCallbackId callbackId);` 取消Tick回调注册 |
| 003-Application | te::application | — | 回调类型 | 每帧回调 | te/application/MainLoop.h | TickCallback | `void (*TickCallback)(float deltaTime);` 主循环每帧调用一次 |
| 003-Application | te::application | — | enum | 时间步模式 | te/application/MainLoop.h | TimeStepMode | `enum class TimeStepMode { Fixed, Variable, Mixed };` 固定时间步、可变时间步、混合模式；Variable为默认 |
| 003-Application | te::application | — | typedef | Tick回调ID | te/application/MainLoop.h | TickCallbackId | `typedef uint64_t TickCallbackId;` Tick回调ID类型 |

### 平台抽象接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 003-Application | te::application | IWindowPlatform | 抽象接口 | 平台窗口抽象 | te/application/Platform.h | IWindowPlatform::CreateNativeWindow | `void* CreateNativeWindow(WindowDesc const& desc);` 创建原生平台窗口，返回原生句柄或nullptr |
| 003-Application | te::application | IWindowPlatform | 抽象接口 | 销毁原生窗口 | te/application/Platform.h | IWindowPlatform::DestroyNativeWindow | `void DestroyNativeWindow(void* handle);` 销毁原生平台窗口 |
| 003-Application | te::application | IWindowPlatform | 抽象接口 | 设置窗口标题 | te/application/Platform.h | IWindowPlatform::SetWindowTitle | `void SetWindowTitle(void* handle, char const* title);` 设置原生窗口标题 |
| 003-Application | te::application | IWindowPlatform | 抽象接口 | 设置窗口尺寸 | te/application/Platform.h | IWindowPlatform::SetWindowSize | `void SetWindowSize(void* handle, uint32_t width, uint32_t height);` 设置原生窗口尺寸 |
| 003-Application | te::application | IWindowPlatform | 抽象接口 | 设置窗口位置 | te/application/Platform.h | IWindowPlatform::SetWindowPosition | `void SetWindowPosition(void* handle, int32_t x, int32_t y);` 设置原生窗口位置 |
| 003-Application | te::application | IWindowPlatform | 抽象接口 | 设置全屏 | te/application/Platform.h | IWindowPlatform::SetFullscreen | `void SetFullscreen(void* handle, bool fullscreen);` 设置原生窗口全屏模式 |
| 003-Application | te::application | IWindowPlatform | 抽象接口 | 获取显示器信息 | te/application/Platform.h | IWindowPlatform::GetDisplayInfo | `DisplayInfo GetDisplayInfo(uint32_t displayIndex) const;` 获取平台显示器信息 |
| 003-Application | te::application | IWindowPlatform | 抽象接口 | 枚举显示器 | te/application/Platform.h | IWindowPlatform::EnumerateDisplays | `uint32_t EnumerateDisplays(DisplayInfo* displays, uint32_t maxCount) const;` 枚举平台显示器 |
| 003-Application | te::application | IEventPumpPlatform | 抽象接口 | 轮询平台事件 | te/application/Platform.h | IEventPumpPlatform::PollEvent | `bool PollEvent(PlatformEvent& event);` 轮询平台事件（非阻塞），返回true表示有事件 |
| 003-Application | te::application | IEventPumpPlatform | 抽象接口 | 转换为引擎事件 | te/application/Platform.h | IEventPumpPlatform::ConvertToEngineEvent | `Event ConvertToEngineEvent(PlatformEvent const& platformEvent, WindowId windowId);` 将平台事件转换为引擎Event |
| 003-Application | te::application | IEventPumpPlatform | 抽象接口 | 检查退出事件 | te/application/Platform.h | IEventPumpPlatform::IsQuitEvent | `bool IsQuitEvent(PlatformEvent const& platformEvent) const;` 检查平台事件是否为退出事件 |
| 003-Application | te::application | — | 自由函数 | 创建窗口平台 | te/application/Platform.h | CreateWindowPlatform | `IWindowPlatform* CreateWindowPlatform();` 创建平台特定的窗口实现，失败返回nullptr |
| 003-Application | te::application | — | 自由函数 | 创建事件泵平台 | te/application/Platform.h | CreateEventPumpPlatform | `IEventPumpPlatform* CreateEventPumpPlatform();` 创建平台特定的事件泵实现，失败返回nullptr |
| 003-Application | te::application | — | struct | 平台事件 | te/application/Platform.h | PlatformEvent | 平台特定事件结构（不透明，平台相关） |

*来源：用户故事 US-lifecycle-001（应用启动并进入主循环）、US-lifecycle-002（编辑器/游戏模式启动）。*

*版本：2.0.0（2026-02-06重新设计，2026-02-06接口整合优化，2026-02-06平台抽象层重新设计）*

*变更说明：*
- *接口整合：窗口、事件、主循环功能整合到IApplication接口中*
- *事件系统简化：移除复杂的事件订阅机制（SubscribeEvent/UnsubscribeEvent/SetEventFilter），直接暴露事件队列供Input模块消费*
- *主循环简化：移除Tick阶段（TickPhase），Tick回调支持优先级但不强制阶段划分*
- *命名规范：与Core模块对齐，使用PascalCase命名*
- *平台抽象层：新增IWindowPlatform和IEventPumpPlatform接口，清晰的平台抽象层设计，改进跨平台兼容性*
- *类型优化：WindowDesc添加IsValid()验证方法，DisplayInfo添加默认值，EventQueue添加Clear()方法*
- *注释风格：统一注释风格对齐Core模块（使用per contract标注）*

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| 2026-02-06 | 重新设计版本 2.0.0；接口整合、事件系统简化、主循环简化、平台抽象层 |
| 2026-02-22 | Verified alignment with code: IApplication::SetWndProcHandler added; InitParams has argc/argv/configPath; RunParams has full fields; WindowDesc has displayIndex; WindowEventType has Minimized/Maximized/Restored; Event includes touch events; IWindowPlatform::SetWndProcHandler has default implementation |
