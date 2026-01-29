# Feature: 002-object minimal (speckit specify)

本 feature 的完整模块规约见 `docs/module-specs/002-object.md`，对外 API 契约见 `specs/_contracts/002-object-public-api.md`。依赖的 Core API 见 `specs/_contracts/001-core-public-api.md`。

本 feature 仅实现其中最小子集：**类型注册 TypeRegistry::RegisterType、简单序列化接口**（必须显式枚举）。

spec.md 中引用规约与契约，只描述本切片范围；不重复完整模块规约。实现时只使用 001-Core 契约中已声明的类型与 API。
