/**
 * @file BuiltinMeshes.h
 * @brief 020-Pipeline: Built-in mesh primitives for rendering.
 *
 * Provides common geometric primitives used in rendering:
 * - Fullscreen quad (post-processing)
 * - Sphere (point light volumes)
 * - Cone (spot light volumes)
 * - Box/_cube (directional light volumes, decals)
 * - Cylinder/_capsule (various effects)
 */

#pragma once

#include <te/rendercore/types.hpp>
#include <cstdint>
#include <cstddef>

namespace te::rhi {
struct IDevice;
struct IBuffer;
}

namespace te::rendercore {
struct IRenderMesh;
struct IRenderElement;
}

namespace te::pipeline {

/// Builtin mesh identifiers
enum class BuiltinMeshId : uint32_t {
  FullscreenQuad = 0,
  Sphere16 = 1,       // 16 segments
  Sphere32 = 2,       // 32 segments
  Sphere64 = 3,       // 64 segments
  Cone = 4,
  ConeTruncated = 5,  // Cone with cap
  Cube = 6,
  Box = 7,            // Axis-aligned box
  Cylinder = 8,
  Capsule = 9,
  Plane2x2 = 10,      // 2x2 subdivided plane
  Plane8x8 = 11,      // 8x8 subdivided plane
  Count = 12
};

/// Mesh creation parameters
struct BuiltinMeshParams {
  BuiltinMeshId id{BuiltinMeshId::FullscreenQuad};
  uint32_t segments{32};        // For sphere, cylinder, etc.
  uint32_t rings{16};           // For sphere
  float radius{1.0f};           // For sphere, cylinder
  float height{1.0f};           // For cylinder, cone
  bool generateNormals{true};
  bool generateUVs{true};
  bool generateTangents{false};
};

/**
 * @brief BuiltinMeshes manages cached built-in geometry.
 *
 * Creates and caches meshes on first use. Thread-safe for access.
 * Must call SetDevice() before use.
 */
class BuiltinMeshes {
public:
  BuiltinMeshes();
  ~BuiltinMeshes();

  BuiltinMeshes(BuiltinMeshes const&) = delete;
  BuiltinMeshes& operator=(BuiltinMeshes const&) = delete;

  /// Set the RHI device (required before creating meshes)
  void SetDevice(rhi::IDevice* device);

  // === Mesh Access ===

  /// Get fullscreen quad mesh (for post-processing)
  rendercore::IRenderMesh* GetFullscreenQuad();

  /// Get sphere mesh with specified segments
  rendercore::IRenderMesh* GetSphere(uint32_t segments = 32);

  /// Get cone mesh
  rendercore::IRenderMesh* GetCone(float height = 1.0f, float radius = 1.0f);

  /// Get truncated cone (cone with cap)
  rendercore::IRenderMesh* GetConeTruncated(float height = 1.0f, float radius = 1.0f);

  /// Get cube mesh
  rendercore::IRenderMesh* GetCube();

  /// Get box mesh (axis-aligned, centered)
  rendercore::IRenderMesh* GetBox(float sizeX = 1.0f, float sizeY = 1.0f, float sizeZ = 1.0f);

  /// Get cylinder mesh
  rendercore::IRenderMesh* GetCylinder(float height = 1.0f, float radius = 1.0f);

  /// Get capsule mesh
  rendercore::IRenderMesh* GetCapsule(float height = 1.0f, float radius = 0.5f);

  /// Get subdivided plane
  rendercore::IRenderMesh* GetPlane(uint32_t subdivisions = 8);

  /// Get mesh by ID with parameters
  rendercore::IRenderMesh* GetMesh(BuiltinMeshId id, BuiltinMeshParams const* params = nullptr);

  // === Direct Vertex/Index Access ===

  /// Get fullscreen quad vertices (NDC space)
  static void GetFullscreenQuadVertices(float* outVertices, size_t stride, float* outUVs = nullptr);

  /// Get fullscreen quad indices
  static void GetFullscreenQuadIndices(uint16_t* outIndices);

  /// Get sphere vertex count
  static size_t GetSphereVertexCount(uint32_t segments, uint32_t rings);

  /// Get sphere index count
  static size_t GetSphereIndexCount(uint32_t segments, uint32_t rings);

  /// Generate sphere geometry
  static void GenerateSphere(
    float* outPositions, float* outNormals, float* outUVs, uint16_t* outIndices,
    uint32_t segments, uint32_t rings, float radius);

  // === Utility ===

  /// Check if a mesh is cached
  bool IsCached(BuiltinMeshId id) const;

  /// Clear all cached meshes
  void ClearCache();

  /// Get memory used by cached meshes
  size_t GetMemoryUsed() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

// === Global Access ===

/// Get global builtin meshes instance
BuiltinMeshes* GetBuiltinMeshes();

/// Initialize global builtin meshes
void InitializeBuiltinMeshes(rhi::IDevice* device);

/// Shutdown global builtin meshes
void ShutdownBuiltinMeshes();

// === Free Functions ===

/// Create a builtin meshes instance
BuiltinMeshes* CreateBuiltinMeshes();

/// Destroy a builtin meshes instance
void DestroyBuiltinMeshes(BuiltinMeshes* meshes);

}  // namespace te::pipeline
