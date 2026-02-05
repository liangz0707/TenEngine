# US-tools-001：资源导入与批处理（导入管线、批处理、CLI）

- **标题**：工具链可**导入**资源（纹理、网格、音频等）、进行**格式转换**与**元数据/依赖记录**、支持**批处理**与 **CLI**；与 Resource 模块的导入器、元数据、可寻址约定对接。
- **编号**：US-tools-001

---

## 1. 角色/触发

- **角色**：构建流水线、美术、程序员
- **触发**：需要将**原始资源**（如 PNG、FBX、WAV）**导入**为引擎格式、生成**元数据**与**依赖记录**；支持**批量导入**与 **CLI** 调用（CI/批处理）；与 Resource 加载时使用的格式与 GUID 一致。

---

## 2. 端到端流程

1. 调用方（或 CLI）调用 **importAsset(path)** 或 **importBatch(paths)**；Tools 模块根据**扩展名或格式检测**选择**导入器**（与 Resource 的 RegisterImporter 约定一致）。
2. 导入器执行**格式转换**（如压缩纹理、烘焙网格 LOD）、产出**引擎格式**文件与**元数据**（GUID、依赖列表、与 FResource 约定一致）；写入输出目录或 Bundle。
3. **批处理**时遍历目录或列表，对每个资源调用导入；**CLI** 参数可指定输入/输出、平台、选项；失败时返回错误码与日志。
4. Resource 运行时加载时使用上述产出（FResource、GUID、依赖）；Tools 与 Resource 约定**格式与元数据**一致。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 025-Tools | importAsset、importBatch、CLI 入口、导入器注册、格式检测、输出目录/Bundle |
| 013-Resource | 导入器协议、元数据格式、GUID、依赖记录；与 Tools 产出一致 |
| 001-Core | 文件 I/O、平台抽象（Tools 内部使用） |

---

## 4. 每模块职责与 I/O

### 025-Tools

- **职责**：提供 **importAsset**、**importBatch**、**CLI main**；导入器注册与格式检测；格式转换、元数据与依赖记录；与 Resource 约定一致；可选打包为 Bundle。
- **输入**：资源路径、输出路径、平台/选项；CLI 参数。
- **输出**：引擎格式文件、元数据、依赖列表；供 Resource 运行时加载。

---

## 5. 派生 ABI（与契约对齐）

- **025-tools-ABI**：importAsset、importBatch、CLI、导入器协议。详见 `specs/_contracts/025-tools-ABI.md`。

---

## 6. 验收要点

- 可导入单资源或批量导入；产出格式与元数据与 Resource 约定一致；支持 CLI 批处理。
