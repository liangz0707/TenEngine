/**
 * @file RenderingConfig.h
 * @brief 020-Pipeline rendering config (Deferred/Forward, lights, shadows, etc.); shared with Editor.
 * Contract: specs/_contracts/020-pipeline-ABI.md
 */

#ifndef TE_PIPELINE_RENDERING_CONFIG_H
#define TE_PIPELINE_RENDERING_CONFIG_H

#include <cstdint>

namespace te {
namespace pipeline {

/// 渲染路径
enum class RenderPath : uint32_t {
  Deferred = 0,
  Forward,
  FullCompute,
};

/// 校验程度（与 RenderMode 对应）
enum class ValidationLevel : uint32_t {
  Debug = 0,   ///< 全量 CheckWarning/CheckError
  Hybrid,      ///< 部分校验
  Resource,    ///< 发布/最小校验
};

/// 渲染配置；Editor 与 Pipeline 共用，下一帧生效
struct RenderingConfig {
  RenderPath renderPath{RenderPath::Deferred};
  ValidationLevel validationLevel{ValidationLevel::Resource};
  uint32_t enableShadows{1u};
  uint32_t enableIBL{0u};
  uint32_t enableDOF{0u};
  uint32_t antiAlias{0u};  ///< 0=off, 1=FXAA, 2=TAA, ...
  // 可扩展：太阳方向、强度、阴影分辨率等
};

/// 按 validationLevel 报告警告（Debug/Hybrid 可打日志）
inline void CheckWarning(RenderingConfig const& config, char const* msg) {
  (void)msg;
  if (config.validationLevel == ValidationLevel::Debug || config.validationLevel == ValidationLevel::Hybrid) {
    (void)0;  /* 可接入日志：TE_LOG_WARNING("Pipeline: %s", msg); */
  }
}

/// 按 validationLevel 报告错误（Debug 可断言）
inline void CheckError(RenderingConfig const& config, char const* msg) {
  (void)msg;
  if (config.validationLevel == ValidationLevel::Debug) {
    (void)0;  /* 可接入断言或日志 */
  }
}

}  // namespace pipeline
}  // namespace te

#endif  // TE_PIPELINE_RENDERING_CONFIG_H
