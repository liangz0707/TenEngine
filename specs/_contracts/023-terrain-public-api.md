# Contract: 023-Terrain Module Public API

## Status: **TO BE IMPLEMENTED**

## Applicable Module

- **Implementer**: 023-Terrain (L3; terrain data, LOD, painting/brushes)
- **Specification**: `docs/module-specs/023-terrain.md`
- **Dependencies**: 001-Core, 013-Resource, 012-Mesh, 020-Pipeline, 009-RenderCore, 028-Texture

## Consumers

- 024-Editor

## Capability List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| TerrainHandle | Terrain handle; heightmap, layers, details, Resource streaming integration | Created until explicitly released |
| TerrainPatchHandle | Terrain patch handle; LOD level, patch selection, streaming request, Mesh format integration | Bound to Terrain |
| LODLevel | Terrain LOD level, SelectLOD, StreamBlock; Resource integration | Bound to patch or terrain |
| PaintBrush | Height/texture brush (optional); PaintHeight, PaintTexture, Brush, Editor integration | Single editing session |

### Capabilities (Provider Guarantees)

| ID | Capability | Description |
|----|------------|-------------|
| 1 | Terrain Data | TerrainHandle, heightmap, layers, details, Resource streaming integration |
| 2 | Patches and LOD | TerrainPatchHandle, LODLevel, SelectLOD, StreamBlock, StreamingRequest |
| 3 | Rendering | Pipeline terrain Pass, Mesh vertex format, RenderCore format integration |
| 4 | Brushes (Optional) | PaintBrush, PaintHeight, PaintTexture, Editor integration |

## Version / ABI

- Follows Constitution: Public API versioned; breaking changes increment MAJOR.
- **Current Status**: Implementation pending. No public headers available.

## Constraints

- Must be used after Core, Resource, Mesh, Pipeline, RenderCore, and Texture initialization.
- Must clearly define integration with Resource terrain blocks/streaming, Mesh vertex format, and Pipeline terrain Pass.

## Implementation Notes

The module directory `Engine/TenEngine-023-terrain/include/` currently contains no header files.
The following interfaces are planned but not yet implemented:

- `te/terrain/Terrain.h` - ITerrain interface
- `te/terrain/TerrainPatch.h` - ITerrainPatch interface
- `te/terrain/TerrainTypes.h` - Terrain type definitions
- `te/terrain/MeshGen.h` - Mesh generation functions
- `te/terrain/PaintBrush.h` - IPaintBrush interface (optional)

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 023-Terrain contract |
| 2026-02-05 | Unified directory; capability list in table format |
| 2026-02-22 | Updated to reflect actual implementation status (to be implemented) |
