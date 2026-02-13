/**
 * @file RenderingConfig.h
 * @brief Rendering configuration ( pipeline/lighting/etc ).
 */
#ifndef TE_EDITOR_RENDERING_CONFIG_H
#define TE_EDITOR_RENDERING_CONFIG_H

namespace te {
namespace editor {

struct RenderingConfig {
  char const* renderPath = "default";
  bool enableShadows = true;
  bool enableIBL = false;
  float exposure = 1.0f;
};

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_RENDERING_CONFIG_H
