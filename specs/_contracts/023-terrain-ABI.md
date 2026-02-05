# 023-Terrain 模块 ABI

- **契约**：[023-terrain-public-api.md](./023-terrain-public-api.md)（能力与类型描述）
- **本文件**：023-Terrain 对外 ABI 显式表。
- **参考**：Unity Terrain、UE Landscape；高度图、分层、LOD、块、流式、绘制/刷。
- **命名**：成员方法采用 **PascalCase**；说明列给出**完整函数签名**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 地形数据（TerrainData）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 023-Terrain | te::terrain | ITerrain | 抽象接口 | 高度图 | te/terrain/Terrain.h | ITerrain::GetHeightMap | `ITextureResource* GetHeightMap() const;` 或 `float const* GetHeightMapData(uint32_t* width, uint32_t* height) const;` 与 Resource 流式对接 |
| 023-Terrain | te::terrain | ITerrain | 抽象接口 | 分层 | te/terrain/Terrain.h | ITerrain::GetLayerCount, GetLayer | `uint32_t GetLayerCount() const;` `ITerrainLayer* GetLayer(uint32_t index) const;` 纹理/混合 |
| 023-Terrain | te::terrain | ITerrain | 抽象接口 | 细节图 | te/terrain/Terrain.h | ITerrain::GetDetailMap | `ITextureResource* GetDetailMap(uint32_t layerIndex) const;` 可选 |
| 023-Terrain | te::terrain | ITerrain | 抽象接口 | 流式请求 | te/terrain/Terrain.h | ITerrain::RequestStreaming | `void RequestStreaming(ResourceId blockId, int priority);` 与 013-Resource 流式对接 |
| 023-Terrain | te::terrain | — | 自由函数/工厂 | 创建地形 | te/terrain/Terrain.h | CreateTerrain | `ITerrain* CreateTerrain(TerrainDesc const& desc);` 失败返回 nullptr |

### 地形块与 LOD

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 023-Terrain | te::terrain | ITerrainPatch | 抽象接口 | 块 LOD 级别 | te/terrain/TerrainPatch.h | ITerrainPatch::GetLODLevel, SetLODLevel | `uint32_t GetLODLevel() const;` `void SetLODLevel(uint32_t lod);` 与 Mesh 格式对接 |
| 023-Terrain | te::terrain | ITerrain | 抽象接口 | 获取块 | te/terrain/Terrain.h | ITerrain::GetPatch | `ITerrainPatch* GetPatch(uint32_t x, uint32_t z) const;` 块选择、流式请求 |
| 023-Terrain | te::terrain | ITerrain | 抽象接口 | 选择 LOD | te/terrain/Terrain.h | ITerrain::SelectLOD | `void SelectLOD(Vector3 const& viewPosition);` 按视点选择块 LOD |
| 023-Terrain | te::terrain | ITerrain | 抽象接口 | 流式块 | te/terrain/Terrain.h | ITerrain::StreamBlock | `void StreamBlock(uint32_t x, uint32_t z, int priority);` 与 Resource 对接 |
| 023-Terrain | te::terrain | — | 类型 | LOD 级别 | te/terrain/TerrainTypes.h | LODLevel | 与块或地形绑定 |

### 网格生成（MeshGen）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 023-Terrain | te::terrain | — | 自由函数/接口 | 生成块网格 | te/terrain/MeshGen.h | GeneratePatch | `IMesh* GeneratePatch(ITerrainPatch* patch, VertexFormat format);` 与 012-Mesh、009-RenderCore 顶点格式对接 |
| 023-Terrain | te::terrain | — | 枚举/struct | 顶点格式 | te/terrain/MeshGen.h | VertexFormat | 与 Mesh/RenderCore 一致 |

### 绘制/刷（可选）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 023-Terrain | te::terrain | IPaintBrush | 抽象接口（可选） | 高度刷 | te/terrain/PaintBrush.h | IPaintBrush::PaintHeight | `void PaintHeight(ITerrain* terrain, Vector3 const& pos, float radius, float delta);` 与 Editor 对接 |
| 023-Terrain | te::terrain | IPaintBrush | 抽象接口（可选） | 纹理刷 | te/terrain/PaintBrush.h | IPaintBrush::PaintTexture | `void PaintTexture(ITerrain* terrain, uint32_t layerIndex, Vector3 const& pos, float radius, float strength);` |
| 023-Terrain | te::terrain | — | struct | 刷参数 | te/terrain/PaintBrush.h | PaintBrushParams | 半径、强度、形状；单次编辑会话 |

*来源：契约能力 TerrainData、LOD、MeshGen、Painting；参考 Unity Terrain、UE Landscape。*
