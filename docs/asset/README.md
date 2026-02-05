# 资源（Asset）设计文档

本目录为引擎**资源依赖、存储、加载、数据格式**的完整设计，依据 `specs/_contracts/000-module-dependency-map.md` 中的资源引用及资源相关模块（013-Resource、028-Texture、029-World、010-Shader、011-Material、012-Mesh、004-Scene、016-Audio、020-Pipeline 等）。

---

## 文档列表

| 文档 | 内容 |
|------|------|
| [01-asset-data-structure.md](./01-asset-data-structure.md) | **数据结构与数据表述**：GUID 管理、IResource 体系、FResource/RResource/DResource、磁盘一目录一资源（描述 / 实际数据 / 导入前源数据）、内存结构、数据格式与 *Desc 归属、与依赖图对应关系。 |
| [02-asset-loading-flow.md](./02-asset-loading-flow.md) | **加载流程与引用关系**：**GUID 与文件路径的对应**（§2.2）、013 为唯一加载入口、同步/异步加载、依赖解析与层级引用、状态检查与回调、EnsureDeviceResources、卸载与流式、调用顺序与约束。所有“如何加载”的约定以本文档为准。 |
| [03-asset-misc.md](./03-asset-misc.md) | **其他说明**：统一 IResource 接口（加载、卸载、导入）、状态检查与回调、层级引用、内部统一接口创建 GPU 资源（DResource）、与 000-module-dependency-map 对应关系、契约与规格索引、其他约定。 |

---

## 设计原则（五项）

1. **所有资源依 GUID 进行管理。**
2. **所有资源继承自 IResource**，需要统一的加载、卸载、导入接口。
3. **磁盘资源结构**：一个目录存储一个资源——资源描述（.material / .mesh / .texture / .model / .level 等）、实际数据（如需要：.texdata / .meshdata 等）、导入前源数据（.png / .obj 等）。
4. **内存结构**：所有资源通过 IResource 接口统一加载；支持异步、同步加载、状态检查、回调、层级引用。
5. **内部有统一接口创建对应的 GPU 资源**（EnsureDeviceResources；由 028-Texture、011-Material、012-Mesh、008-RHI 创建 DResource，013 不参与）。

---

## 相关契约与规格

- **模块依赖图**：`specs/_contracts/000-module-dependency-map.md`
- **013-Resource**：`specs/_contracts/013-resource-public-api.md`、`docs/module-specs/013-resource.md`
- **028-Texture**：`specs/_contracts/028-texture-public-api.md`
- **029-World**：`specs/_contracts/029-world-public-api.md`
