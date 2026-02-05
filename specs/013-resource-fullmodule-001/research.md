# Research: 013-Resource 完整模块实现

**Feature**: 013-resource-fullmodule-001  
**Date**: 2026-02-05

## 1. 资源管理统一入口与缓存语义

**Decision**: 统一加载入口（RequestLoadAsync/LoadSync）为唯一入口；同一 ResourceId 多次 Load 返回同一 IResource*，引用计数；GetCached 仅查缓存，未命中返回 nullptr 不触发加载。

**Rationale**: 与 spec 澄清一致；与 Unity Addressables、Unreal StreamableManager 的「按 ID 缓存、按需加载」模式一致；职责清晰，避免 GetCached 侧效应。

**Alternatives considered**: GetCached 未命中时触发按需加载 — 增加 API 歧义与调用线程不确定性，故不采用。

---

## 2. 反序列化与 Loader 边界

**Decision**: 013 按 ResourceType 调用各模块注册的反序列化器，得到 opaque payload 后原样传入已注册的 IResourceLoader；013 不解析 *Desc，不要求直接调 002 按 TypeId 反序列化。

**Rationale**: 各模块拥有 *Desc 与反序列化逻辑，013 只做调度与磁盘 I/O；与契约「*Desc 对 013 不可见」一致；各模块可在反序列化器内部使用 002。

**Alternatives considered**: 013 统一调 002 按 TypeId 反序列化 — 需 013 维护 ResourceType→TypeId 映射且与 002 强耦合，故采用「按 ResourceType 调各模块反序列化器」。

---

## 3. 循环引用策略

**Decision**: 禁止循环引用；检测到循环时加载失败并返回 LoadResult::Error（或等价）；依赖图必须无环。

**Rationale**: 实现与语义简单；避免延迟/弱引用带来的生命周期与回调顺序复杂化；与 spec 澄清一致。

**Alternatives considered**: 延迟或弱引用 — 需在 plan 中细化「根已创建」与依赖就绪顺序，增加实现与测试成本，本 feature 采用禁止策略。

---

## 4. 异步加载完成语义

**Decision**: RequestLoadAsync 的 LoadCompleteCallback 仅在「根资源及其递归依赖均加载完成」时调用一次；不单独提供「根已创建」回调；GetLoadStatus 区分 Pending/Loading/Completed/Failed/Cancelled。

**Rationale**: 调用方拿到的 IResource* 即可安全使用，无需再轮询依赖；与 spec 澄清一致；若日后需要「根已创建」可再扩展。

**Alternatives considered**: 双回调（根已创建 + 已加载）— 增加 API 与实现复杂度，本 feature 仅「已加载」单次回调。

---

## 5. 技术栈与测试

**Decision**: C++17；CMake 构建；单元测试与集成测试验证契约行为（加载、缓存、卸载、引用计数、循环检测）；目标平台 Win/Linux/macOS。

**Rationale**: 与 Constitution 及 `docs/engine-build-module-convention.md` 一致；013 无自有存储，仅内存缓存与文件 I/O；测试以契约与 spec 验收场景为准。

**Alternatives considered**: 无；语言与构建由引擎规约确定。
