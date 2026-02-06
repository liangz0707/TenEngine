/** @file SerializedBuffer.hpp
 *  ABI: specs/_contracts/002-object-ABI.md
 */
#ifndef TE_OBJECT_SERIALIZED_BUFFER_HPP
#define TE_OBJECT_SERIALIZED_BUFFER_HPP

#include <cstddef>

namespace te {
namespace object {

struct SerializedBuffer {
  void* data = nullptr;
  size_t size = 0;
  size_t capacity = 0;
};

}  // namespace object
}  // namespace te

#endif  // TE_OBJECT_SERIALIZED_BUFFER_HPP
