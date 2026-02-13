/**
 * @file Focus.cpp
 * @brief Focus chain implementation (017-UICore).
 */
#include <te/uicore/Focus.h>
#include <te/application/Event.h>

namespace te {
namespace uicore {

namespace {
class FocusChainImpl : public IFocusChain {
public:
  ILayoutNode* GetFocus() const override { return m_focus; }
  void SetFocus(ILayoutNode* node) override { m_focus = node; }
  bool RouteEvent(te::application::Event const&) override { return false; }
private:
  ILayoutNode* m_focus = nullptr;
};
FocusChainImpl g_focusChain;
}  // namespace

IFocusChain* GetFocusChain() {
  return &g_focusChain;
}

}  // namespace uicore
}  // namespace te
