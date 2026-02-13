/**
 * @file HitTest.h
 * @brief Hit testing for UI (contract: specs/_contracts/017-uicore-ABI.md).
 */
#ifndef TE_UICORE_HIT_TEST_H
#define TE_UICORE_HIT_TEST_H

#include "Layout.h"

namespace te {
namespace uicore {

/** Result of hit test: node and local coordinates. */
struct HitTestResult {
  ILayoutNode* node = nullptr;
  float localX = 0.f;
  float localY = 0.f;
  bool hit = false;
};

/** Hit test at (x, y) against layout tree. */
HitTestResult HitTest(ILayoutNode* root, int x, int y);

}  // namespace uicore
}  // namespace te

#endif  // TE_UICORE_HIT_TEST_H
