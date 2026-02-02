# US-rendering-005：Shader 支持 HLSL/GLSL、宏切换与实时更新

- **标题**：Shader 可加载 HLSL、GLSL 两种格式；支持宏切换代码路径；游戏中可动态切换宏；支持 Shader 实时更新（热重载），无需重启应用即可生效。
- **编号**：US-rendering-005

---

## 1. 角色/触发

- **角色**：程序员（引擎或游戏侧）、美术/TA（材质与 Shader 调试）
- **触发**：需要从 **HLSL** 或 **GLSL** 源码加载并编译 Shader；在 Shader 内使用**宏**切换不同代码路径（如 LOD、特性开关）；在**游戏运行中**根据逻辑或调试需要**动态切换宏**；在修改 Shader 源码或宏后希望**实时更新** Shader 并立即在画面中看到效果，无需重启应用。

---

## 2. 端到端流程

1. 程序员/TA 提供 **HLSL** 或 **GLSL** 源码文件（或内存块），通过 Shader 模块的 **loadSource(path, format)** 或按扩展名自动识别格式加载。
2. 编译时指定目标后端（SPIR-V/DXIL/MSL），得到 **Bytecode**，提交给 RHI 创建 PSO/ShaderModule。
3. Shader 内通过 **预定义宏** 或 **变体关键字** 切换代码路径（如 `#ifdef USE_NORMAL_MAP`、`#if QUALITY_LEVEL >= 2`）；程序员通过 **setMacros(macros)** 或 **selectVariant(variantKey)** 在**运行时**切换宏组合，下一帧或下一次绑定 PSO 时生效。
4. 当源码文件或宏配置在磁盘/编辑器中发生变更时，通过 **reloadShader(handle)** 或 **onSourceChanged** 回调触发重新编译；编译完成后 **notifyShaderUpdated(handle)** 通知 Material/Pipeline 等下游，下游可刷新 PSO 或重新绑定，实现**实时更新**，无需重启应用。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 010-Shader | 支持 HLSL/GLSL 加载与编译、宏/变体定义与运行时动态切换、热重载与实时更新、缓存与错误报告 |
| 008-RHI | 接收 Shader 字节码创建 PSO/ShaderModule；热重载后 PSO 可由上层重建或替换 |
| 011-Material | 引用 Shader 与变体键；收到 Shader 更新通知后刷新绑定或变体 |
| 020-Pipeline / 021-Effects | 使用 Shader 字节码提交绘制；可选监听 Shader 更新并刷新 PSO |

---

## 4. 每模块职责与 I/O

### 010-Shader

- **职责**：提供 **ShaderSourceFormat**（HLSL、GLSL）；**loadSource(path, format)** 按格式加载源码；**compile** 产出多后端字节码；**setMacros** / **selectVariant** 实现**游戏中动态切换宏**；**reloadShader**、**onSourceChanged**、**notifyShaderUpdated** 实现 **Shader 热重载/实时更新**；DefineKeyword、EnumerateVariants、Precompile 与 Cache 同现有契约。
- **输入**：源码路径或内存、格式（或扩展名推断）、宏集/变体键、热重载触发（文件变更或显式调用）。
- **输出**：ShaderSourceFormat、loadSource、compile、getBytecode、setMacros、getVariantKey、selectVariant、reloadShader、onSourceChanged、notifyShaderUpdated；错误报告与缓存失效。

### 008-RHI

- **职责**：接收 Shader 模块产出的 Bytecode 创建 PSO/ShaderModule；热重载后由上层（Material/Pipeline）决定是否重建 PSO；本故事中不新增 ABI，仅作为字节码消费者。
- **输入**：Bytecode（来自 010-Shader）。
- **输出**：无新增；现有 PSO 创建接口不变。

### 011-Material

- **职责**：持有 Shader 引用与变体键；可选订阅 Shader 更新通知，在 **notifyShaderUpdated** 后刷新材质绑定的 PSO 或变体键；本故事中可仅约定“收到通知后刷新”的语义，具体 ABI 可由 Material 契约补充。
- **输入**：ShaderHandle、VariantKey；可选 ShaderUpdated 回调。
- **输出**：与 010-Shader 的 notifyShaderUpdated 对接；具体接口见 011-Material 契约。

---

## 5. 派生 ABI（010-Shader）

以下条目写入 **specs/_contracts/010-shader-ABI.md**（若已存在则合并，避免重复）。

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| 010-Shader | TenEngine::shader | — | 枚举 | 源码格式 | TenEngine/shader/ShaderTypes.h | ShaderSourceFormat | enum class ShaderSourceFormat { HLSL, GLSL }; 支持 HLSL/GLSL 加载与编译 |
| 010-Shader | TenEngine::shader | IShaderCompiler | 接口 | 加载与编译 | TenEngine/shader/ShaderCompiler.h | loadSource(path, format)、compile(handle, options)、getBytecode(handle) | 按 HLSL/GLSL 加载源码并编译为后端字节码 |
| 010-Shader | TenEngine::shader | — | 类型 | 宏/变体键 | TenEngine/shader/ShaderTypes.h | MacroSet、VariantKey | 宏名-值集合与变体键；用于宏切换代码路径 |
| 010-Shader | TenEngine::shader | IShaderHandle | 接口 | 动态宏 | TenEngine/shader/ShaderHandle.h | setMacros(macros)、getVariantKey()、selectVariant(key) | 游戏中动态切换宏并生效 |
| 010-Shader | TenEngine::shader | IShaderHotReload（可选） | 接口 | 实时更新 | TenEngine/shader/ShaderHotReload.h | reloadShader(handle)、onSourceChanged(path, callback)、notifyShaderUpdated(handle) | Shader 热重载；源码/宏变更后重新编译并通知下游，运行中实时生效 |

---

## 6. 验收要点

- 可从 **HLSL**、**GLSL** 文件（或内存）加载并成功编译为当前后端字节码。
- Shader 内可通过宏/变体切换代码路径；运行中调用 **setMacros** 或 **selectVariant** 后，下一帧或下次绑定使用新变体。
- 修改源码或触发 **reloadShader** 后，Shader 重新编译并通知下游；画面中使用的该 Shader 在无需重启应用的情况下**实时更新**。
