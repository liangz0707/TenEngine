/**
 * @file DrawList.h
 * @brief Draw command list, ImGui backend (contract: specs/_contracts/017-uicore-ABI.md).
 */
#ifndef TE_UICORE_DRAW_LIST_H
#define TE_UICORE_DRAW_LIST_H

#include "Layout.h"

namespace te {
namespace uicore {

/** RGBA color (0-1). */
struct Color {
  float r = 1.f, g = 1.f, b = 1.f, a = 1.f;
};

/** Draw command list: records DrawRect/DrawTexture/DrawText; submits to ImGui. */
class IDrawCommandList {
public:
  virtual ~IDrawCommandList() = default;
  virtual void DrawRect(Rect const& rect, Color const& color) = 0;
  virtual void DrawTexture(Rect const& rect, void* textureId, Rect const* uv) = 0;
  virtual void DrawText(Rect const& rect, char const* text, void* font, Color const& color) = 0;
  /** Submit to ImGui (render to current window). Does not depend on Pipeline. */
  virtual void Submit() = 0;
};

/** Create draw command list. Caller does not own; single-frame use. */
IDrawCommandList* CreateDrawCommandList();

}  // namespace uicore
}  // namespace te

#endif  // TE_UICORE_DRAW_LIST_H
