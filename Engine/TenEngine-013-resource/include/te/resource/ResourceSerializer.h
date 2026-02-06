// 013-Resource: IResourceSerializer — 序列化与反序列化一致（Load 用 Deserialize，Save 用 Serialize）
#pragma once

#include <cstddef>

namespace te {
namespace resource {

class IResource;

class IResourceSerializer {
public:
    virtual ~IResourceSerializer() = default;
    // Load 路径：buffer → opaque payload；013 不解析内容
    virtual void* Deserialize(void const* buffer, size_t size) = 0;
    // Save 路径：resource → 写入 out_buffer，最多 buffer_size 字节，out_written 返回实际写入；返回 true 表示成功
    virtual bool Serialize(IResource* resource, void* out_buffer, size_t buffer_size, size_t* out_written) = 0;
};

} // namespace resource
} // namespace te
