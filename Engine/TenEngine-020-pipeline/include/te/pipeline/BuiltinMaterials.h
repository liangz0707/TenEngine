/**
 * @file BuiltinMaterials.h
 * @brief 020-Pipeline: Built-in materials for rendering.
 *
 * Provides common materials used in rendering:
 * - Post-process materials (tonemapping, bloom, etc.)
 * - Light volume materials
 * - Debug visualization materials
 * - Shadow depth materials
 */

#pragma once

#include <te/rendercore/types.hpp>
#include <cstdint>
#include <cstddef>
#include <memory>

namespace te::rhi {
struct IDevice;
struct IRenderPass;
struct IDescriptorSetLayout;
}

namespace te::rendercore {
struct IRenderMaterial;
struct IShaderEntry;
}

namespace te::pipeline {

/// Builtin material identifiers
enum class BuiltinMaterialId : uint32_t {
  // Post-processing
  PostProcessCopy = 0,
  PostProcessTonemap = 1,
  PostProcessBloom = 2,
  PostProcessBloomDownsample = 3,
  PostProcessBloomUpsample = 4,
  PostProcessColorGrading = 5,
  PostProcessVignette = 6,
  PostProcessChromaticAberration = 7,
  PostProcessFXAA = 8,
  PostProcessTAA = 9,

  // Lighting
  LightPoint = 10,
  LightSpot = 11,
  LightDirectional = 12,
  LightAmbient = 13,
  LightIBL = 14,

  // Shadows
  ShadowDepthOpaque = 15,
  ShadowDepthAlphaTest = 16,
  ShadowDepthVSM = 17,

  // Debug
  DebugWireframe = 20,
  DebugNormals = 21,
  DebugUVs = 22,
  DebugDepth = 23,
  DebugLightComplexity = 24,

  // Utility
  ClearColor = 30,
  ClearDepth = 31,
  Blit = 32,

  Count = 33
};

/// Material creation parameters
struct BuiltinMaterialParams {
  BuiltinMaterialId id{BuiltinMaterialId::PostProcessCopy};
  char const* customShaderPath{nullptr};  // Override with custom shader
  uint32_t renderPassIndex{0};            // For multi-pass materials
  uint32_t subpassIndex{0};
  bool alphaBlend{false};
  bool depthTest{false};
  bool depthWrite{true};
};

/**
 * @brief BuiltinMaterials manages cached built-in materials.
 *
 * Creates and caches materials on first use. Thread-safe for access.
 * Must call SetDevice() before use.
 *
 * Shader paths are relative to builtin/shaders/:
 * - post_process/copy.hlsl
 * - post_process/tonemap.hlsl
 * - lighting/point_light.hlsl
 * etc.
 */
class BuiltinMaterials {
public:
  BuiltinMaterials();
  ~BuiltinMaterials();

  BuiltinMaterials(BuiltinMaterials const&) = delete;
  BuiltinMaterials& operator=(BuiltinMaterials const&) = delete;

  /// Set the RHI device (required before creating materials)
  void SetDevice(rhi::IDevice* device);

  // === Post-Process Materials ===

  /// Get simple copy/blit material
  rendercore::IRenderMaterial* GetPostProcessCopy();

  /// Get tonemapping material (ACES, Reinhard, etc.)
  rendercore::IRenderMaterial* GetPostProcessTonemap();

  /// Get bloom threshold + downsample material
  rendercore::IRenderMaterial* GetPostProcessBloom();

  /// Get bloom downsample material
  rendercore::IRenderMaterial* GetPostProcessBloomDownsample();

  /// Get bloom upsample material
  rendercore::IRenderMaterial* GetPostProcessBloomUpsample();

  /// Get color grading (LUT-based) material
  rendercore::IRenderMaterial* GetPostProcessColorGrading();

  /// Get vignette material
  rendercore::IRenderMaterial* GetPostProcessVignette();

  /// Get chromatic aberration material
  rendercore::IRenderMaterial* GetPostProcessChromaticAberration();

  /// Get FXAA material
  rendercore::IRenderMaterial* GetPostProcessFXAA();

  /// Get TAA material
  rendercore::IRenderMaterial* GetPostProcessTAA();

  // === Lighting Materials ===

  /// Get point light volume material
  rendercore::IRenderMaterial* GetLightPoint();

  /// Get spot light volume material
  rendercore::IRenderMaterial* GetLightSpot();

  /// Get directional light material
  rendercore::IRenderMaterial* GetLightDirectional();

  /// Get ambient light material
  rendercore::IRenderMaterial* GetLightAmbient();

  /// Get IBL material
  rendercore::IRenderMaterial* GetLightIBL();

  // === Shadow Materials ===

  /// Get shadow depth material for opaque objects
  rendercore::IRenderMaterial* GetShadowDepthOpaque();

  /// Get shadow depth material for alpha-tested objects
  rendercore::IRenderMaterial* GetShadowDepthAlphaTest();

  /// Get VSM (Variance Shadow Map) material
  rendercore::IRenderMaterial* GetShadowDepthVSM();

  // === Debug Materials ===

  /// Get wireframe debug material
  rendercore::IRenderMaterial* GetDebugWireframe();

  /// Get normal visualization material
  rendercore::IRenderMaterial* GetDebugNormals();

  /// Get UV visualization material
  rendercore::IRenderMaterial* GetDebugUVs();

  /// Get depth visualization material
  rendercore::IRenderMaterial* GetDebugDepth();

  /// Get light complexity debug material
  rendercore::IRenderMaterial* GetDebugLightComplexity();

  // === Utility Materials ===

  /// Get clear color material
  rendercore::IRenderMaterial* GetClearColor();

  /// Get clear depth material
  rendercore::IRenderMaterial* GetClearDepth();

  /// Get blit material
  rendercore::IRenderMaterial* GetBlit();

  // === Generic Access ===

  /// Get material by ID with parameters
  rendercore::IRenderMaterial* GetMaterial(
    BuiltinMaterialId id,
    BuiltinMaterialParams const* params = nullptr);

  /// Get material by name (e.g., "post_process/tonemap")
  rendercore::IRenderMaterial* GetMaterialByName(char const* name);

  // === Resource Creation ===

  /// Create all commonly used materials upfront
  void WarmupCache();

  /// Set custom shader search path (default: builtin/shaders/)
  void SetShaderSearchPath(char const* path);

  // === Utility ===

  /// Check if a material is cached
  bool IsCached(BuiltinMaterialId id) const;

  /// Clear all cached materials
  void ClearCache();

  /// Get memory used by cached materials
  size_t GetMemoryUsed() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

// === Global Access ===

/// Get global builtin materials instance
BuiltinMaterials* GetBuiltinMaterials();

/// Initialize global builtin materials
void InitializeBuiltinMaterials(rhi::IDevice* device);

/// Shutdown global builtin materials
void ShutdownBuiltinMaterials();

// === Free Functions ===

/// Create a builtin materials instance
BuiltinMaterials* CreateBuiltinMaterials();

/// Destroy a builtin materials instance
void DestroyBuiltinMaterials(BuiltinMaterials* materials);

// === Shader Path Helpers ===

/// Get full shader path for builtin shader
char const* GetBuiltinShaderPath(char const* shaderName);

/// Check if builtin shader exists
bool BuiltinShaderExists(char const* shaderName);

}  // namespace te::pipeline
