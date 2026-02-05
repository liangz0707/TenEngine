# 契约：023-Terrain 模块对外 API

## 适用模块

- **实现方**：023-Terrain（L3；地形数据、LOD、绘制/刷）
- **对应规格**：`docs/module-specs/023-terrain.md`
- **依赖**：001-Core、013-Resource、012-Mesh、020-Pipeline、009-RenderCore、028-Texture

## 消费者

- 024-Editor

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| TerrainHandle | 地形句柄；高度图、分层、细节、与 Resource 流式对接 | 创建后直至显式释放 |
| TerrainPatchHandle | 地形块句柄；LOD 级别、块选择、流式请求、与 Mesh 格式对接 | 与 Terrain 绑定 |
| LODLevel | 地形 LOD 级别、SelectLOD、StreamBlock；与 Resource 对接 | 与块或地形绑定 |
| PaintBrush | 高度/纹理刷（可选）；PaintHeight、PaintTexture、Brush、与 Editor 对接 | 单次编辑会话 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 地形数据 | TerrainHandle、高度图、分层、细节、与 Resource 流式对接 |
| 2 | 块与 LOD | TerrainPatchHandle、LODLevel、SelectLOD、StreamBlock、StreamingRequest |
| 3 | 绘制 | 与 Pipeline 地形 Pass、Mesh 顶点格式、RenderCore 格式对接 |
| 4 | 刷（可选） | PaintBrush、PaintHeight、PaintTexture、与 Editor 对接 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Resource、Mesh、Pipeline、RenderCore、Texture 初始化之后使用。与 Resource 地形块/流式、Mesh 顶点格式、Pipeline 地形 Pass 对接须明确。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 023-Terrain 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
