/** SerializedBuffer (contract: 002-object-public-api.md fullversion-001) */

#ifndef TE_OBJECT_SERIALIZED_BUFFER_HPP
#define TE_OBJECT_SERIALIZED_BUFFER_HPP

#include <cstddef>

namespace te::object {

/** Caller-owned buffer for serialized bytes; data may be allocated with Core Alloc/Free. */
struct SerializedBuffer {
    void*  data;
    size_t size;
    size_t capacity;
};

}  // namespace te::object

#endif
