/**
 * @file Canvas.cpp
 * @brief Canvas implementation (018-UI).
 */
#include <te/ui/Canvas.h>

namespace te {
namespace ui {

class CanvasImpl : public ICanvas {
public:
  void AddChild(uicore::ILayoutNode* child) override { (void)child; }
  void Layout() override {}
  void Draw() override {}
};

ICanvas* CreateCanvas() {
  return new CanvasImpl();
}

}  // namespace ui
}  // namespace te
