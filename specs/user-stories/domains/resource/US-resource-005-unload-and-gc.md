# US-resource-005：资源卸载与引用计数/GC（显式卸载、依赖解除、GC 策略）

- **标题**：应用可**显式卸载**某资源或依赖该资源的对象；Resource 模块支持**引用计数**或 **GC 策略**（按引用解除、按策略回收）；卸载顺序与各模块句柄释放协同，避免悬空引用。
- **编号**：US-resource-005

---

## 1. 角色/触发

- **角色**：游戏逻辑、场景/关卡切换、Editor
- **触发**：需要**释放**某资源（如关卡切换时卸载旧关卡资源）；或依赖该资源的句柄已全部释放，希望**按策略回收**（GC）；卸载顺序须与 Pipeline/Entity/Scene 等句柄释放协同。

---

## 2. 端到端流程

1. 调用方**释放**对某资源的引用（如 RResource 句柄、或场景卸载导致对资源引用解除）；Resource 模块**引用计数**减一；当计数为 0 时，可**回收**该资源（或加入待回收队列）。
2. 调用方也可**显式卸载**：**unloadResource(resourceId)** 或 **release(handle)**；Resource 解除该资源与依赖关系、释放 FResource/RResource/DResource 相关内存与 GPU 资源；若仍有引用则按实现约定（忽略或延迟至 0）。
3. **GC 策略**（可选）：定时或按帧扫描无引用资源并回收；与各模块约定「句柄释放后不再使用」。
4. 各模块（Pipeline、Entity、Material 等）在释放自身对象时**先释放对 Resource 的引用**，再销毁对象；Resource 卸载顺序与 RHI/GPU 资源释放顺序协同（见 pipeline-to-rci 与各契约）。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 013-Resource | unloadResource、release、引用计数、GC 策略、与各模块句柄释放协同 |
| 020-Pipeline / 005-Entity / 011-Material 等 | 释放资源句柄顺序、与 Resource 卸载协同 |

---

## 4. 每模块职责与 I/O

### 013-Resource

- **职责**：提供 **unloadResource(resourceId)** 或 **release(handle)**；**引用计数**（增加/减少引用、计数为 0 时回收）；可选 **GC**（按策略扫描无引用资源并回收）；与各模块约定释放顺序，避免悬空引用。
- **输入**：resourceId 或 handle、释放请求；各模块句柄释放事件（若 GC 依赖）。
- **输出**：资源已卸载或加入待回收；下游不再使用该资源句柄。

---

## 5. 派生 ABI（与契约对齐）

- **013-resource-ABI**：unloadResource、release、引用计数、GC 策略。详见 `specs/_contracts/013-resource-ABI.md`。

---

## 6. 验收要点

- 可显式卸载资源或通过释放引用触发回收；引用计数为 0 时资源可回收；卸载顺序与各模块句柄释放协同，无悬空引用。
