# Research: 009-RenderCore Shader Reflection 对接

**Feature**: 009-render-core-shader-reflection-abi  
**Date**: 2026-02-03

## 1. UniformLayoutDesc 与 010-Shader 反射格式对齐

**Decision**: UniformLayoutDesc 使用 `te::rendercore::UniformMember` 结构（name[64], type, offset, size），与 010-Shader 契约中「Reflection（可选）」描述的 Uniform 布局、槽位约定一致。010-Shader GetReflection 产出时，成员类型映射使用 `te::rendercore::UniformMemberType` 枚举（Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4），偏移按 std140 规则计算。

**Rationale**: 009-RenderCore 已定义 UniformMemberType 与 UniformLayoutDesc，010-Shader 契约将 Reflection 列为可选能力；对齐由 010-Shader 产出方按 009 的 UniformMember 结构填充，避免 009 依赖 010 的具体实现。

**Alternatives considered**:
- 009 自行解析 SPIR-V：增加 009 对 spirv-cross 等依赖，违反模块边界
- 010 产出独立格式、009 转换：多余转换层；约定统一格式更简洁

## 2. std140 对齐规则

**Decision**: GetTotalSize 与成员偏移遵循 std140（OpenGL/Vulkan 通用）规则：标量 4B、vec2 8B、vec3/vec4 16B、mat3 48B、mat4 64B；结构体成员按 16B 对齐。

**Rationale**: RHI 与 Shader 后端普遍支持 std140；与 008-RHI、010-Shader 契约兼容。

## 3. 010-Shader GetReflection 尚未实现时的行为

**Decision**: 本 feature 支持手写 UniformLayoutDesc；格式约定以 ABI 与 010-Shader 契约为准。待 010-Shader 实现 GetReflection 后，通过契约测试验证格式对齐；当前不阻塞 009 实现。

**Rationale**: 模块可独立交付；反射为可选依赖，手写布局为常见用法。
