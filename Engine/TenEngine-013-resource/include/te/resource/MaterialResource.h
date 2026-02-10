/**
 * @file MaterialResource.h
 * @brief IMaterialResource view (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_MATERIAL_RESOURCE_H
#define TE_RESOURCE_MATERIAL_RESOURCE_H

#include <te/resource/Resource.h>
#include <cstdint>

namespace te {
namespace resource {

/** Slot for a texture binding (set, binding). Used by GetTextureRefs. */
struct MaterialTextureSlot {
  std::uint32_t set = 0;
  std::uint32_t binding = 0;
};

/** Material resource view; implemented by 011; 013 returns IResource* then caller may cast. */
class IMaterialResource : public IResource {
 public:
  ~IMaterialResource() override = default;

  /**
   * Get texture references: (set, binding) and GUID string per slot.
   * outPaths[i] is the texture ResourceId as GUID string (valid until next Load or destroy).
   * @return number of texture slots filled (up to maxCount).
   */
  virtual std::uint32_t GetTextureRefs(MaterialTextureSlot* outSlots, char const** outPaths,
                                       std::uint32_t maxCount) const = 0;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_MATERIAL_RESOURCE_H
