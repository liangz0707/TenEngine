# US-lifecycle-002：用户可选择编辑器启动或游戏模式启动

- **标题**：用户可选择编辑器启动或游戏模式启动；编辑器启动可操作编辑器制作场景、控制资源；游戏模式启动可运行脚本与引擎交互。
- **编号**：US-lifecycle-002

---

## 1. 角色/触发

- **角色**：用户（开发者或玩家）
- **触发**：启动引擎进程时，通过命令行参数、配置文件或启动器选择**编辑器模式**或**游戏模式**；两种模式共用同一可执行体或不同入口，但均由同一套 Application/Engine 生命周期驱动。

---

## 2. 端到端流程

### 2.1 选择启动模式

1. 用户启动进程（如 `TenEngine.exe --editor` 或 `TenEngine.exe --game`，或由启动器/配置指定）。
2. **Application**（或 Engine 入口）解析启动参数/配置，得到 **RunMode**：`Editor` 或 `Game`。
3. **Core** 与 **Subsystems** 初始化（同 US-lifecycle-001）；随后根据 RunMode 进入不同主循环分支。

### 2.2 编辑器模式（RunMode == Editor）

1. **Application** 创建主窗口后，不直接进入「游戏主循环」，而是启动 **Editor** 子系统。
2. **Editor**：提供编辑器 UI（视口、场景树、属性面板、资源浏览器）；用户可**制作场景**（放置/移动/删除节点、编辑属性）、**控制资源**（导入、查看、引用、保存）。
3. 主循环由 Editor 驱动：每帧处理输入、更新视口与 UI、可选运行「游戏预览」子模式（见下）；用户点击「运行」时可切换到游戏模式预览，或另起进程运行游戏。
4. 退出时 **Editor** 先关闭，再按 US-lifecycle-001 逆序关闭 Subsystems、Application、Core。

### 2.3 游戏模式（RunMode == Game）

1. **Application** 创建主窗口（或无窗口、headless）后，进入**游戏主循环**：每帧轮询事件、调用 **TickCallback**（或脚本层注册的 Update）。
2. **脚本**与引擎交互：脚本通过引擎提供的 API（场景、实体、输入、资源等）驱动逻辑；脚本运行时可由 **Subsystems** 注册（如 ScriptSubsystem）或由上层注入回调实现。
3. 主循环直至用户退出（关闭窗口、调用 requestQuit、或脚本请求退出）；随后按 US-lifecycle-001 关闭 Subsystems、Application、Core。

### 2.4 与 US-lifecycle-001 的关系

- US-lifecycle-001 描述「进程初始化 + 主循环 + 子系统 init/shutdown」的通用流程；本故事在**主循环之前**增加「按 RunMode 分支」，在**主循环之内**区分 Editor 分支（Editor UI + 场景/资源操作）与 Game 分支（脚本 + 引擎 API 交互）。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 001-Core | 进程级初始化/关闭（同 US-lifecycle-001） |
| 003-Application | 解析启动模式（RunMode）；根据模式创建窗口并进入 Editor 主循环或 Game 主循环 |
| 007-Subsystems | 子系统注册与按序初始化/关闭；Editor 模式下注册 Editor 子系统，Game 模式下可选注册脚本子系统 |
| 024-Editor | 仅在 Editor 模式：提供编辑器 UI（视口、场景树、属性面板、资源浏览器）、场景编辑、资源控制 |

---

## 4. 每模块职责与 I/O

### 003-Application

- **职责**：在 run 之前或 run 参数中确定 **RunMode**（Editor | Game）；run(RunParams) 时若 mode==Editor 则启动 Editor 并交由其驱动主循环，若 mode==Game 则直接使用 RunParams 中的 TickCallback 驱动游戏主循环。
- **输入**：`RunParams` 增加 `runMode`（RunMode）；可选：命令行/配置由调用方解析后填入 RunParams。
- **输出**：`RunMode` 枚举；`run(RunParams)` 行为按 `runMode` 分支；可选 `getRunMode()` 供子系统查询。

### 007-Subsystems

- **职责**：与 US-lifecycle-001 一致；在 Editor 模式下，**Editor** 作为子系统注册并在 initAll 后由 Application 启动主循环（Editor 接管消息循环）；在 Game 模式下，可选注册「脚本」相关子系统，在 TickCallback 中由脚本层调用。
- **输入**：RunMode 决定是否注册 Editor 子系统；其他同 US-lifecycle-001。
- **输出**：同 US-lifecycle-001；Editor 实现 ISubsystem 并在 init 后提供 IEditor。

### 024-Editor

- **职责**：仅在 **RunMode == Editor** 时激活。提供 **IEditor**：主窗口内嵌编辑器 UI（视口、场景树、属性面板、资源浏览器）；用户可制作场景（节点增删改、属性编辑）、控制资源（导入、查看、引用、保存）；可提供「运行游戏预览」入口（切换为游戏模式或子进程）。
- **输入**：由 Application 在 Editor 模式下创建并传入窗口句柄或上下文；来自 Input、RHI、Scene、Entity、Resource 等模块的只读/读写接口。
- **输出**：`IEditor` 接口；`createEditor(context)` 或由 SubsystemRegistry 获取；`IEditor::run()` 进入编辑器主循环直至用户关闭；场景/资源操作通过 Editor 内部调用 Scene、Resource 等模块 API。

---

## 5. 派生接口（ABI 条目）

以下按 `docs/engine-abi-interface-generation-spec.md` 书写。

### 003-Application

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 003-Application | TenEngine::application | — | 枚举 | 运行模式 | TenEngine/application/Application.h | RunMode | enum class RunMode { Editor, Game }; 编辑器启动 vs 游戏启动 |
| 003-Application | TenEngine::application | — | struct | 运行参数（含模式） | TenEngine/application/Application.h | RunParams | 增加 runMode: RunMode；其余同 US-lifecycle-001（窗口标题、宽高、TickCallback 等）；mode==Game 时 TickCallback 为游戏主循环回调 |
| 003-Application | TenEngine::application | IApplication | 抽象接口 | 获取当前运行模式 | TenEngine/application/Application.h | IApplication::getRunMode | RunMode getRunMode() const; 供子系统或脚本查询当前是编辑器还是游戏 |

### 007-Subsystems

- 无新增符号；Editor 作为实现 ISubsystem 的子系统注册，通过 `getSubsystem<IEditor>()` 或约定获取（见 024-Editor）。

### 024-Editor

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 024-Editor | TenEngine::editor | IEditor | 抽象接口 | 编辑器主入口 | TenEngine/editor/Editor.h | IEditor::run | void run(EditorContext const& ctx); 进入编辑器主循环（视口、场景树、属性、资源浏览器），直至用户关闭；仅在 RunMode::Editor 时由 Application 调用 |
| 024-Editor | TenEngine::editor | — | struct | 编辑器上下文 | TenEngine/editor/Editor.h | EditorContext | 窗口句柄、可选 RHI/Scene/Resource 等句柄；由 Application 或 SubsystemRegistry 在启动 Editor 时填充 |
| 024-Editor | TenEngine::editor | — | 自由函数/工厂 | 创建编辑器实例 | TenEngine/editor/Editor.h | createEditor | IEditor* createEditor(EditorContext const& ctx); 或通过 SubsystemRegistry::getSubsystem<IEditor>() 获取；调用方不拥有指针时由引擎管理生命周期 |
| 024-Editor | TenEngine::editor | IEditor | 抽象接口 | 场景编辑能力（可选细化） | TenEngine/editor/SceneView.h | IEditor::getSceneView | 返回场景树/视口控制接口；制作场景（节点增删改、属性）通过该接口或单独 ISceneView |
| 024-Editor | TenEngine::editor | IEditor | 抽象接口 | 资源控制能力（可选细化） | TenEngine/editor/ResourceView.h | IEditor::getResourceView | 返回资源浏览器接口；控制资源（导入、查看、引用、保存）通过该接口或单独 IResourceView |

---

## 6. 参考（可选）

- **Unity**：Editor 与 Player 分离（Editor 为独立进程/包）；Play Mode 与 Edit Mode 切换；[Unity Editor](https://docs.unity3d.com/Manual/UnityEditor.html)。
- **Unreal**：Editor 与 Game 同一进程，通过 GEngine 与 GWorld 区分；Slate 编辑器 UI；[Unreal Editor](https://docs.unrealengine.com/en-US/ProductionPipelines/Developers/Editor/)。

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/003-application-ABI.md`、`024-editor-ABI.md`。与 US-lifecycle-001 互补：001 为通用生命周期，002 为启动模式与 Editor/Game 分支。*
