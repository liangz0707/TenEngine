# ABI/接口生成规范

本文档定义：**从用户故事与模块职责推导出的接口**，在写入全局 ABI 列表（`specs/_contracts/NNN-modulename-ABI.md`）时必须遵守的**代码标准、命名规范、注释规范**。所有「用户故事 → 模块拆解 → 接口/类型/回调」产出的条目，均按本规范生成，保证风格一致且可直接落入 ABI 表。

---

## 1. 代码标准：Google C++ 风格指南

**来源**：[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)（官方英文）。本项目 ABI/接口生成采用该标准；以下为与接口/头文件/ABI 相关的要点摘要。

| 项目 | 约定（Google 风格摘要） |
|------|--------------------------|
| **C++ 版本** | 以 C++14 或 C++17 为目标（指南允许项目选择）；TenEngine 建议 **C++17**。 |
| **头文件** | 自包含、#include guard；仅暴露必要类型与函数；实现细节不进入公开头文件。 |
| **可见性** | 仅契约声明的类型与函数放入公开头文件；内部实现不进入 ABI。 |
| **错误处理** | 优先通过返回值表示错误（如 `bool`、`Status`、`optional`）；不用异常跨模块边界。 |
| **命名** | 见 §2，与 Google 命名规范一致。 |
| **注释** | 见 §3，与 Google 注释规范一致。 |

*详细规则（禁止的语言特性、继承与虚函数、智能指针等）以官方文档为准；生成 ABI 条目时以本节及 §2、§3 为准即可。*

---

## 2. 命名规范（Google C++ 风格）

**来源**：与 §1 一致，出自 [Google C++ Style Guide - Naming](https://google.github.io/styleguide/cppguide.html#Naming)。生成的接口名须符合下表，以便直接填入 `specs/_contracts/000-module-ABI.md` 与各 ABI 表的「命名空间」「类名」「符号」列。

| 对象 | 约定（Google） | 示例 |
|------|----------------|------|
| **命名空间** | **引擎全名**为根，可多级（项目约定） | TenEngine::core, TenEngine::rhi |
| **类型名（class/struct）** | PascalCase（UpperCamelCase） | IDevice, LoadResult, UserProfile |
| **函数/方法** | **驼峰**（项目约定，覆盖 Google） | createDevice(), getQueue() |
| **枚举类型** | PascalCase | LogLevel, Backend |
| **枚举值** | PascalCase（与类型同风格） | LogLevel::Debug, Backend::Vulkan |
| **常量** | k 前缀 + PascalCase | kMaxCount, kDefaultAlignment |
| **成员变量** | **m 开头驼峰**（项目约定，覆盖 Google） | mDeviceHandle, mMemberName |
| **参数/局部变量** | snake_case | size, alignment |
| **文件命名** | **大写开头驼峰**（项目约定，覆盖 Google） | Alloc.h, Device.hpp, MyClass.cpp |
| **回调/事件** | 驼峰或 OnXxx / XxxCallback（与函数一致） | onLoadComplete, tickCallback |

*与 ABI 表对应*：符号列中自由函数写 `createDevice`；静态方法写 `TypeRegistry::registerType`；类型名写 `IDevice`、`LogLevel`；枚举值写 `LogLevel::Debug`。

**项目例外**：① 命名空间以引擎全名 **TenEngine** 为根（如 `TenEngine::core`、`TenEngine::rhi`）；② 函数/方法采用**驼峰**（如 `createDevice()`、`getQueue()`）；③ 成员变量采用 m 前缀驼峰（如 `mDeviceHandle`）；④ 文件命名采用大写开头驼峰（如 `Alloc.h`）。以上与 Google 原规范不同，以本项目约定为准。

---

## 3. 注释规范（Google C++ 风格）

**来源**：与 §1 一致，出自 [Google C++ Style Guide - Comments](https://google.github.io/styleguide/cppguide.html#Comments)。公开 API 及 ABI「说明」列应满足以下约定，便于下游正确使用。

| 对象 | 约定（Google 风格摘要） |
|------|--------------------------|
| **文件头** | 简要说明文件内容；可选版权与作者。 |
| **类/类型** | 说明类型用途、生命周期、谁创建/谁释放；若为接口/抽象类，注明用法。 |
| **函数** | 说明做什么、参数含义、返回值（含失败语义）；线程安全若非常规则注明；前置/后置条件若重要则写明。 |
| **参数/返回值** | 在函数注释中写清含义与约束；复杂类型可单独一句。 |
| **回调/事件** | 谁在何时调用、参数含义、是否可重入、调用后对象是否仍有效。 |

*风格*：使用 `//` 行注释；可选用 Doxygen 风格（`///` 或 `/** */`）以便生成文档。生成的 ABI 条目「说明」列应为一句话或简短列表，涵盖上述要点中与下游使用直接相关的部分。

---

## 4. 从规范到 ABI 表条目（固定）

以下为**固定流程**：按 §1–§3 生成的接口，统一写成可放入 `specs/_contracts/000-module-ABI.md` 与各 `NNN-modulename-ABI.md` 的格式。

### 4.1 ABI 表列与规范对应

| ABI 列 | 填写内容 | 遵守规范 |
|--------|----------|----------|
| 模块名 | 模块 ID（001-Core … 027-XR） | 与依赖图一致 |
| 命名空间 | C++ 命名空间 | §2 命名规范 |
| 类名 | 类型名或 —（自由函数/全局枚举等） | §2 命名规范 |
| 导出形式 | 可选；自由函数/单例/接口/枚举/模板等 | 见 000-module-ABI 导出形式约定 |
| 接口说明 | 一句话职责 | §3 注释规范（简要说明） |
| 头文件 | 相对 include 路径 | 与模块目录约定一致 |
| 符号 | 下游使用的符号名 | §2 命名规范 |
| 说明 | 签名摘要、失败语义、线程安全、生命周期 | §3 注释规范 + §1 错误处理/可见性 |

### 4.2 一条「接口」的产出物

每条由用户故事推导出的接口，应产出：

1. **类型/枚举**（若新增）：类型名、所属头文件、成员或枚举值、生命周期或语义说明（按 §2–§3）。
2. **函数/方法签名**：命名空间、类名（若有）、函数名、参数类型与名、返回类型、失败/回调语义（按 §1–§3）。
3. **回调/事件**（若适用）：类型名或函数指针签名、谁在何时调用、参数含义（按 §3）。
4. **ABI 表一行或几行**：将上述填入 ABI 表列，使「符号」「说明」可直接粘贴到 `NNN-modulename-ABI.md`。

### 4.3 与用户故事的衔接

- 用户故事文档（见 `specs/user-stories/`）中「各模块所需类型、函数签名、回调/事件」**必须**按本规范书写。
- 定稿后的条目由人工或脚本同步到对应模块的 `specs/_contracts/NNN-modulename-ABI.md` 与契约（若涉及能力描述）。

---

## 5. 协作方式摘要

| 步骤 | 您提供 | 产出 |
|------|--------|------|
| **A** | 代码标准、命名规范、注释规范（或指向现有 doc） | 本文档 §1–§3 定稿，作为「ABI/接口生成规范」 |
| **B.1** | 用户故事（可交付的端到端行为） | 按故事拆解：涉及模块、每模块职责与输入/输出 |
| **B.2** | （可选）无 | 补充重要但缺失的用户故事，同格式写入 `specs/user-stories/` |
| **B.3** | — | 关键用户故事单独成文并纳入索引 |
| **B.4** | — | 每故事涉及的模块：类型、函数签名、回调/事件，**严格按 §1–§3** 写成可放入全局 ABI 的条目 |

---

*本文档与 `specs/_contracts/000-module-ABI.md`、`specs/user-stories/` 配套使用。§1–§3 已采用 **Google C++ 风格指南**；所有从用户故事生成的接口按本规范生成并写入 ABI。若某条需项目级覆盖，可在本节后增补「项目例外」说明。*
