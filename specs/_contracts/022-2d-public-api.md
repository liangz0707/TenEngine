# Contract: 022-2D Module Public API

## Status: **TO BE IMPLEMENTED**

## Applicable Module

- **Implementer**: 022-2D (L3; sprites, tilemap, 2D physics, 2D rendering)
- **Specification**: `docs/module-specs/022-2d.md`
- **Dependencies**: 001-Core, 013-Resource, 014-Physics, 020-Pipeline, 009-RenderCore, 028-Texture

## Consumers

- 024-Editor

## Capability List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| SpriteHandle | Sprite/atlas handle; LoadSprite, Atlas, Slice, SortOrder, SubmitToPipeline | Created until explicitly released |
| TilemapHandle | Tilemap handle; TileSet, Tilemap, Layer, CollisionLayer, Draw | Created until explicitly released |
| Physics2DHandle | 2D collider/rigidbody bridge; Create2DBody, Create2DShape, Physics integration | Created until explicitly released |
| 2DCamera / SortingLayer | 2D camera, sorting layer; Pipeline 2D Pass, RenderCore format integration | Managed by caller or scene |

### Capabilities (Provider Guarantees)

| ID | Capability | Description |
|----|------------|-------------|
| 1 | Sprites | SpriteHandle, LoadSprite, Atlas, Slice, SortOrder, SubmitToPipeline |
| 2 | Tilemap | TilemapHandle, TileSet, Tilemap, Layer, CollisionLayer, Draw |
| 3 | 2D Physics | Physics2DHandle, Create2DBody, Create2DShape; 014-Physics bridge |
| 4 | 2D Rendering | 2DCamera, SortingLayer; Pipeline 2D Pass, RenderCore format integration |

## Version / ABI

- Follows Constitution: Public API versioned; breaking changes increment MAJOR.
- **Current Status**: Implementation pending. No public headers available.

## Constraints

- Must be used after Core, Resource, Physics, Pipeline, RenderCore, and Texture initialization.
- Must clearly define integration with Resource sprites/atlas/tilesets, Physics 2D, and Pipeline 2D Pass.

## Implementation Notes

The module directory `Engine/TenEngine-022-2d/include/` currently contains no header files.
The following interfaces are planned but not yet implemented:

- `te/2d/Sprite.h` - ISprite interface
- `te/2d/Tilemap.h` - ITilemap, ITilemapLayer interfaces
- `te/2d/Physics2D.h` - 2D physics bridge functions
- `te/2d/Camera2D.h` - ICamera2D interface
- `te/2d/SortingLayer.h` - SortingLayer type

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 022-2D contract |
| 2026-02-05 | Unified directory; capability list in table format |
| 2026-02-22 | Updated to reflect actual implementation status (to be implemented) |
