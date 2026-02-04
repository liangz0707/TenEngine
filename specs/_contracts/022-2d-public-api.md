# 契约：022-2D 模块对外 API

## 适用模块

- **实现方**：**022-2D**（精灵、Tilemap 与 2D 渲染）
- **对应规格**：`docs/module-specs/022-2d.md`
- **依赖**：001-Core、013-Resource、014-Physics、020-Pipeline、009-RenderCore（见 `000-module-dependency-map.md`）

## 消费者（T0 下游）

- 024-Editor（2D 场景编辑、精灵/Tilemap 预览）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SpriteHandle | 精灵/图集句柄；LoadSprite、Atlas、Slice、SortOrder、SubmitToPipeline | 创建后直至显式释放 |
| TilemapHandle | 瓦片地图句柄；TileSet、Tilemap、Layer、CollisionLayer、Draw | 创建后直至显式释放 |
| Physics2DHandle | 2D 碰撞体/刚体桥接；Create2DBody、Create2DShape、与 Physics 对接 | 创建后直至显式释放 |
| 2DCamera / SortingLayer | 2D 相机、排序层；与 Pipeline 2D Pass、RenderCore 格式对接 | 由调用方或场景管理 |

下游仅通过上述类型与句柄访问；与 Resource 精灵/图集/瓦片集、Physics 2D、Pipeline 2D Pass、RenderCore 顶点格式对接。**ABI 显式表**：[022-2d-ABI.md](./022-2d-ABI.md)。

## 能力列表（提供方保证）

1. **Sprite**：ISprite::Load、GetAtlas、GetSlice、SetSortOrder、GetSortOrder、SubmitToPipeline；CreateSprite；与 Resource、Pipeline 2D 绘制对接。
2. **Tilemap**：ITilemap::GetTileSet、GetLayerCount、GetLayer、Draw、SetCollisionLayer、GetCollisionLayer；CreateTilemap；与 Pipeline 绘制对接。
3. **Physics2D**：Create2DBody、Create2DShape；与 014-Physics 2D 对接。
4. **Rendering2D**：ICamera2D、SortingLayer；与 Pipeline、RenderCore 对接。

## 调用顺序与约束

- 须在 Core、Resource、Physics、Pipeline、RenderCore 初始化之后使用；2D Pass 与 Pipeline 约定一致。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：022-2D 对应本契约；与 docs/module-specs/022-2d.md 一致 |
| 2026-01-28 | 根据 022-2d-ABI 反向更新：ISprite、ITilemap、CreateSprite、CreateTilemap、Create2DBody、Create2DShape、ICamera2D、SortingLayer；能力与类型与 ABI 表一致 |