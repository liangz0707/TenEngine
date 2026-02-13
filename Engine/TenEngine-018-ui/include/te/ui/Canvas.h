/**
 * @file Canvas.h
 * @brief Canvas for UI (contract: specs/_contracts/018-ui-public-api.md).
 */
#ifndef TE_UI_CANVAS_H
#define TE_UI_CANVAS_H

#include <te/uicore/Layout.h>

namespace te {
namespace ui {

class ICanvas {
public:
  virtual ~ICanvas() = default;
  virtual void AddChild(uicore::ILayoutNode* child) = 0;
  virtual void Layout() = 0;
  virtual void Draw() = 0;
};

ICanvas* CreateCanvas();

}  // namespace ui
}  // namespace te

#endif  // TE_UI_CANVAS_H
