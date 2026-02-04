# Data Model: 002-Object Full Module Content

**Feature**: 002-object-fullversion-002  
**Date**: 2026-01-29

## Entities

### TypeId

- **语义**: 类型唯一标识（uint32_t）
- **约束**: 0 或 kInvalidTypeId 表示无效；注册后直至卸载有效

### TypeDescriptor

- **字段**: id (TypeId), name (char const*), size (size_t), properties (PropertyDescriptor const*), propertyCount (size_t), methods (MethodDescriptor const*), methodCount (size_t), baseTypeId (TypeId)
- **关系**: 与类型绑定；baseTypeId 指向基类，无则为 0 或无效值

### PropertyDescriptor

- **字段**: name (char const*), valueTypeId (TypeId), defaultValue (void const*)
- **约束**: 元数据、范围/枚举可由扩展结构提供

### MethodDescriptor

- **语义**: 方法描述；本切片可为占位或最小集

### SerializedBuffer

- **字段**: data (void*), size (size_t), capacity (size_t)
- **生命周期**: 调用方管理；data 可由 Core Alloc 分配

### ObjectRef / GUID

- **字段**: uint8_t data[16]（或 guid[16]）
- **约束**: 跨资源引用仅读写 GUID，不存指针或路径

### PropertyBag

- **接口**: GetProperty(void* outValue, char const* name), SetProperty(void const* value, char const* name), FindProperty(char const* name)
- **关系**: 与类型或实例绑定；与反射和序列化联动

## State Transitions

- **类型注册**: RegisterType(TypeDescriptor) → 重复 TypeId 拒绝；重复 name 由实现约定
- **序列化**: Serialize(obj, buf, typeId) → 写出 formatVersion、数据、GUID；反序列化前若版本低则 Migrate 再 Deserialize
