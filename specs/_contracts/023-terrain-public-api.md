# 契约：023-Terrain 模块对外 API

## 适用模块

- **实现方**：**023-Terrain**（地形数据、LOD 与绘制/刷）
- **对应规格**：`docs/module-specs/023-terrain.md`
- **依赖**：001-Core、013-Resource、012-Mesh、020-Pipeline、009-RenderCore（见 `000-module-dependency-map.md`）

## 消费者（T0 下游）

- 024-Editor（地形编辑与刷、高度/纹理绘制、Brush）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| TerrainHandle | 地形句柄；高度图、分层、细节、与 Resource 流式对接 | 创建后直至显式释放 |
| TerrainPatchHandle | 地形块句柄；LOD 级别、块选择、流式请求、与 Mesh 格式对接 | 与 Terrain 绑定 |
| LODLevel | 地形 LOD 级别、SelectLOD、StreamBlock；与 Resource 对接 | 与块或地形绑定 |
| PaintBrush | 高度/纹理刷（可选）；PaintHeight、PaintTexture、Brush、与 Editor 对接 | 单次编辑会话 |

下游仅通过上述类型与句柄访问；与 Resource 地形块/流式、Mesh 顶点格式、Pipeline 地形 Pass、RenderCore 格式对接。

## 能力列表（提供方保证）

1. **TerrainData**：HeightMap、Layers、DetailMap、StreamingRequest；与 Resource 流式对接。
2. **LOD**：LODLevel、SelectLOD、StreamBlock；与 Resource 对接。
3. **MeshGen**：GeneratePatch、VertexFormat；与 Mesh/RenderCore 对接。
4. **Painting（可选）**：PaintHeight、PaintTexture、Brush；与 Editor 对接。

## 调用顺序与约束

- 须在 Core、Resource、Mesh、Pipeline、RenderCore 初始化之后使用；地形 Pass 与 Pipeline、顶点格式与 Mesh/RenderCore 一致。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：023-Terrain 对应本契约；与 docs/module-specs/023-terrain.md 一致 |
