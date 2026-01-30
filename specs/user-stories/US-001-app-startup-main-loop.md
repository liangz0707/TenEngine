# US-001：应用启动并进入主循环（已迁移至领域）

- **状态**：已迁移至 **lifecycle** 领域，新编号 **US-lifecycle-001**。
- **正式文档**：[domains/lifecycle/US-lifecycle-001-app-startup-main-loop.md](./domains/lifecycle/US-lifecycle-001-app-startup-main-loop.md)
- **领域索引**：[domains/lifecycle/index.md](./domains/lifecycle/index.md)

以下为原内容备份，以领域内文档为准。

---

# US-lifecycle-001：应用启动并进入主循环（原 US-001）

- **标题**：应用启动并进入主循环
- **编号**：US-001（现 US-lifecycle-001）

---

## 1. 角色/触发

- **角色**：引擎/游戏进程
- **触发**：进程入口（如 `main`）调用引擎初始化与运行接口，期望得到可交互的窗口与持续的消息循环，直到用户退出。

---

## 2. 端到端流程

1. 进程启动，调用引擎「初始化」入口（如 `Engine::init()` 或 `Application::run(args)`）。
2. **Core**：初始化全局基础设施（可选：日志、默认分配器、模块加载器）。
3. **Application**：创建主窗口、绑定到当前线程；可选：设置窗口标题/尺寸。
4. **Subsystems**：按依赖顺序初始化已注册的子系统（Input、RHI、Resource 等）。
5. **主循环**：每帧执行「轮询事件 → 更新 → 渲染」（渲染细节见 US-002）；循环由 Application 或上层驱动。
6. 用户请求退出（关闭窗口或调用退出 API）→ Application 退出主循环。
7. **Subsystems**：按逆序关闭各子系统；**Application** 销毁窗口；**Core** 做最终清理（若有）。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 001-Core | 进程级初始化/关闭、日志、分配器；可选模块加载 |
| 003-Application | 窗口创建与生命周期、消息循环、事件轮询、退出请求 |
| 007-Subsystems | 子系统注册与按序初始化/关闭 |

---

## 4. 每模块职责与 I/O

### 001-Core

- **职责**：提供进程级初始化与关闭；日志、默认分配器在首次使用前可用。
- **输入**：无（或 `InitParams`：日志路径、分配器策略等）。
- **输出**：`bool init()` / `void shutdown()`；全局或单例的 Log、Allocator 可用。

### 003-Application

- **职责**：创建/销毁主窗口；运行主循环（轮询事件、每帧回调）；处理退出请求。
- **输入**：`RunParams`（窗口标题、宽高、每帧回调等）；来自窗口系统的关闭事件。
- **输出**：窗口句柄（或不暴露）；主循环控制；`pollEvents()`；`requestQuit()` / `shouldQuit()`。

### 007-Subsystems

- **职责**：注册子系统；按依赖顺序 `initAll()`；主循环结束前按逆序 `shutdownAll()`。
- **输入**：各子系统实现 `ISubsystem`（init/shutdown）；依赖顺序由注册顺序或依赖图决定。
- **输出**：`registerSubsystem()`；`initAll()` / `shutdownAll()`；可选 `getSubsystem<T>()`。

---

## 5. 派生接口（ABI 条目）

以下按 `docs/engine-abi-interface-generation-spec.md` 书写，命名空间 **TenEngine**，函数/方法**驼峰**，头文件**大写开头驼峰**。

### 001-Core

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 001-Core | TenEngine::core | — | 自由函数 | 进程级初始化 | TenEngine/core/Engine.h | init | bool init(InitParams const* params); 失败返回 false，可重复调用时幂等 |
| 001-Core | TenEngine::core | — | 自由函数 | 进程级关闭 | TenEngine/core/Engine.h | shutdown | void shutdown(); 在进程退出前调用，init 之后仅调用一次 |
| 001-Core | TenEngine::core | — | struct | 初始化参数 | TenEngine/core/Engine.h | InitParams | 可选：log_path, allocator_policy；下游按需填充 |

### 003-Application

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 003-Application | TenEngine::application | IApplication | 抽象接口 | 应用生命周期与主循环 | TenEngine/application/Application.h | IApplication::run, IApplication::requestQuit | run(args) 进入主循环直至退出；requestQuit() 请求下一帧退出 |
| 003-Application | TenEngine::application | — | 自由函数 | 创建应用实例 | TenEngine/application/Application.h | createApplication | IApplication* createApplication(); 调用方负责释放或由引擎管理 |
| 003-Application | TenEngine::application | — | struct | 运行参数 | TenEngine/application/Application.h | RunParams | 窗口标题、宽高、每帧回调 TickCallback；下游填充 |
| 003-Application | TenEngine::application | — | 回调类型 | 每帧回调 | TenEngine/application/Application.h | TickCallback | void (*)(float delta_time); 主循环每帧调用一次 |

### 007-Subsystems

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 007-Subsystems | TenEngine::subsystems | ISubsystem | 抽象接口 | 子系统生命周期 | TenEngine/subsystems/Subsystem.h | ISubsystem::init, ISubsystem::shutdown | bool init(); void shutdown(); 由 SubsystemRegistry 按序调用 |
| 007-Subsystems | TenEngine::subsystems | SubsystemRegistry | 单例 | 子系统注册与批量初始化 | TenEngine/subsystems/SubsystemRegistry.h | SubsystemRegistry::getInstance, registerSubsystem, initAll, shutdownAll | getInstance() 后 registerSubsystem(ptr)；initAll() 按注册顺序 init；shutdownAll() 逆序 shutdown |
| 007-Subsystems | TenEngine::subsystems | SubsystemRegistry | 模板方法 | 按类型获取子系统 | TenEngine/subsystems/SubsystemRegistry.h | SubsystemRegistry::getSubsystem<T> | 返回已注册的 T*；未注册返回 nullptr |

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/001-core-ABI.md`、`003-application-ABI.md`、`007-subsystems-ABI.md`。*
