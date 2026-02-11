/** @file descriptor_set.hpp
 *  008-RHI ABI: DescriptorSetLayoutDesc, DescriptorWrite, IDescriptorSetLayout, IDescriptorSet.
 */
#pragma once

#include <te/rhi/types.hpp>
#include <te/rhi/resources.hpp>
#include <cstdint>

namespace te {
namespace rhi {

/** Descriptor type for layout/reflection mapping. Vulkan: VK_DESCRIPTOR_TYPE_*. */
enum class DescriptorType : uint32_t {
  UniformBuffer = 0,
  CombinedImageSampler = 1,
  Sampler = 2,
  StorageBuffer = 3,
  StorageImage = 4,
};

struct DescriptorSetLayoutBinding {
  uint32_t binding;
  uint32_t descriptorType;  /* DescriptorType or backend-specific (e.g. VK_DESCRIPTOR_TYPE_*) */
  uint32_t descriptorCount;
};

struct DescriptorSetLayoutDesc {
  static constexpr uint32_t kMaxBindings = 16u;
  DescriptorSetLayoutBinding bindings[kMaxBindings];
  uint32_t bindingCount;
};

struct IDescriptorSet;

struct DescriptorWrite {
  IDescriptorSet* dstSet;
  uint32_t        binding;
  uint32_t        type;
  IBuffer*        buffer;
  size_t          bufferOffset;  /* for uniform buffer ring offset */
  ITexture*       texture;
  ISampler*       sampler;
};

struct IDescriptorSetLayout {
  virtual ~IDescriptorSetLayout() = default;
};

struct IDescriptorSet {
  virtual ~IDescriptorSet() = default;
};

}  // namespace rhi
}  // namespace te
