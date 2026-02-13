/**
 * @file HitTest.cpp
 * @brief Hit testing implementation (017-UICore).
 */
#include <te/uicore/HitTest.h>

namespace te {
namespace uicore {

namespace {
void HitTestRecursive(ILayoutNode* node, int x, int y, HitTestResult& out) {
  if (!node) return;
  Rect r = node->GetArrangeRect();
  if (x >= r.x && x < r.x + r.width && y >= r.y && y < r.y + r.height) {
    out.node = node;
    out.localX = static_cast<float>(x) - r.x;
    out.localY = static_cast<float>(y) - r.y;
    out.hit = true;
  }
}
}  // namespace

HitTestResult HitTest(ILayoutNode* root, int x, int y) {
  HitTestResult out;
  HitTestRecursive(root, x, y, out);
  return out;
}

}  // namespace uicore
}  // namespace te
