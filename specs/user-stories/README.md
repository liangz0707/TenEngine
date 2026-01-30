# 用户故事（User Stories）

本目录存放 **TenEngine T0 架构** 下「可交付的端到端行为」——**用户故事**：从「用户/系统在做什么」反推涉及的模块、每模块职责与输入/输出，并派生出各模块需要的**类型、函数签名、回调/事件**，严格按 **`docs/engine-abi-interface-generation-spec.md`** 的代码标准、命名规范、注释规范写成可放入全局 ABI 的条目。

**规模**：用户故事数量级可能达到**上万条**，本目录按**领域（domain）**分文件夹组织，每领域内按编号排序，便于检索与扩展。

**总览清单**：**尽可能完备**、覆盖**各层级**（L0 应用/引擎级、L1 子系统级、L2 功能级、L3 工作流级、L4 非功能）的用户故事清单见 **[001-user-stories-master-list.md](./001-user-stories-master-list.md)**；已实现故事见各领域 `domains/<domain>/index.md`，待建故事可在该清单中按优先级补充。

---

## 1. 组织结构（支持上万条）

### 1.1 领域分类（Domain）

| 领域目录 | 说明 | 典型涉及模块 | 参考（Unity / Unreal） |
|----------|------|----------------------|------------------------|
| **lifecycle** | 进程/应用生命周期、主循环、子系统启停 | 001-Core, 003-Application, 007-Subsystems | Unity Subsystems (Start/Stop)、UE Module LoadingPhase |
| **rendering** | 一帧渲染、多帧流水线、Pass、Present、RDG 风格 | 008-RHI, 019-PipelineCore, 020-Pipeline, 004-Scene, 005-Entity | UE RDG、Unity SRP (HDRP/URP) |
| **resource** | 加载/卸载/流式、异步、ResourceId、句柄 | 013-Resource, 001-Core, 002-Object | Unity Addressables、UE StreamableManager |
| **input** | 输入设备、事件、映射、轮询 | 006-Input, 003-Application | Unity Input System、UE FInputManager |
| **scene** | 场景图、节点、空间、加载/切换 | 004-Scene, 005-Entity, 013-Resource | Unity SceneManager、UE World / Level |
| **entity** | 实体/组件、层级、查询、序列化 | 005-Entity, 002-Object, 004-Scene | Unity GameObject/Component、UE Actor/Component |
| **editor** | 视口、场景树、属性面板、资源浏览器 | 024-Editor, 006-Input, 008-RHI, 020-Pipeline, 018-UI | Unity Editor、UE Slate/Editor |
| **audio** | 音源、监听、混音、3D 音效 | 016-Audio, 013-Resource | Unity AudioSource、UE USoundBase |
| **physics** | 碰撞、刚体、射线、查询 | 014-Physics, 004-Scene, 005-Entity | Unity PhysX、UE Chaos |
| **animation** | 剪辑、骨骼、混合、播放 | 015-Animation, 005-Entity, 012-Mesh | Unity Animator、UE USkeletalMeshComponent |
| **ui** | 画布、控件树、布局、事件 | 018-UI, 017-UICore, 006-Input | Unity UGUI、UE Slate/UMG |
| **networking** | 复制、RPC、连接、同步 | 026-Networking, 005-Entity | Unity Netcode、UE Replication |
| **xr** | XR 会话、帧、提交、输入 | 027-XR, 007-Subsystems, 006-Input, 020-Pipeline | Unity XR Subsystems、UE OpenXR |
| **tools** | 构建、批处理、CLI、资源导入 | 025-Tools, 013-Resource | Unity Build Pipeline、UE UBT |

新增领域时，在本表与 **`000-user-stories-index.md`** 的「领域列表」中补充一行，并在 `domains/` 下新建同名目录及 `README.md`、`index.md`。

### 1.2 目录与编号约定

```
specs/user-stories/
  README.md                    # 本文件：总览、领域、冲突规则、参考
  000-user-stories-index.md     # 顶层索引：领域导航、冲突记录、统计
  domains/
    <domain>/                   # 如 lifecycle, rendering, resource
      README.md                 # 本领域范围、涉及模块、参考接口（可选）
      index.md                  # 本领域故事列表：编号、标题、涉及模块、文档链接
      US-<domain>-<NNN>-<slug>.md   # 单条故事，NNN 为 3 位数字 001～999
```

- **编号**：`US-<domain>-<NNN>`，例如 `US-lifecycle-001`、`US-rendering-002`。同一领域内 NNN 唯一，建议按添加顺序递增；单领域可容纳 999 条，多领域合计可上万。
- **文档名**：`US-<domain>-<NNN>-<简短英文描述>.md`，便于排序与检索。
- **顶层索引**：不列举全部故事，只做**领域导航**、**冲突与裁决记录**、**与 ABI 的衔接说明**；各领域详情见 `domains/<domain>/index.md`。

### 1.3 单条故事格式（建议）

每条用户故事建议包含：

1. **标题**：一句话描述可交付的端到端行为。
2. **角色/触发**：谁（用户/系统）在什么场景下触发。
3. **端到端流程**：从触发到完成的步骤（可含分支、异步回调、用户操作）。
4. **涉及模块**：列出参与该行为的模块（001-Core … 027-XR）。
5. **每模块职责与 I/O**：职责、输入、输出。
6. **派生接口（ABI 条目）**：按 `docs/engine-abi-interface-generation-spec.md` 写出的类型、函数签名、回调/事件，可直接填入 `specs/_contracts/NNN-modulename-ABI.md`。
7. **参考（可选）**：对齐的 Unity / Unreal 接口或文档链接。

---

## 2. 冲突检测与处理

当**多条用户故事**对**同一模块**的**同一符号或流程**有**不同约定或互斥行为**时，视为**冲突**，需人工裁决并记录。

### 2.1 视为冲突的情况（需提示）

| 类型 | 说明 | 示例 |
|------|------|------|
| **同符号不同约定** | 同一模块、同一符号（类型/函数/枚举），签名或语义不一致 | 故事 A 要求 `init()` 无参，故事 B 要求 `init(InitParams const*)` |
| **同流程互斥** | 同一端到端流程，两条故事要求的顺序或分支不可同时满足 | 故事 A「先 Present 再 submit」，故事 B「先 submit 再 Present」 |
| **资源/生命周期互斥** | 对同一资源或对象的创建/销毁/所有权约定矛盾 | 故事 A「ResourceManager 拥有 IResource」，故事 B「调用方拥有 IResource」 |

### 2.2 不视为冲突（可共存）

- 同一模块**不同符号**（不同故事补充不同接口）。
- 同一符号**仅做扩展**（如增加可选参数、重载），且不改变既有语义。
- 不同领域的故事对**不同模块**的约定（由模块契约与 ABI 统一收敛）。

### 2.3 冲突处理流程

1. **检测**：在拆解故事、写入 ABI 时，若发现与已有 ABI 条目或已有故事对同一模块同一符号/流程约定不一致，**提示用户**：「用户故事 US-xxx 与 US-yyy 在模块 NNN 的符号 Z 上存在冲突：…」。
2. **裁决**：由人工决定采纳其一、合并（如重载）、或拆成两条符号（如 `init()` 与 `initWithParams()`）并更新故事与 ABI。
3. **记录**：在 **`000-user-stories-index.md`** 的「冲突与裁决」小节或 **`domains/_conflicts.md`** 中记录：故事编号、冲突描述、裁决结果、修订后的 ABI/故事。

---

## 3. 与 Unity / Unreal 的参考

- 接口设计时可参考：
  - **Unity**：[Scripting API](https://docs.unity3d.com/ScriptReference/)、[Subsystems](https://docs.unity3d.com/Manual/com.unity.modules.subsystems.html)、[SRP](https://docs.unity3d.com/Manual/render-pipelines.html)
  - **Unreal**：[API Reference](https://docs.unrealengine.com/en-US/API/)、[Modules](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules)、[RDG](https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine)
- 在单条故事的「参考」中可注明对齐的引擎与接口，便于后续对照与避免冲突。
- 引擎模块对照见 **`docs/research/engine-reference-unity-unreal-modules.md`**。

---

## 4. 协作方式（B）

| 步骤 | 您提供 | 产出 |
|------|--------|------|
| B.1 | 用户故事（可交付的端到端行为）及所属领域 | 按故事拆解：涉及模块、每模块职责与 I/O；若与已有故事/ABI 冲突则**提示** |
| B.2 | （可选）无 | 补充重要但缺失的用户故事，按领域写入 `domains/<domain>/` |
| B.3 | — | 关键用户故事单独成文，纳入对应 `domains/<domain>/index.md` |
| B.4 | — | 每故事涉及的模块：类型、函数签名、回调/事件，**严格按 ABI/接口生成规范**写成 ABI 条目；冲突时暂停并提示 |

---

## 5. 与 ABI/契约的衔接

- 从用户故事推导出的接口，**必须**符合 **`docs/engine-abi-interface-generation-spec.md`**（§1 代码标准、§2 命名规范、§3 注释规范）。
- 定稿后的条目同步到 **`specs/_contracts/NNN-modulename-ABI.md`** 及对应契约（若涉及能力描述）；ABI 总索引见 **`specs/_contracts/000-module-ABI.md`**。
- 同一模块的 ABI 若由多条故事共同贡献，以**无冲突合并**为准；发现冲突即按上文「冲突处理流程」执行并记录。
