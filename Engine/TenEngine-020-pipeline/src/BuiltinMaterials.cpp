/**
 * @file BuiltinMaterials.cpp
 * @brief Implementation of BuiltinMaterials.
 */

#include <te/pipeline/BuiltinMaterials.h>

#include <te/rhi/device.hpp>
#include <te/rendercore/IRenderMaterial.hpp>

#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace te::pipeline {

// === Shader paths ===
static char const* kDefaultShaderPath = "builtin/shaders/";

// === Material names to IDs ===
static std::unordered_map<std::string, BuiltinMaterialId> sMaterialNames = {
  {"post_process/copy", BuiltinMaterialId::PostProcessCopy},
  {"post_process/tonemap", BuiltinMaterialId::PostProcessTonemap},
  {"post_process/bloom", BuiltinMaterialId::PostProcessBloom},
  {"lighting/point", BuiltinMaterialId::LightPoint},
  {"lighting/spot", BuiltinMaterialId::LightSpot},
  {"lighting/directional", BuiltinMaterialId::LightDirectional},
};

// === BuiltinMaterials::Impl ===

struct CachedMaterial {
  rendercore::IRenderMaterial* material{nullptr};
  BuiltinMaterialId id;
  size_t memoryUsed{0};
};

struct BuiltinMaterials::Impl {
  rhi::IDevice* device{nullptr};
  std::string shaderPath{kDefaultShaderPath};
  std::unordered_map<uint32_t, CachedMaterial> cache;
  std::mutex mutex;
  size_t totalMemory{0};

  uint32_t MakeKey(BuiltinMaterialId id, uint32_t param = 0) {
    return (static_cast<uint32_t>(id) << 8) | (param & 0xFF);
  }

  void Clear() {
    std::lock_guard<std::mutex> lock(mutex);
    // Materials are managed externally (by 011-material module)
    cache.clear();
    totalMemory = 0;
  }

  // Create a stub material for the given ID
  // Real implementation would load shaders and create PSOs
  rendercore::IRenderMaterial* CreateMaterial(BuiltinMaterialId id) {
    // This is a stub - actual material creation would:
    // 1. Load shader bytecode from disk or embedded
    // 2. Create IShaderEntry
    // 3. Create IRenderMaterial with PSO, descriptor sets, etc.

    // For now, return nullptr to indicate the pattern
    // The 011-material module would implement this properly
    (void)id;
    return nullptr;
  }
};

// === BuiltinMaterials ===

BuiltinMaterials::BuiltinMaterials()
  : impl_(std::make_unique<Impl>()) {
}

BuiltinMaterials::~BuiltinMaterials() {
  impl_->Clear();
}

void BuiltinMaterials::SetDevice(rhi::IDevice* device) {
  impl_->device = device;
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessCopy() {
  return GetMaterial(BuiltinMaterialId::PostProcessCopy);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessTonemap() {
  return GetMaterial(BuiltinMaterialId::PostProcessTonemap);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessBloom() {
  return GetMaterial(BuiltinMaterialId::PostProcessBloom);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessBloomDownsample() {
  return GetMaterial(BuiltinMaterialId::PostProcessBloomDownsample);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessBloomUpsample() {
  return GetMaterial(BuiltinMaterialId::PostProcessBloomUpsample);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessColorGrading() {
  return GetMaterial(BuiltinMaterialId::PostProcessColorGrading);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessVignette() {
  return GetMaterial(BuiltinMaterialId::PostProcessVignette);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessChromaticAberration() {
  return GetMaterial(BuiltinMaterialId::PostProcessChromaticAberration);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessFXAA() {
  return GetMaterial(BuiltinMaterialId::PostProcessFXAA);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetPostProcessTAA() {
  return GetMaterial(BuiltinMaterialId::PostProcessTAA);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetLightPoint() {
  return GetMaterial(BuiltinMaterialId::LightPoint);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetLightSpot() {
  return GetMaterial(BuiltinMaterialId::LightSpot);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetLightDirectional() {
  return GetMaterial(BuiltinMaterialId::LightDirectional);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetLightAmbient() {
  return GetMaterial(BuiltinMaterialId::LightAmbient);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetLightIBL() {
  return GetMaterial(BuiltinMaterialId::LightIBL);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetShadowDepthOpaque() {
  return GetMaterial(BuiltinMaterialId::ShadowDepthOpaque);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetShadowDepthAlphaTest() {
  return GetMaterial(BuiltinMaterialId::ShadowDepthAlphaTest);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetShadowDepthVSM() {
  return GetMaterial(BuiltinMaterialId::ShadowDepthVSM);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetDebugWireframe() {
  return GetMaterial(BuiltinMaterialId::DebugWireframe);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetDebugNormals() {
  return GetMaterial(BuiltinMaterialId::DebugNormals);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetDebugUVs() {
  return GetMaterial(BuiltinMaterialId::DebugUVs);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetDebugDepth() {
  return GetMaterial(BuiltinMaterialId::DebugDepth);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetDebugLightComplexity() {
  return GetMaterial(BuiltinMaterialId::DebugLightComplexity);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetClearColor() {
  return GetMaterial(BuiltinMaterialId::ClearColor);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetClearDepth() {
  return GetMaterial(BuiltinMaterialId::ClearDepth);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetBlit() {
  return GetMaterial(BuiltinMaterialId::Blit);
}

rendercore::IRenderMaterial* BuiltinMaterials::GetMaterial(
    BuiltinMaterialId id,
    BuiltinMaterialParams const* params) {

  uint32_t key = impl_->MakeKey(id, params ? params->renderPassIndex : 0);

  std::lock_guard<std::mutex> lock(impl_->mutex);

  auto it = impl_->cache.find(key);
  if (it != impl_->cache.end()) {
    return it->second.material;
  }

  // Create new material
  CachedMaterial cached;
  cached.id = id;
  cached.material = impl_->CreateMaterial(id);
  cached.memoryUsed = 0; // Would calculate actual size

  impl_->cache[key] = cached;
  impl_->totalMemory += cached.memoryUsed;

  return cached.material;
}

rendercore::IRenderMaterial* BuiltinMaterials::GetMaterialByName(char const* name) {
  if (!name) return nullptr;

  auto it = sMaterialNames.find(name);
  if (it != sMaterialNames.end()) {
    return GetMaterial(it->second);
  }
  return nullptr;
}

void BuiltinMaterials::WarmupCache() {
  // Create commonly used materials upfront
  GetPostProcessCopy();
  GetPostProcessTonemap();
  GetLightPoint();
  GetLightSpot();
  GetLightDirectional();
  GetShadowDepthOpaque();
}

void BuiltinMaterials::SetShaderSearchPath(char const* path) {
  if (path) {
    impl_->shaderPath = path;
  }
}

bool BuiltinMaterials::IsCached(BuiltinMaterialId id) const {
  std::lock_guard<std::mutex> lock(impl_->mutex);
  return impl_->cache.find(static_cast<uint32_t>(id)) != impl_->cache.end();
}

void BuiltinMaterials::ClearCache() {
  impl_->Clear();
}

size_t BuiltinMaterials::GetMemoryUsed() const {
  return impl_->totalMemory;
}

// === Global Instance ===

static std::unique_ptr<BuiltinMaterials> g_builtinMaterials;
static std::mutex g_builtinMutex;

BuiltinMaterials* GetBuiltinMaterials() {
  std::lock_guard<std::mutex> lock(g_builtinMutex);
  if (!g_builtinMaterials) {
    g_builtinMaterials = std::make_unique<BuiltinMaterials>();
  }
  return g_builtinMaterials.get();
}

void InitializeBuiltinMaterials(rhi::IDevice* device) {
  std::lock_guard<std::mutex> lock(g_builtinMutex);
  if (!g_builtinMaterials) {
    g_builtinMaterials = std::make_unique<BuiltinMaterials>();
  }
  g_builtinMaterials->SetDevice(device);
}

void ShutdownBuiltinMaterials() {
  std::lock_guard<std::mutex> lock(g_builtinMutex);
  g_builtinMaterials.reset();
}

// === Free Functions ===

BuiltinMaterials* CreateBuiltinMaterials() {
  return new BuiltinMaterials();
}

void DestroyBuiltinMaterials(BuiltinMaterials* materials) {
  delete materials;
}

// === Shader Path Helpers ===

char const* GetBuiltinShaderPath(char const* shaderName) {
  static std::string path;
  path = kDefaultShaderPath;
  path += shaderName;
  return path.c_str();
}

bool BuiltinShaderExists(char const* shaderName) {
  // Would check if shader file exists
  // For now, assume all builtin shaders exist
  (void)shaderName;
  return true;
}

}  // namespace te::pipeline
