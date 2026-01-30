# T0 模块 ABI 总索引

本文件为 **TenEngine T0 架构（27 模块）** 的 **ABI 总索引**：汇总各模块对外符号的命名空间、头文件与符号列表，供实现与下游消费时统一查阅。

- **契约**：各模块的能力与类型见 `NNN-modulename-public-api.md`；契约文件引用对应的 ABI 文件；接口符号以 ABI 文件为准。
- **本索引**：引用全部模块的 ABI 文件；各 ABI 文件列出该模块的显式 ABI 表，列定义与导出形式约定见下。

## ABI 表列定义与导出形式约定

### 列定义（必填）

| 列 | 含义 | 示例 |
|----|------|------|
| **模块名** | 模块 ID（如 001-Core） | 001-Core |
| **命名空间** | C++ 命名空间，下游 include 后使用的限定前缀 | te::core |
| **类名** | 所属类型名；自由函数/全局枚举/全局常量填 **—** | Allocator、— |
| **接口说明** | 一句话职责 | 全局堆分配、日志级别枚举 |
| **头文件** | 相对 include 路径（下游以该路径 include） | te/core/alloc.h |
| **符号** | 下游使用的符号名：自由函数/枚举名/常量名，或 类型名、类型名::方法、类型名::枚举值 | Alloc、LogLevel、Registry::Instance |
| **说明** | 签名摘要、线程安全、生命周期等 | void* Alloc(size_t, size_t); 失败 nullptr |

### 导出形式（可选列或写在说明中）

实现可能采用多种 C++ 导出方式，ABI 表需能区分，便于下游正确调用。建议在表中增加 **导出形式** 列，或在本模块 ABI 文件开头的「约定」中说明本模块所用形式；符号与说明列中也可直接写出调用方式。

| 导出形式 | 类名列 | 符号列示例 | 调用方式 |
|----------|--------|------------|----------|
| **自由函数** | — | Alloc, Free, Log | 命名空间::函数名(…) |
| **全局枚举** | — 或 枚举名 | LogLevel, Backend | 命名空间::枚举名::值 |
| **全局常量** | — | kMaxCount, DefaultAlignment | 命名空间::常量名 |
| **struct/class 类型** | 类型名 | Vector3, IDevice | 类型为值类型或句柄；构造/工厂见说明 |
| **抽象接口（多态）** | 接口名 | IDevice, Allocator | 通过工厂或返回接口指针；不直接构造 |
| **静态方法** | 类名 | TypeRegistry::Register | 命名空间::类名::方法(…) |
| **单例** | 类名 | Engine::Instance(), Registry::Get() | 先取实例再调成员，或 类名::Instance()->Method() |
| **模板类/函数** | 类名或 — | Array&lt;T&gt;, clamp&lt;T&gt; | 符号写 类型名&lt;T&gt; 或 函数名&lt;T&gt;；说明中写约束与特化约定 |
| **模板方法** | 类名 | Registry::GetSubsystem&lt;T&gt;() | 类名::方法&lt;T&gt;(…)；说明中写约束 |

### 调用策略与约定（建议在说明或契约中写清）

- **获取实例**：自由函数无需实例；单例写清是 `Instance()` / `Get()` / 其他；接口写清工厂函数或从哪获取指针。
- **struct vs class**：若影响用法（值拷贝 vs 引用/指针），在说明中注明「值类型」或「句柄/接口，不可拷贝」。
- **枚举**：注明底层类型（如 enum class LogLevel : unsigned）若与 ABI 稳定相关；枚举值列在说明或单独行。
- **线程安全**：若该符号有线程安全要求，在说明中写「线程安全」或「单线程调用」。

### 示例（多种形式混在同一表）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 001-Core | te::core | — | 自由函数 | 全局堆分配 | te/core/alloc.h | Alloc | void* Alloc(size_t, size_t); 失败 nullptr |
| 001-Core | te::core | — | 全局枚举 | 日志级别 | te/core/log.h | LogLevel | enum class LogLevel { Debug, Info, Warn, Error }; |
| 001-Core | te::core | Registry | 单例 | 类型注册表 | te/object/TypeRegistry.hpp | Registry::Instance | 单例；Instance() 后调 Register/GetTypeByName |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 图形设备 | te/rhi/device.hpp | CreateDevice, DestroyDevice | 工厂函数返回 IDevice*；不直接构造 |

各模块 ABI 文件可保留原有 7 列（不强制加「导出形式」），在 **说明** 列中写出调用方式与导出形式即可；若希望机器可解析或统一检索，可增加 **导出形式** 列。

## 各模块 ABI 文件

| 模块 | ABI 文件 | 说明 |
|------|----------|------|
| 001-Core | [001-core-ABI.md](./001-core-ABI.md) | 内存、线程、平台、日志、数学、容器、模块加载 |
| 002-Object | [002-object-ABI.md](./002-object-ABI.md) | 反射、序列化、属性、类型注册 |
| 003-Application | [003-application-ABI.md](./003-application-ABI.md) | 窗口、消息循环、生命周期 |
| 004-Scene | [004-scene-ABI.md](./004-scene-ABI.md) | 场景图、节点、空间 |
| 005-Entity | [005-entity-ABI.md](./005-entity-ABI.md) | 实体、组件、层级、查询 |
| 006-Input | [006-input-ABI.md](./006-input-ABI.md) | 输入设备、事件、映射 |
| 007-Subsystems | [007-subsystems-ABI.md](./007-subsystems-ABI.md) | 子系统注册、生命周期 |
| 008-RHI | [008-rhi-ABI.md](./008-rhi-ABI.md) | 设备、命令列表、资源、PSO、同步 |
| 009-RenderCore | [009-rendercore-ABI.md](./009-rendercore-ABI.md) | Uniform、Pass 资源描述 |
| 010-Shader | [010-shader-ABI.md](./010-shader-ABI.md) | 编译、变体、Bytecode |
| 011-Material | [011-material-ABI.md](./011-material-ABI.md) | 材质定义、参数、实例 |
| 012-Mesh | [012-mesh-ABI.md](./012-mesh-ABI.md) | 顶点/索引、子网格、LOD |
| 013-Resource | [013-resource-ABI.md](./013-resource-ABI.md) | 导入、加载、流式、ResourceId |
| 014-Physics | [014-physics-ABI.md](./014-physics-ABI.md) | 碰撞、刚体、查询 |
| 015-Animation | [015-animation-ABI.md](./015-animation-ABI.md) | 剪辑、骨骼、播放 |
| 016-Audio | [016-audio-ABI.md](./016-audio-ABI.md) | 音源、监听、混音 |
| 017-UICore | [017-uicore-ABI.md](./017-uicore-ABI.md) | 布局、绘制列表、焦点 |
| 018-UI | [018-ui-ABI.md](./018-ui-ABI.md) | 画布、控件树、事件 |
| 019-PipelineCore | [019-pipelinecore-ABI.md](./019-pipelinecore-ABI.md) | Pass 图、资源生命周期、提交 |
| 020-Pipeline | [020-pipeline-ABI.md](./020-pipeline-ABI.md) | 管线上下文、渲染目标、DrawCall |
| 021-Effects | [021-effects-ABI.md](./021-effects-ABI.md) | 后处理、粒子、VFX |
| 022-2D | [022-2d-ABI.md](./022-2d-ABI.md) | 精灵、Tilemap、2D 相机 |
| 023-Terrain | [023-terrain-ABI.md](./023-terrain-ABI.md) | 地形、块、LOD |
| 024-Editor | [024-editor-ABI.md](./024-editor-ABI.md) | 视口、场景树、属性、资源浏览器 |
| 025-Tools | [025-tools-ABI.md](./025-tools-ABI.md) | 构建、批处理、CLI |
| 026-Networking | [026-networking-ABI.md](./026-networking-ABI.md) | 复制、RPC、连接 |
| 027-XR | [027-xr-ABI.md](./027-xr-ABI.md) | XR 会话、帧、提交 |

**说明**：ABI 表格式与列定义、导出形式约定见上文「ABI 表列定义与导出形式约定」；各模块 ABI 文件至少含 7 列（模块名、命名空间、类名、接口说明、头文件、符号、说明），可选增加「导出形式」列或在说明中写明调用方式与导出形式。契约与 ABI 同步维护于 **T0-contracts** 分支。
