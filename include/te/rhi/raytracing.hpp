/**
 * @file raytracing.hpp
 * @brief RHI ray tracing (DXR) abstraction; D3D12 only implementation.
 */
#ifndef TE_RHI_RAYTRACING_HPP
#define TE_RHI_RAYTRACING_HPP

#include "te/rhi/types.hpp"
#include "te/rhi/resources.hpp"
#include <cstddef>
#include <cstdint>

namespace te {
namespace rhi {

/** Acceleration structure type. */
enum class AccelerationStructureType : uint32_t {
  BottomLevel = 0,
  TopLevel = 1,
};

/** Minimal geometry descriptor for BLAS (P2: extend with vertex/index buffers). */
struct RaytracingGeometryDesc {
  IBuffer* vertexBuffer{nullptr};
  size_t vertexOffset{0};
  uint32_t vertexCount{0};
  uint32_t vertexStride{0};
  IBuffer* indexBuffer{nullptr};
  size_t indexOffset{0};
  uint32_t indexCount{0};
  uint32_t transformOffset{0};  // optional 3x4 transform
};

/** Build inputs for acceleration structure (maps to D3D12 BLAS/TLAS). */
constexpr uint32_t kMaxRaytracingGeometries = 64u;
struct RaytracingAccelerationStructureDesc {
  AccelerationStructureType type{AccelerationStructureType::BottomLevel};
  uint32_t geometryCount{0};
  RaytracingGeometryDesc geometries[kMaxRaytracingGeometries]{};
};

/** Shader table / ray dispatch descriptor (maps to D3D12_DISPATCH_RAYS_DESC). */
struct DispatchRaysDesc {
  IBuffer* rayGenerationShaderTable{nullptr};
  size_t rayGenerationShaderTableSize{0};
  size_t rayGenerationShaderTableStride{0};
  IBuffer* missShaderTable{nullptr};
  size_t missShaderTableSize{0};
  size_t missShaderTableStride{0};
  IBuffer* hitGroupTable{nullptr};
  size_t hitGroupTableSize{0};
  size_t hitGroupTableStride{0};
  IBuffer* callableShaderTable{nullptr};
  size_t callableShaderTableSize{0};
  size_t callableShaderTableStride{0};
  uint32_t width{0};
  uint32_t height{0};
  uint32_t depth{1};
};

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_RAYTRACING_HPP
