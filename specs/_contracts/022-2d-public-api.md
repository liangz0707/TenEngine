# 契约：022-2D 模块对外 API

## 适用模块

- **实现方**：022-2D（L3；精灵、Tilemap、2D 物理、2D 渲染）
- **对应规格**：`docs/module-specs/022-2d.md`
- **依赖**：001-Core、013-Resource、014-Physics、020-Pipeline、009-RenderCore、028-Texture

## 消费者

- 024-Editor

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SpriteHandle | 精灵/图集句柄；LoadSprite、Atlas、Slice、SortOrder、SubmitToPipeline | 创建后直至显式释放 |
| TilemapHandle | 瓦片地图句柄；TileSet、Tilemap、Layer、CollisionLayer、Draw | 创建后直至显式释放 |
| Physics2DHandle | 2D 碰撞体/刚体桥接；Create2DBody、Create2DShape、与 Physics 对接 | 创建后直至显式释放 |
| 2DCamera / SortingLayer | 2D 相机、排序层；与 Pipeline 2D Pass、RenderCore 格式对接 | 由调用方或场景管理 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 精灵 | SpriteHandle、LoadSprite、Atlas、Slice、SortOrder、SubmitToPipeline |
| 2 | Tilemap | TilemapHandle、TileSet、Tilemap、Layer、CollisionLayer、Draw |
| 3 | 2D 物理 | Physics2DHandle、Create2DBody、Create2DShape；与 014-Physics 桥接 |
| 4 | 2D 渲染 | 2DCamera、SortingLayer；与 Pipeline 2D Pass、RenderCore 格式对接 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Resource、Physics、Pipeline、RenderCore、Texture 初始化之后使用。与 Resource 精灵/图集/瓦片集、Physics 2D、Pipeline 2D Pass 对接须明确。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 022-2D 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
