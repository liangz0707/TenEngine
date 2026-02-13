/**
 * @file Focus.h
 * @brief Focus chain for UI (contract: specs/_contracts/017-uicore-ABI.md).
 */
#ifndef TE_UICORE_FOCUS_H
#define TE_UICORE_FOCUS_H

#include "Layout.h"

namespace te {
namespace application {
struct Event;
}  // namespace application

namespace uicore {

/** Focus chain: manages focus and routes input events. */
class IFocusChain {
public:
  virtual ~IFocusChain() = default;
  virtual ILayoutNode* GetFocus() const = 0;
  virtual void SetFocus(ILayoutNode* node) = 0;
  virtual bool RouteEvent(te::application::Event const& ev) = 0;
};

/** Get global focus chain. Managed by UICore; caller does not own. */
IFocusChain* GetFocusChain();

}  // namespace uicore
}  // namespace te

#endif  // TE_UICORE_FOCUS_H
