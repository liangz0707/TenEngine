/**
 * @file ResourceViewImpl.cpp
 * @brief Resource browser stub (024-Editor).
 */
#include <te/editor/ResourceView.h>
#include <string>

namespace te {
namespace editor {

class ResourceViewImpl : public IResourceView {
public:
  void OnDraw() override {}
  void SetRootPath(char const* path) override { m_rootPath = path ? path : ""; }
private:
  std::string m_rootPath;
};

IResourceView* CreateResourceView() {
  return new ResourceViewImpl();
}

}  // namespace editor
}  // namespace te
