/**
 * @file descriptor_set.hpp
 * @brief RHI descriptor set layout and descriptor set (P2); contract: specs/_contracts/008-rhi-ABI.md + delta.
 */
#ifndef TE_RHI_DESCRIPTOR_SET_HPP
#define TE_RHI_DESCRIPTOR_SET_HPP

#include "te/rhi/types.hpp"
#include "te/rhi/resources.hpp"
#include <cstddef>
#include <cstdint>

namespace te {
namespace rhi {

/** Descriptor type for bindings. */
enum class DescriptorType : uint32_t {
  Sampler = 0,
  UniformBuffer = 1,
  ShaderResource = 2,
  UnorderedAccess = 3,
};

/** Single binding in a descriptor set layout. */
struct DescriptorSetLayoutBinding {
  uint32_t binding{0};
  DescriptorType type{DescriptorType::UniformBuffer};
  uint32_t count{1};
  uint32_t stageFlags{0};  // shader stage mask
};

/** Descriptor set layout description (P2). */
constexpr uint32_t kMaxDescriptorSetLayoutBindings = 16u;
struct DescriptorSetLayoutDesc {
  uint32_t bindingCount{0};
  DescriptorSetLayoutBinding bindings[kMaxDescriptorSetLayoutBindings]{};
};

/** Single descriptor write (dstSet, binding, type, resource). */
struct DescriptorWrite {
  void* dstSet{nullptr};   // IDescriptorSet* cast to void for header independence
  uint32_t binding{0};
  DescriptorType type{DescriptorType::UniformBuffer};
  IBuffer* buffer{nullptr};
  ITexture* texture{nullptr};
  ISampler* sampler{nullptr};
  size_t offset{0};
  size_t range{0};
};

/** Descriptor set layout; immutable after creation. */
struct IDescriptorSetLayout {
  virtual ~IDescriptorSetLayout() = default;
};

/** Descriptor set; writable via UpdateDescriptorSet. */
struct IDescriptorSet {
  virtual ~IDescriptorSet() = default;
};

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_DESCRIPTOR_SET_HPP
