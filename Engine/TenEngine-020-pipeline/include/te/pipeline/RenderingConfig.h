/**
 * @file RenderingConfig.h
 * @brief 020-Pipeline: Rendering configuration and validation helpers.
 */

#pragma once

#include <cstdint>

namespace te::pipeline {

/// Validation level for rendering
enum class ValidationLevel : uint8_t {
  Disabled = 0,   // No validation
  Warning = 1,    // Log warnings only
  Error = 2,      // Log warnings and errors
  Strict = 3,     // Strict validation, abort on errors
};

/// Render path mode
enum class RenderPath : uint8_t {
  Forward = 0,    // Traditional forward rendering
  Deferred = 1,   // Deferred shading
  ForwardPlus = 2,// Forward with tiled/clustered shading
  Custom = 3,     // Custom render path
};

/// VSync mode
enum class VSyncMode : uint8_t {
  Off = 0,        // No VSync, unlimited FPS
  On = 1,         // VSync enabled
  Adaptive = 2,   // Adaptive VSync (sync when FPS >= refresh rate)
  Mailbox = 3,    // Mailbox mode (fast VSync, lowest latency)
};

/// HDR mode
enum class HDRMode : uint8_t {
  SDR = 0,        // Standard dynamic range
  HDR10 = 1,      // HDR10 (ST.2084 PQ curve)
  scRGB = 2,      // scRGB linear (Windows)
  DolbyVision = 3,// Dolby Vision (if supported)
};

/// Anti-aliasing mode
enum class AAMode : uint8_t {
  None = 0,
  MSAA2x = 1,
  MSAA4x = 2,
  MSAA8x = 3,
  TAA = 4,        // Temporal AA
  FXAA = 5,       // Fast approximate AA
  SMAA = 6,       // Subpixel morphological AA
};

/// Shadow quality level
enum class ShadowQuality : uint8_t {
  Off = 0,
  Low = 1,
  Medium = 2,
  High = 3,
  Ultra = 4,
};

/// Global rendering configuration
struct RenderingConfig {
  // Validation
  ValidationLevel validationLevel{ValidationLevel::Warning};
  bool enableDebugMarkers{false};      // GPU debug markers
  bool enableProfiling{false};         // CPU/GPU profiling

  // Render path
  RenderPath renderPath{RenderPath::Forward};

  // Display
  VSyncMode vsyncMode{VSyncMode::On};
  HDRMode hdrMode{HDRMode::SDR};
  uint32_t targetFrameRate{60};        // Target FPS (0 = unlimited)

  // Anti-aliasing
  AAMode aaMode{AAMode::None};
  uint32_t msaaSamples{1};             // 1, 2, 4, or 8

  // Shadows
  ShadowQuality shadowQuality{ShadowQuality::Medium};
  uint32_t shadowMapResolution{1024};
  uint32_t maxShadowCascades{4};
  float shadowDistance{100.0f};

  // Post-processing
  bool enableBloom{true};
  bool enableTonemapping{true};
  bool enableColorGrading{true};
  bool enableVignette{false};
  bool enableChromaticAberration{false};
  float bloomIntensity{0.5f};

  // Resolution
  uint32_t renderScale{100};           // Percentage (100 = native)
  bool dynamicResolution{false};       // Dynamic resolution scaling
  uint32_t minDynamicScale{50};        // Min scale percentage

  // Culling
  bool enableFrustumCulling{true};
  bool enableOcclusionCulling{false};
  bool enableLOD{true};

  // Instancing
  bool enableInstancing{true};
  uint32_t maxInstancesPerDraw{1024};

  // Multithreading
  bool enableMultithreadedRendering{true};
  uint32_t workerThreadCount{0};       // 0 = auto-detect

  // Memory
  uint32_t maxFramesInFlight{2};
  uint32_t transientResourcePoolSizeMB{256};

  // Effects
  bool enableParticles{true};
  bool enableDecals{true};
  bool enableVolumetricFog{false};
  bool enableScreenSpaceReflections{false};
  bool enableAmbientOcclusion{false};

  // Get scaled resolution
  void GetScaledResolution(uint32_t baseWidth, uint32_t baseHeight,
                           uint32_t& outWidth, uint32_t& outHeight) const {
    float scale = static_cast<float>(renderScale) / 100.0f;
    outWidth = static_cast<uint32_t>(baseWidth * scale);
    outHeight = static_cast<uint32_t>(baseHeight * scale);
    outWidth = (outWidth > 0) ? outWidth : 1;
    outHeight = (outHeight > 0) ? outHeight : 1;
  }

  // Check if MSAA is enabled
  bool IsMSAAEnabled() const {
    return aaMode >= AAMode::MSAA2x && aaMode <= AAMode::MSAA8x;
  }

  // Get MSAA sample count
  uint32_t GetMSAASampleCount() const {
    switch (aaMode) {
      case AAMode::MSAA2x: return 2;
      case AAMode::MSAA4x: return 4;
      case AAMode::MSAA8x: return 8;
      default: return 1;
    }
  }
};

// === Validation Helpers ===

/// Log a warning if validation is enabled
inline void CheckWarning(RenderingConfig const* config, char const* message) {
  if (config && static_cast<uint8_t>(config->validationLevel) >= 1) {
    // In production, this would log to a proper logging system
    // For now, use a simple approach
    (void)config;
    (void)message;
  }
}

/// Log an error if validation is enabled
inline void CheckError(RenderingConfig const* config, char const* message) {
  if (config && static_cast<uint8_t>(config->validationLevel) >= 2) {
    // In production, this would log to a proper logging system
    (void)config;
    (void)message;
  }
}

/// Strict check - may abort in strict mode
inline void CheckStrict(RenderingConfig const* config, char const* message, bool condition) {
  if (!condition) {
    CheckError(config, message);
    if (config && config->validationLevel == ValidationLevel::Strict) {
      // In production, would abort or throw
    }
  }
}

// === Default Configurations ===

/// Get default rendering configuration
inline RenderingConfig GetDefaultConfig() {
  return RenderingConfig{};
}

/// Get high quality rendering configuration
inline RenderingConfig GetHighQualityConfig() {
  RenderingConfig config{};
  config.aaMode = AAMode::TAA;
  config.shadowQuality = ShadowQuality::High;
  config.shadowMapResolution = 2048;
  config.enableBloom = true;
  config.enableTonemapping = true;
  config.enableColorGrading = true;
  config.enableScreenSpaceReflections = true;
  config.enableAmbientOcclusion = true;
  return config;
}

/// Get performance-oriented configuration
inline RenderingConfig GetPerformanceConfig() {
  RenderingConfig config{};
  config.validationLevel = ValidationLevel::Disabled;
  config.aaMode = AAMode::None;
  config.shadowQuality = ShadowQuality::Low;
  config.shadowMapResolution = 512;
  config.enableBloom = false;
  config.enableScreenSpaceReflections = false;
  config.enableAmbientOcclusion = false;
  config.renderScale = 75;
  return config;
}

}  // namespace te::pipeline
