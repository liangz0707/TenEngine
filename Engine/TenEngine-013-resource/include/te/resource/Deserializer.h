/**
 * @file Deserializer.h
 * @brief IDeserializer (contract: specs/_contracts/013-resource-ABI.md).
 *
 * Serialization: Deserialize is per-type via RegisterDeserializer (buffer -> opaque payload).
 * The inverse (IResource -> memory/disk) is per-type via IResourceManager::RegisterSaver and
 * Save(IResource*, path); modules register a saver that produces content and 013 or the module writes.
 */
#ifndef TE_RESOURCE_DESERIALIZER_H
#define TE_RESOURCE_DESERIALIZER_H

#include <cstddef>

namespace te {
namespace resource {

/** Per-type deserializer; buffer -> opaque payload; 013 does not parse payload. */
class IDeserializer {
 public:
  virtual ~IDeserializer() = default;
  virtual void* Deserialize(void const* buffer, size_t size) = 0;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_DESERIALIZER_H
