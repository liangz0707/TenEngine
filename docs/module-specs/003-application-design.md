# 003-Application 模块详细设计文档

本文档描述 Application 模块的**详细接口设计、类结构、实现架构**，作为实现参考。

## 1. 接口设计

### 1.1 核心接口：IApplication

```cpp
namespace te::application {

// 运行模式
enum class RunMode {
    Editor,    // 编辑器模式
    Game,      // 游戏模式
    Headless   // 无头模式（无窗口）
};

// 初始化参数
struct InitParams {
    int argc = 0;
    char const** argv = nullptr;
    char const* configPath = nullptr;
    // 其他可选参数...
};

// 运行参数
struct RunParams {
    char const* windowTitle = "TenEngine";
    uint32_t windowWidth = 1280;
    uint32_t windowHeight = 720;
    RunMode runMode = RunMode::Game;
    TickCallback tickCallback = nullptr;
    // 其他可选参数...
};

// 应用主接口（整合窗口、事件、主循环功能）
class IApplication {
public:
    virtual ~IApplication() = default;
    
    // ========== 生命周期 ==========
    virtual bool Initialize(InitParams const* params = nullptr) = 0;
    virtual void Run(RunParams const& args) = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual void RequestExit(int exitCode = 0) = 0;
    virtual int GetExitCode() const = 0;
    virtual RunMode GetRunMode() const = 0;
    virtual bool SetRunMode(RunMode mode) = 0;
    
    // ========== 窗口管理 ==========
    virtual WindowId CreateWindow(WindowDesc const& desc) = 0;
    virtual void DestroyWindow(WindowId windowId) = 0;
    virtual WindowId GetMainWindow() const = 0;
    virtual void SetWindowTitle(WindowId windowId, char const* title) = 0;
    virtual void SetWindowSize(WindowId windowId, uint32_t width, uint32_t height) = 0;
    virtual void SetWindowPosition(WindowId windowId, int32_t x, int32_t y) = 0;
    virtual void SetFullscreen(WindowId windowId, bool fullscreen) = 0;
    virtual void* GetNativeHandle(WindowId windowId) const = 0;
    virtual DisplayInfo GetDisplayInfo(uint32_t displayIndex) const = 0;
    virtual uint32_t EnumerateDisplays(DisplayInfo* displays, uint32_t maxCount) const = 0;
    virtual void SetWindowCallback(WindowId windowId, WindowCallback callback) = 0;
    
    // ========== 事件系统 ==========
    virtual void PumpEvents() = 0;
    virtual void PushEvent(Event const& event) = 0;  // 可选
    virtual EventQueue const& GetEventQueue() const = 0;
    
    // ========== 主循环 ==========
    virtual float GetDeltaTime() const = 0;
    virtual float GetTotalTime() const = 0;
    virtual uint64_t GetFrameCount() const = 0;
    virtual void SetTargetFPS(uint32_t fps) = 0;
    virtual void SetTimeStepMode(TimeStepMode mode) = 0;
    virtual TickCallbackId RegisterTickCallback(TickCallback callback, int32_t priority = 0) = 0;
    virtual void UnregisterTickCallback(TickCallbackId callbackId) = 0;
};

// 工厂函数
IApplication* CreateApplication();

} // namespace te::application
```

### 1.2 窗口相关类型（整合到IApplication）

```cpp
namespace te::application {

// 窗口ID类型
using WindowId = uint32_t;
constexpr WindowId InvalidWindowId = 0;

// 窗口描述
struct WindowDesc {
    char const* title = "Window";
    uint32_t width = 800;
    uint32_t height = 600;
    int32_t x = -1;  // -1 表示居中
    int32_t y = -1;  // -1 表示居中
    bool resizable = true;
    bool borderless = false;
    bool alwaysOnTop = false;
    bool fullscreen = false;
    uint32_t displayIndex = 0;
};

// 窗口事件类型
enum class WindowEventType {
    Created,
    Destroyed,
    Resized,
    Moved,
    Focused,
    Unfocused,
    Closed,
    Minimized,
    Maximized,
    Restored
};

// 窗口事件
struct WindowEvent {
    WindowEventType type;
    WindowId windowId;
    union {
        struct { uint32_t width, height; } resized;
        struct { int32_t x, y; } moved;
        // 其他事件数据...
    };
};

// 窗口事件回调
using WindowCallback = void (*)(WindowId windowId, WindowEvent const& event);

// 显示器信息
struct DisplayInfo {
    uint32_t index;
    int32_t x, y;
    uint32_t width, height;
    float dpiX, dpiY;
    float refreshRate;
    bool primary;
};

} // namespace te::application
```

**注意**：窗口管理功能已整合到IApplication接口中，不再需要独立的IWindowManager接口。

**改进**：
- WindowDesc添加IsValid()验证方法
- DisplayInfo添加默认值，更安全
- WindowCallback签名优化（使用void const*，更灵活）

### 1.3 事件相关类型（整合到IApplication）

```cpp
namespace te::application {

// 事件类型
enum class EventType {
    // 窗口事件
    WindowCreated,
    WindowDestroyed,
    WindowResized,
    WindowMoved,
    WindowFocused,
    WindowClosed,
    
    // 输入事件（与Input模块解耦）
    KeyDown,
    KeyUp,
    MouseMove,
    MouseButtonDown,
    MouseButtonUp,
    MouseWheel,
    TouchDown,
    TouchUp,
    TouchMove,
    
    // 系统事件
    AppPaused,
    AppResumed,
    AppWillTerminate
};

// 事件
struct Event {
    EventType type;
    float timestamp;
    WindowId windowId;
    union {
        struct { uint32_t keyCode; } key;
        struct { int32_t x, y; } mouse;
        struct { uint32_t touchId; float x, y; } touch;
        // 其他事件数据...
    };
};

// 事件队列（供Input模块直接消费）
class EventQueue {
public:
    bool Pop(Event& event);
    void Push(Event const& event);
    bool Empty() const;
    size_t Size() const;
};

} // namespace te::application
```

**注意**：事件系统功能已整合到IApplication接口中。简化设计：
- 移除复杂的事件订阅机制（SubscribeEvent/UnsubscribeEvent）
- 移除事件过滤器（SetEventFilter）
- 直接暴露事件队列供Input模块消费，更简单高效

**改进**：
- EventQueue添加Clear()方法，便于清空队列
- 改进线程安全设计说明

### 1.4 主循环相关类型（整合到IApplication）

```cpp
namespace te::application {

// 时间步模式
enum class TimeStepMode {
    Fixed,     // 固定时间步（适合物理模拟）
    Variable,  // 可变时间步（适合游戏逻辑，默认）
    Mixed      // 混合模式（固定更新，可变渲染）
};

// Tick回调ID
using TickCallbackId = uint64_t;

// Tick回调
using TickCallback = void (*)(float deltaTime);

} // namespace te::application
```

**注意**：主循环功能已整合到IApplication接口中。简化设计：
- 移除Tick阶段（TickPhase），不再强制Early/Update/Late阶段划分
- Tick回调支持优先级排序，更灵活
- RegisterTickCallback(callback, priority) 简化接口

**改进**：
- TimeStepMode默认值为Variable，更符合游戏开发习惯
- 注释风格统一对齐Core模块

## 2. 类结构设计

### 2.1 接口设计

```
IApplication (主接口，整合所有功能)
├── 生命周期方法
├── 窗口管理方法（整合）
├── 事件系统方法（整合，简化）
└── 主循环方法（整合，简化）

平台抽象层（PAL）
├── IWindowPlatform (平台窗口抽象)
│   ├── WindowsWindowPlatform (Windows实现)
│   ├── X11WindowPlatform (Linux实现)
│   ├── CocoaWindowPlatform (macOS实现)
│   └── AndroidWindowPlatform / IOSWindowPlatform
└── IEventPumpPlatform (平台事件泵抽象)
    ├── WindowsEventPumpPlatform (Windows实现)
    ├── X11EventPumpPlatform (Linux实现)
    ├── CocoaEventPumpPlatform (macOS实现)
    └── AndroidEventPumpPlatform / IOSEventPumpPlatform
```

**设计原则**：
- 参考Core模块的Allocator设计模式，单一接口但功能完整，使用更简单
- 平台抽象层清晰分离，易于测试和扩展
- 使用工厂函数创建平台特定实现（CreateWindowPlatform、CreateEventPumpPlatform）

### 2.2 实现类结构

```
Application (IApplication实现)
├── WindowManager (窗口管理实现)
│   ├── Window (窗口封装)
│   │   ├── WindowsWindow (Windows平台实现)
│   │   ├── X11Window (Linux平台实现)
│   │   ├── CocoaWindow (macOS平台实现)
│   │   └── ...
│   └── DisplayManager (显示器管理)
├── EventSystem (事件系统实现)
│   ├── EventQueue (事件队列，线程安全)
│   └── PlatformEventPump (平台事件泵)
└── MainLoop (主循环实现)
    ├── TimeManager (时间管理)
    ├── TickScheduler (Tick调度器，按优先级排序)
    └── FrameRateController (帧率控制器)
```

**简化**：移除EventDispatcher（事件分发器），事件队列直接暴露；TickScheduler按优先级排序，不强制阶段划分。

## 3. 实现架构

### 3.1 平台抽象层（PAL）

```
Platform Abstraction Layer
├── IWindowPlatform (平台窗口抽象接口)
│   ├── WindowsWindowPlatform (Win32实现)
│   ├── X11WindowPlatform (X11实现)
│   ├── CocoaWindowPlatform (Cocoa实现)
│   ├── AndroidWindowPlatform (Android实现)
│   └── IOSWindowPlatform (iOS实现)
├── IEventPumpPlatform (平台事件泵抽象接口)
│   ├── WindowsEventPumpPlatform (Win32消息循环)
│   ├── X11EventPumpPlatform (X11事件循环)
│   ├── CocoaEventPumpPlatform (Cocoa事件循环)
│   ├── AndroidEventPumpPlatform (Android事件循环)
│   └── IOSEventPumpPlatform (iOS事件循环)
└── 工厂函数
    ├── CreateWindowPlatform() (根据TE_PLATFORM_*宏创建)
    └── CreateEventPumpPlatform() (根据TE_PLATFORM_*宏创建)
```

**设计改进**：
- 使用struct而非class定义接口（对齐Core模块风格）
- 清晰的接口分离（窗口和事件泵分开）
- 显示器管理整合到IWindowPlatform中
- 使用TE_PLATFORM_*宏进行平台检测

### 3.2 模块组织

```
Engine/TenEngine-003-application/
├── CMakeLists.txt
├── include/
│   └── te/
│       └── application/
│           ├── Application.h          # IApplication接口（整合所有功能）
│           ├── Window.h               # WindowId, WindowDesc, DisplayInfo
│           ├── Event.h                 # Event, EventType, EventQueue
│           ├── MainLoop.h              # TimeStepMode, TickCallback
│           └── Platform.h              # IWindowPlatform, IEventPumpPlatform（新增）
├── src/
│   ├── Application.cpp                 # Application实现
│   ├── ApplicationImpl.cpp             # Application实现细节（可选）
│   ├── EventQueue.cpp                 # EventQueue实现
│   ├── window/
│   │   ├── WindowManager.cpp
│   │   └── Window.cpp
│   ├── event/
│   │   └── EventSystem.cpp
│   ├── mainloop/
│   │   ├── MainLoop.cpp
│   │   └── TimeManager.cpp
│   └── platform/
│       ├── PlatformAbstraction.cpp     # 平台工厂函数实现（新增）
│       ├── windows/
│       │   ├── WindowsWindowPlatform.cpp
│       │   └── WindowsEventPumpPlatform.cpp
│       ├── x11/
│       │   ├── X11WindowPlatform.cpp
│       │   └── X11EventPumpPlatform.cpp
│       └── cocoa/
│           ├── CocoaWindowPlatform.mm
│           └── CocoaEventPumpPlatform.mm
└── tests/
    ├── unit/
    │   ├── test_application.cpp
    │   ├── test_window.cpp
    │   ├── test_event.cpp
    │   └── test_platform.cpp           # 平台抽象测试（新增）
    └── integration/
        └── test_main_loop.cpp
```

## 4. 主循环实现示例

### 4.1 可变时间步主循环

```cpp
void Application::Run(RunParams const& args) {
    // 初始化
    if (!Initialize()) {
        return;
    }
    
    // 创建主窗口
    WindowDesc desc;
    desc.title = args.windowTitle;
    desc.width = args.windowWidth;
    desc.height = args.windowHeight;
    WindowId mainWindow = CreateWindow(desc);
    
    // 设置运行模式
    SetRunMode(args.runMode);
    
    // 主循环
    TimeManager timeManager;
    bool shouldExit = false;
    
    while (!shouldExit) {
        // 1. 轮询事件
        PumpEvents();
        
        // 2. 处理退出请求
        if (m_exitRequested) {
            shouldExit = true;
            break;
        }
        
        // 3. 更新时间
        float deltaTime = timeManager.Update();
        
        // 4. 执行Tick回调（按优先级排序）
        ExecuteTickCallbacks(deltaTime);
        
        // 5. 用户回调（兼容旧接口）
        if (args.tickCallback) {
            args.tickCallback(deltaTime);
        }
        
        // 6. 帧率控制
        timeManager.SleepForTargetFPS(m_targetFPS);
        
        m_frameCount++;
    }
    
    // 清理
    DestroyWindow(mainWindow);
}
```

### 4.2 固定时间步主循环

```cpp
void Application::RunFixedTimeStep(RunParams const& args) {
    const float fixedDeltaTime = 1.0f / 60.0f; // 60 FPS
    float accumulator = 0.0f;
    
    TimeManager timeManager;
    
    while (!m_exitRequested) {
        PumpEvents();
        
        float frameTime = timeManager.GetFrameTime();
        accumulator += frameTime;
        
        // 固定时间步更新
        while (accumulator >= fixedDeltaTime) {
            ExecuteTickCallbacks(fixedDeltaTime);
            accumulator -= fixedDeltaTime;
        }
        
        // 可变时间步渲染（如果需要）
        float alpha = accumulator / fixedDeltaTime;
        
        timeManager.SleepForTargetFPS(m_targetFPS);
    }
}
```

## 5. 事件系统实现示例

### 5.1 事件分发（简化设计）

```cpp
void EventSystem::PumpEvents() {
    // 1. 从平台获取原始事件
    PlatformEvent platformEvent;
    while (m_eventPump->PollEvent(platformEvent)) {
        // 2. 检查是否为退出事件
        if (m_eventPump->IsQuitEvent(platformEvent)) {
            RequestExit(0);
            continue;
        }
        
        // 3. 转换为引擎事件
        Event engineEvent = m_eventPump->ConvertToEngineEvent(platformEvent, GetCurrentWindowId());
        
        // 4. 推送到队列（供Input模块消费）
        m_eventQueue.Push(engineEvent);
        
        // 5. 触发窗口回调（如果有）
        TriggerWindowCallbacks(engineEvent);
    }
}
```

**简化**：移除事件过滤器和订阅机制，直接推送到队列，更简单高效。

### 5.2 事件队列访问（简化设计）

```cpp
// Input模块直接消费事件队列
void InputModule::ProcessEvents(IApplication* app) {
    EventQueue const& queue = app->GetEventQueue();
    Event event;
    while (queue.Pop(event)) {
        // 处理事件
        ProcessEvent(event);
    }
}
```

**简化**：移除复杂的事件订阅机制，事件队列直接暴露，Input模块直接消费，更简单高效。

## 6. 线程安全考虑

### 6.1 窗口操作

- **约束**：所有窗口操作必须在主线程执行（平台限制）
- **实现**：使用线程本地存储或主线程检查

### 6.2 事件队列

- **实现**：使用线程安全的队列（如无锁队列或互斥锁保护）
- **消费**：Input模块可在主线程或工作线程消费事件

### 6.3 主循环

- **约束**：主循环必须在主线程运行
- **Tick回调**：可在不同线程执行，但需要同步机制

## 7. 资源管理

### 7.1 窗口生命周期

```cpp
class Window {
public:
    Window(WindowDesc const& desc);
    ~Window();
    
    // 禁止拷贝
    Window(Window const&) = delete;
    Window& operator=(Window const&) = delete;
    
    // 移动语义
    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;
};
```

### 7.2 RAII管理

- 窗口创建时自动注册到WindowManager
- 窗口销毁时自动从WindowManager移除
- 使用智能指针管理窗口生命周期

## 8. 错误处理

### 8.1 初始化失败

```cpp
bool Application::Initialize(InitParams const* params) {
    if (m_initialized) {
        return true; // 幂等
    }
    
    // 初始化平台抽象层
    if (!InitializePlatform()) {
        LogError("Failed to initialize platform");
        return false;
    }
    
    // 初始化窗口管理器
    if (!m_windowManager.Initialize()) {
        LogError("Failed to initialize window manager");
        return false;
    }
    
    m_initialized = true;
    return true;
}
```

### 8.2 窗口创建失败

```cpp
WindowId Application::CreateWindow(WindowDesc const& desc) {
    try {
        auto window = std::make_unique<Window>(desc);
        WindowId id = window->GetId();
        m_windows[id] = std::move(window);
        return id;
    } catch (std::exception const& e) {
        LogError("Failed to create window: %s", e.what());
        return InvalidWindowId;
    }
}
```

## 9. 测试策略

### 9.1 单元测试

- 测试窗口创建/销毁
- 测试事件队列操作（Push/Pop）
- 测试主循环时间管理
- 测试Tick回调注册/取消（优先级排序）

### 9.2 集成测试

- 测试完整的主循环流程
- 测试窗口事件处理
- 测试多窗口场景
- 测试不同时间步模式

### 9.3 平台测试

- 测试各平台的窗口创建
- 测试各平台的事件泵
- 测试跨平台兼容性

## 10. 性能考虑

### 10.1 事件队列

- 使用环形缓冲区避免内存分配
- 批量处理事件减少函数调用开销

### 10.2 Tick调度

- 按优先级排序回调，减少查找时间
- 使用函数指针表优化回调调用

### 10.3 时间管理

- 使用高精度计时器（Core模块提供）
- 避免频繁的系统调用

## 11. 扩展性考虑

### 11.1 插件系统

- 允许外部注册自定义事件类型
- 允许外部注册自定义Tick阶段

### 11.2 配置系统

- 支持配置文件设置窗口属性
- 支持运行时修改主循环参数

## 12. 平台抽象层实现示例

### 12.1 Windows平台实现

```cpp
// src/platform/windows/WindowsWindowPlatform.cpp
#include "te/application/Platform.h"
#include "te/core/platform.h"

#if TE_PLATFORM_WINDOWS
#include <windows.h>

namespace te {
namespace application {

class WindowsWindowPlatform : public IWindowPlatform {
public:
    void* CreateNativeWindow(WindowDesc const& desc) override {
        // Win32窗口创建实现
        HWND hwnd = CreateWindowEx(...);
        return hwnd;
    }
    
    void DestroyNativeWindow(void* handle) override {
        DestroyWindow(static_cast<HWND>(handle));
    }
    
    // ... 其他方法实现
};

}  // namespace application
}  // namespace te

IWindowPlatform* CreateWindowPlatform() {
    return new WindowsWindowPlatform();
}

#endif  // TE_PLATFORM_WINDOWS
```

### 12.2 平台工厂函数

```cpp
// src/platform/PlatformAbstraction.cpp
IWindowPlatform* CreateWindowPlatform() {
#if TE_PLATFORM_WINDOWS
    return new WindowsWindowPlatform();
#elif TE_PLATFORM_LINUX
    return new X11WindowPlatform();
#elif TE_PLATFORM_MACOS
    return new CocoaWindowPlatform();
#elif TE_PLATFORM_ANDROID
    return new AndroidWindowPlatform();
#elif TE_PLATFORM_IOS
    return new IOSWindowPlatform();
#else
    return nullptr;
#endif
}
```

## 13. 参考实现

- **GLFW**：跨平台窗口和事件处理库
- **SDL2**：多媒体库，包含窗口和事件系统
- **Unreal Engine**：ApplicationCore模块
- **Unity**：Application类和相关系统
- **Core模块**：Allocator设计模式、命名规范、注释风格

## 14. 与Core模块对齐的具体实现

1. **注释风格**：统一使用`/** */`格式，添加`per contract`标注
2. **命名规范**：统一使用PascalCase（CreateWindow、PumpEvents、GetDeltaTime）
3. **接口组织**：public方法使用`public:`，对齐Core模块风格
4. **错误处理**：使用Core模块的日志系统（Log、CheckError等）
5. **资源管理**：使用RAII模式，智能指针管理资源
6. **平台宏**：使用TE_PLATFORM_*宏进行平台检测

---

*本文档作为实现参考，具体实现细节可根据实际情况调整。*
