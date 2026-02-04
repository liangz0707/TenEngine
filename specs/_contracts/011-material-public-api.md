# 契约：011-Material 模块对外 API

## 适用模块

- **实现方**：**011-Material**（材质定义与 Shader 绑定）
- **对应规格**：`docs/module-specs/011-material.md`
- **依赖**：009-RenderCore（009-rendercore-public-api）、010-Shader（010-shader-public-api）

## 消费者（T0 下游）

- 020-Pipeline（材质实例、DrawCall、SubmitToPipeline、BindToPSO）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| MaterialHandle | 材质资源句柄（**引擎自有格式**）；Load、GetParameters、GetShaderRef、GetTextureRefs；**材质中保存了 Shader**，并引用贴图与材质参数 | 创建后直至显式释放 |
| MaterialInstanceHandle | 材质实例句柄；CreateInstance、SetOverride、Release、Pool | 创建后直至显式释放 |
| ParameterSlot | 参数槽位；SetScalar、SetTexture、SetBuffer、与 RenderCore Uniform/纹理槽对接 | 与材质或实例绑定 |
| VariantKey | 与 Shader 变体对应；BindToPSO、GetVariantKey、SubmitToPipeline | 由调用方管理 |

下游仅通过上述类型与句柄访问；不直接持有 GPU 资源，通过 Pipeline 提交时绑定 PSO 与资源。

## 能力列表（提供方保证）

1. **MaterialDef**：Load、GetParameters、GetDefaultValues、GetShaderRef、GetTextureRefs；材质为**引擎自有格式**，**保存 Shader 引用**，并引用贴图与材质参数（渲染 Shader 的参数值）。
2. **Parameters**：SetScalar、SetTexture、SetBuffer、GetSlotMapping；与 RenderCore Uniform/纹理槽对接。
3. **Instancing**：CreateInstance、SetOverride、Release、Pool；实例池与生命周期。
4. **Binding**：BindToPSO、GetVariantKey、SubmitToPipeline；与 Shader 变体、RHI PSO、Pipeline 提交接口对接。

## 调用顺序与约束

- 须在 RenderCore、Shader 初始化之后使用；参数槽位与 RenderCore/Shader 约定一致。
- 材质实例释放顺序须与 Pipeline DrawCall 生命周期协调。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：011-Material 对应本契约；与 docs/module-specs/011-material.md 一致 |
