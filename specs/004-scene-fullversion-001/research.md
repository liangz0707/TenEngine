# Research: 004-Scene 完整功能集

**Branch**: 004-scene-fullversion-001 | **Date**: 2026-01-29

## 技术栈

- **Decision**: C++17、CMake。
- **Rationale**: Constitution 规定 Language 为 C++17 or later；Build 为 CMake。本 feature 仅暴露契约已声明的类型与 API，不引入额外技术栈。
- **Alternatives considered**: C++20 延后至全仓统一升级；其他构建系统不采纳，以 Constitution 为准。

## 依赖边界

- **Decision**: 仅使用 001-Core、002-Object 契约中已声明的类型与 API；Transform 与 Core.Math 或共用数学类型一致（Vector3、Quaternion、Matrix4 等）；Level 资源句柄与 013-Resource 约定；Entity 根挂接与 005-Entity 在 plan 中约定。
- **Rationale**: 规约与 Constitution VI 规定「实现 MUST 只使用契约中已声明的类型与接口」。
- **Alternatives considered**: 无；跨契约类型需在 T0-contracts 上先扩展契约再使用。

## HierarchyIterator 生命周期

- **Decision**: 单次有效（遍历结束后即失效，不可复用）。
- **Rationale**: 已在本 feature 的 spec 澄清中采纳；实现采用栈上或单次分配的迭代器，Traverse/Find 返回后调用方不得再使用。
- **Alternatives considered**: 由调用方管理的迭代器会增加生命周期与线程安全复杂度，本切片不采纳。

## SetActiveWorld 生效时机

- **Decision**: 下一帧或下一次 UpdateTransforms 生效；本帧内 GetCurrentWorld 仍返回旧 WorldRef。
- **Rationale**: 已在本 feature 的 spec 澄清中采纳；实现可在帧边界或 UpdateTransforms 入口切换“当前 World”指针。
- **Alternatives considered**: 立即生效会增加同一帧内前后半段语义不一致的风险，不采纳。

## 父子成环 / 非法层级

- **Decision**: 拒绝并返回错误；API 返回错误码/结果，不修改场景图，调用方可恢复。
- **Rationale**: 已在本 feature 的 spec 澄清中采纳；实现中 SetParent 等会检查成环，返回 bool 或 Result 类型。
- **Alternatives considered**: 断言崩溃不利于下游恢复；未定义不利于验收。采纳可测试、可恢复的返回错误。

## LoadLevel / UnloadLevel 与 013-Resource

- **Decision**: 本 spec 不规定调用与失败语义；由 plan 与 013-Resource 对接时约定句柄、加载时机与失败语义。API 雏形中仅声明 LoadLevel/UnloadLevel 的签名占位，具体参数与返回在对接后补全。
- **Rationale**: 契约已写「关卡加载可与 013-Resource 约定」；spec 澄清采纳「plan 与 013 约定」。
- **Alternatives considered**: 在 004-Scene 契约中单方面规定同步/异步会与 013-Resource 契约冲突，故延后至对接。

## Entity 根挂接

- **Decision**: 本 spec 不规定接口归属；由 plan 与 005-Entity 对接时约定接口归属与签名。API 雏形中可预留“挂接根实体”的语义占位（如 AttachEntityRoot 或由 Entity 侧调用 Scene），具体签名在对接后补全。
- **Rationale**: spec 澄清采纳「plan 与 005-Entity 约定」。
- **Alternatives considered**: 无；接口归属属于跨模块设计，在 T0-contracts 上协调。
