/**
 * @file ShaderAssetDesc.h
 * @brief Shader asset description (contract: specs/_contracts/010-shader-ABI.md).
 *
 * ShaderAssetDesc is owned by 010-Shader. Serialized format: .shader (AssetDesc) + same-dir source file.
 */
#ifndef TE_SHADER_SHADER_ASSET_DESC_H
#define TE_SHADER_SHADER_ASSET_DESC_H

#include <te/resource/ResourceId.h>
#include <te/shader/types.hpp>
#include <cstddef>

namespace te {
namespace shader {

/** Max length for source file name in asset desc (serializable as fixed buffer). */
constexpr size_t kShaderSourceFileNameMaxLen = 256;

/**
 * Shader asset description.
 * One directory per shader: .shader file (this desc) + source file (e.g. main.hlsl).
 * Load uses desc->guid as ResourceId for cache; no separate .shader.data.
 */
struct ShaderAssetDesc {
  resource::ResourceId guid;
  /** Source file name in same directory (e.g. "main.hlsl", "pbr.glsl"). Null-terminated. */
  char sourceFileName[kShaderSourceFileNameMaxLen] = {};
  ShaderSourceFormat sourceFormat = ShaderSourceFormat::HLSL;
  /** Default compile options (stage, entryPoint, etc.). */
  CompileOptions compileOptions;

  bool IsValid() const {
    return !guid.IsNull() && sourceFileName[0] != '\0';
  }
};

}  // namespace shader
}  // namespace te

#endif  // TE_SHADER_SHADER_ASSET_DESC_H
