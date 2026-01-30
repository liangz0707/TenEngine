# 契约：007-Subsystems 模块对外 API

## 适用模块

- **实现方**：**007-Subsystems**（T0 可插拔子系统）
- **对应规格**：`docs/module-specs/007-subsystems.md`
- **依赖**：001-Core（001-core-public-api）, 002-Object（002-object-public-api）

## 消费者（T0 下游）

- 027-XR（作为子系统挂接）, 024-Editor（子系统列表与开关）。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SubsystemDescriptor | 子系统元数据、依赖列表、优先级、平台过滤 | 注册后直至卸载 |
| ISubsystem / GetSubsystem<T> | 子系统实例、按类型/接口查询 | 与生命周期 Start/Stop 一致 |
| 生命周期钩子 | Initialize、Start、Stop、Shutdown | 按依赖顺序调用 |

## 能力列表（提供方保证）

1. **描述符**：SubsystemDescriptor、Dependencies、Priority、PlatformFilter；可序列化、反射（Object）。
2. **注册表**：Register、GetSubsystem<T>、Unregister；按类型查询、单例或按实例管理。
3. **生命周期**：InitializeAll、StartAll、StopAll、ShutdownAll；依赖拓扑排序、与主循环或按需调用。
4. **可选发现**：ScanPlugins、RegisterFromPlugin；与 Core.ModuleLoad 配合。

## 调用顺序与约束

- 须在 Core、Object 可用之后使用；子系统启动/停止顺序按依赖图保证。
- XR、Display 等子系统作为实现挂接于本模块；契约仅约定注册、查询与生命周期接口。

## API 雏形（007-subsystems-complete-feature）

来源：plan `007-subsystems-complete-feature` 末尾 **契约更新**。实现须仅使用 001-Core、002-Object 契约已声明的类型与 API。

### 类型与句柄（本 feature 暴露）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SubsystemDescriptor | 子系统元数据、Dependencies、Priority、PlatformFilter | 注册后直至卸载 |
| ISubsystem | 子系统实例抽象；Initialize / Start / Stop / Shutdown | 与 Start/Stop 一致 |
| 模块句柄 | 动态库句柄（见 Core 模块加载） | Load 后直至 Unload |

### 描述符（Descriptor）

```cpp
namespace te::subsystems {

struct SubsystemDescriptor {
    void const*       typeInfo;         // 与 Object TypeId 或 TypeDescriptor 对应
    char const* const* dependencies;    // 依赖的子系统类型名或 ID，null 结尾或配 dependencyCount
    size_t            dependencyCount;
    int               priority;
    uint32_t          platformFilter;   // 平台过滤；语义与 Core 平台检测一致
};

}
```

### 子系统接口与注册表（Registry）

```cpp
namespace te::subsystems {

class ISubsystem {
public:
    virtual ~ISubsystem() = default;
    virtual void Initialize() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Shutdown() = 0;
};

class Registry {
public:
    // 注册。重复同类型 → 返回 false，不修改注册表。instance 由 Registry 接管所有权。
    static bool Register(SubsystemDescriptor const& desc, std::unique_ptr<ISubsystem> instance);

    template<typename T>
    static T* GetSubsystem();   // 未注册或 ShutdownAll 后返回 nullptr

    static void Unregister(void const* typeInfo);  // 按 typeInfo 移除
};

}
```

### 生命周期（Lifecycle）

```cpp
namespace te::subsystems {

class Lifecycle {
public:
    // 依赖拓扑分层，同层按 Priority：初始化升序、关闭降序。仅主循环单线程调用。
    static bool InitializeAll(Registry const& reg);  // 循环依赖时返回 false
    static void StartAll(Registry const& reg);
    static void StopAll(Registry const& reg);
    static void ShutdownAll(Registry& reg);
};

}
```

### 发现（Discovery）

```cpp
namespace te::subsystems {

class Discovery {
public:
    static bool ScanPlugins(Registry& reg);
    static bool RegisterFromPlugin(Registry& reg, void* moduleHandle);  // Core 模块句柄
};

}
```

### 调用顺序与约束（本 feature）

- 须在 Core、Object 可用之后使用。
- 生命周期仅由引擎主循环单线程调用；应用不直接调用 InitializeAll / StartAll / StopAll / ShutdownAll。
- 重复注册同一类型 → `Register` 返回 `false`；`GetSubsystem<T>` 在 ShutdownAll 之后返回 `nullptr`。
- `platformFilter`、`dependencies` 所用平台与类型标识须与 Core、Object 契约一致；序列化/反射经 Object 能力实现。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 007-Subsystems 模块规格与依赖表新增契约 |
| 2026-01-30 | API 雏形由 plan 007-subsystems-complete-feature 同步（契约更新写回） |
