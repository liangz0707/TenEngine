/** @file Serializer.hpp
 *  ABI: specs/_contracts/002-object-ABI.md
 */
#ifndef TE_OBJECT_SERIALIZER_HPP
#define TE_OBJECT_SERIALIZER_HPP

#include <cstdint>
#include <te/object/SerializedBuffer.hpp>
#include <te/object/TypeId.hpp>
#include <te/object/VersionMigration.hpp>

namespace te {
namespace object {

struct ISerializer {
  virtual bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) = 0;
  virtual bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) = 0;
  virtual uint32_t GetCurrentVersion() const = 0;
  virtual void SetVersionMigration(IVersionMigration* migration) = 0;
  virtual ~ISerializer() = default;
};

}  // namespace object
}  // namespace te

#endif  // TE_OBJECT_SERIALIZER_HPP
