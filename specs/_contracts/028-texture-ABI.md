# 028-Texture 模块 ABI

- **契约**：[028-texture-public-api.md](./028-texture-public-api.md)（能力与类型描述）
- **本文件**：028-Texture 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| 028-Texture | te::texture | — | 数据结构 | te/texture/TextureAssetDesc.h | TextureAssetDesc | .texture 资产描述；028 拥有并向 002 注册；013 反序列化后交 028 |

---

数据与接口 TODO 已迁移至本模块契约 [028-texture-public-api.md](./028-texture-public-api.md) 的 TODO 列表；本文件仅保留 ABI 表与实现说明。
