# US-resource-004：资源三态（FResource / RResource / DResource）与引用方式

- **标题**：为便于管理，所有资源可有三种形态：**FResource**（硬盘）、**RResource**（运行时/内存）、**DResource**（GPU）。硬盘加载使用 FResource；内存引用使用 RResource；GPU 资源 DResource 直接保存在 RResource 内部。FResource 通过全局唯一 GUID 引用其他资源；RResource 根据 FResource 的引用通过指针引用其他资源。部分资源可能只存在某一形态。
- **编号**：US-resource-004

---

## 1. 角色/触发

- **角色**：引擎资源系统、渲染/音频等下游模块
- **触发**：需要统一资源管理方式：**硬盘加载使用 FResource**；**内存引用使用 RResource**；**GPU 类型资源使用 DResource**，且 DResource 直接保存在 RResource 内部。引用方式：FResource 在硬盘上，通过**全局唯一 GUID** 引用其他资源；RResource 根据 FResource 的引用，通过**指针**引用其他资源。部分资源可能只存在某一形态。

---

## 2. 端到端流程与约定

1. **FResource（硬盘形态）**：在**硬盘上**；**硬盘加载使用 FResource**。引用其他资源时，通过**全局唯一的 GUID** 引用（FResource 内不存指针，只存 GUID）。用于导入、打包、加载入口。
2. **RResource（运行时/内存形态）**：**内存引用使用 RResource**。根据 FResource 的引用关系，在内存中通过**指针**引用其他资源（RResource 持有对其它 RResource 的指针）。**DResource 直接保存在 RResource 内部**（GPU 资源由 RResource 持有，不单独作为引用对象）。加载完成后，下游拿到的是 RResource（或 IResource 对应的运行时表示）。
3. **DResource（GPU 形态）**：**GPU 类型资源**；不单独作为跨对象引用，而是**保存在 RResource 内部**，由 RResource 管理生命周期与绑定。渲染/管线通过 RResource 访问其内部的 DResource。
4. **单形态资源**：部分资源可能**只存在某一形态**（例如仅 FResource 的元数据、仅 RResource 的运行时生成数据、或仅作为 RResource 内 DResource 的 GPU 资源），按需定义即可。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 013-Resource | 定义 FResource（硬盘、GUID 引用）、RResource（内存、指针引用、内部持有 DResource）、DResource（GPU、保存在 RResource 内部）；加载从 FResource 得到 RResource；部分资源可能只存在某一形态 |
| 008-RHI / 020-Pipeline 等 | 消费 RResource 内部的 DResource（纹理/缓冲等 GPU 资源） |

---

## 4. 每模块职责与 I/O

### 013-Resource

- **职责**：提供 **FResource**（硬盘形态，通过 **GUID** 引用其他资源）、**RResource**（运行时形态，根据 FResource 引用通过**指针**引用其他 RResource，**DResource 直接保存在 RResource 内部**）、**DResource**（GPU 形态，保存在 RResource 内部）；硬盘加载使用 FResource；内存引用使用 RResource；部分资源可能只存在某一形态。
- **输入**：磁盘/包中的 FResource（GUID 引用）；加载请求。
- **输出**：FResource 表示、RResource 表示（含内部 DResource）；GUID 与指针引用约定。

---

## 5. 派生 ABI（与契约对齐）

以下与 **specs/_contracts/013-resource-ABI.md** 及 **013-resource-public-api.md** 一致：FResource、RResource、DResource 类型/概念；FResource 通过 GUID 引用；RResource 通过指针引用并内部持有 DResource；部分资源可能只存在某一形态。

---

## 6. 验收要点

- 硬盘加载时使用 **FResource**；FResource 引用其他资源仅通过**全局唯一 GUID**。
- 内存引用时使用 **RResource**；RResource 根据 FResource 的引用通过**指针**引用其他 RResource；**DResource 直接保存在 RResource 内部**。
- GPU 资源 **DResource** 由 RResource 内部持有，不单独作为跨对象引用。
- 部分资源可只存在某一形态（仅 F、仅 R、或仅 R 内 D）。
