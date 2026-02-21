# 023-Terrain Module ABI

## Status: **TO BE IMPLEMENTED**

- **Contract**: [023-terrain-public-api.md](./023-terrain-public-api.md) (Capabilities and types description)
- **This Document**: 023-Terrain external ABI explicit table.
- **Reference**: Unity Terrain, UE Landscape; heightmaps, layers, LOD, patches, streaming, painting/brushes.
- **Naming**: Member methods use **PascalCase**; Description column provides **complete function signatures**.

## Implementation Status

The module directory `Engine/TenEngine-023-terrain/include/` currently contains no header files.
All interfaces listed below are planned but not yet implemented.

## ABI Table (Planned)

Column Definition: **Module | Namespace | Class | Export | Interface | Header | Symbol | Description**

### Terrain Data (Planned)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 023-Terrain | te::terrain | ITerrain | Abstract Interface | Heightmap | te/terrain/Terrain.h | ITerrain::GetHeightMap | `ITextureResource* GetHeightMap() const;` or `float const* GetHeightMapData(uint32_t* width, uint32_t* height) const;` Resource streaming integration |
| 023-Terrain | te::terrain | ITerrain | Abstract Interface | Layers | te/terrain/Terrain.h | ITerrain::GetLayerCount, GetLayer | `uint32_t GetLayerCount() const;` `ITerrainLayer* GetLayer(uint32_t index) const;` Texture/blend |
| 023-Terrain | te::terrain | ITerrain | Abstract Interface | Detail Map | te/terrain/Terrain.h | ITerrain::GetDetailMap | `ITextureResource* GetDetailMap(uint32_t layerIndex) const;` Optional |
| 023-Terrain | te::terrain | ITerrain | Abstract Interface | Streaming Request | te/terrain/Terrain.h | ITerrain::RequestStreaming | `void RequestStreaming(ResourceId blockId, int priority);` 013-Resource streaming integration |
| 023-Terrain | te::terrain | — | Free Function/Factory | Create Terrain | te/terrain/Terrain.h | CreateTerrain | `ITerrain* CreateTerrain(TerrainDesc const& desc);` Returns nullptr on failure |

### Terrain Patches and LOD (Planned)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 023-Terrain | te::terrain | ITerrainPatch | Abstract Interface | Patch LOD Level | te/terrain/TerrainPatch.h | ITerrainPatch::GetLODLevel, SetLODLevel | `uint32_t GetLODLevel() const;` `void SetLODLevel(uint32_t lod);` Mesh format integration |
| 023-Terrain | te::terrain | ITerrain | Abstract Interface | Get Patch | te/terrain/Terrain.h | ITerrain::GetPatch | `ITerrainPatch* GetPatch(uint32_t x, uint32_t z) const;` Patch selection, streaming request |
| 023-Terrain | te::terrain | ITerrain | Abstract Interface | Select LOD | te/terrain/Terrain.h | ITerrain::SelectLOD | `void SelectLOD(Vector3 const& viewPosition);` Select patch LOD by viewpoint |
| 023-Terrain | te::terrain | ITerrain | Abstract Interface | Stream Block | te/terrain/Terrain.h | ITerrain::StreamBlock | `void StreamBlock(uint32_t x, uint32_t z, int priority);` Resource integration |
| 023-Terrain | te::terrain | — | Type | LOD Level | te/terrain/TerrainTypes.h | LODLevel | Bound to patch or terrain |

### Mesh Generation (Planned)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 023-Terrain | te::terrain | — | Free Function/Interface | Generate Patch Mesh | te/terrain/MeshGen.h | GeneratePatch | `IMesh* GeneratePatch(ITerrainPatch* patch, VertexFormat format);` 012-Mesh, 009-RenderCore vertex format integration |
| 023-Terrain | te::terrain | — | Enum/Struct | Vertex Format | te/terrain/MeshGen.h | VertexFormat | Consistent with Mesh/RenderCore |

### Painting/Brushes (Optional, Planned)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 023-Terrain | te::terrain | IPaintBrush | Abstract Interface (Optional) | Height Brush | te/terrain/PaintBrush.h | IPaintBrush::PaintHeight | `void PaintHeight(ITerrain* terrain, Vector3 const& pos, float radius, float delta);` Editor integration |
| 023-Terrain | te::terrain | IPaintBrush | Abstract Interface (Optional) | Texture Brush | te/terrain/PaintBrush.h | IPaintBrush::PaintTexture | `void PaintTexture(ITerrain* terrain, uint32_t layerIndex, Vector3 const& pos, float radius, float strength);` |
| 023-Terrain | te::terrain | — | Struct | Brush Params | te/terrain/PaintBrush.h | PaintBrushParams | Radius, strength, shape; single editing session |

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 023-Terrain ABI |
| 2026-02-05 | Unified directory format |
| 2026-02-22 | Updated to reflect actual implementation status (to be implemented) |

*Source: Contract capabilities TerrainData, LOD, MeshGen, Painting; Reference: Unity Terrain, UE Landscape.*
