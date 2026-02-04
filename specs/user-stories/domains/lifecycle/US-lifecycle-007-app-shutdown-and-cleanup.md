# US-lifecycle-007：应用退出与清理（逆序关闭子系统、释放资源、日志冲刷）

- **标题**：用户请求退出或主循环结束后，应用**按逆序关闭子系统**、**释放资源**、**冲刷日志**；与 init 顺序对称，避免悬空引用与泄漏。
- **编号**：US-lifecycle-007

---

## 1. 角色/触发

- **角色**：引擎/游戏进程
- **触发**：用户关闭窗口或调用 **requestQuit()**；主循环退出后需**有序关闭**：Subsystems 逆序 shutdown、Application 销毁窗口、Core 最终清理与日志冲刷。

---

## 2. 端到端流程

1. **Application** 退出主循环后，通知 **Subsystems** 开始关闭；**Subsystems** 按**逆序**（与初始化顺序相反）调用各子系统的 **shutdown()**。
2. 各子系统在 shutdown 内释放其持有的资源（RHI 设备、Resource 句柄、场景、输入等）；不在此后使用其他子系统。
3. **Application** 销毁窗口、释放窗口相关资源；**Core** 做最终清理（若有全局状态、日志冲刷、泄漏报告等）。
4. **shutdown()** 后进程可安全退出；重复调用 shutdown 或 init 的语义由实现约定（幂等或禁止）。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 007-Subsystems | shutdownAll() 逆序调用各子系统 shutdown；与 initAll 顺序对称 |
| 003-Application | 销毁窗口、释放窗口资源；在 Subsystems 之后 |
| 001-Core | shutdown() 最终清理、日志冲刷、可选泄漏报告 |

---

## 4. 每模块职责与 I/O

### 007-Subsystems

- **职责**：提供 **shutdownAll()**，按**逆序**调用已注册子系统的 **shutdown()**；保证依赖关系（后初始化的先关闭）。
- **输入**：无（由 Application 或主循环退出触发）。
- **输出**：各子系统已关闭；此后不再调用子系统 API。

### 003-Application

- **职责**：主循环退出后**销毁窗口**、释放窗口与消息泵资源；在 Subsystems shutdownAll 之后调用。
- **输入**：主循环已退出。
- **输出**：窗口与事件资源已释放。

### 001-Core

- **职责**：提供 **shutdown()**；在 Application 之后调用；做**最终清理**（全局分配器统计、**日志冲刷**、可选泄漏报告）；init 之后仅调用一次。
- **输入**：无。
- **输出**：进程可安全退出。

---

## 5. 派生 ABI（与契约对齐）

- **007-subsystems-ABI**：shutdownAll、各子系统 shutdown 逆序约定。
- **003-application-ABI**：窗口销毁、与主循环退出协同。
- **001-core-ABI**：shutdown、日志冲刷。详见各模块 ABI。

---

## 6. 验收要点

- 退出时 Subsystems 逆序关闭、Application 销毁窗口、Core 做最终清理与日志冲刷；无悬空引用与泄漏。
