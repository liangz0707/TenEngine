/**
 * @file ResourceViewImpl.cpp
 * @brief Resource browser (024-Editor) - three-column layout.
 */
#include <te/editor/ResourceView.h>
#include <te/core/platform.h>
#include <imgui.h>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>

namespace te {
namespace editor {

namespace fs = std::filesystem;

static bool MatchesFilter(char const* filterExt, std::string const& ext) {
  if (!filterExt || !filterExt[0]) return true;
  if (ext.empty()) return false;
  return ext == filterExt;
}

static bool MatchesNameFilter(std::string const& name, std::string const& filter) {
  if (filter.empty()) return true;
  std::string lowerName = name;
  std::string lowerFilter = filter;
  std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
  std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);
  return lowerName.find(lowerFilter) != std::string::npos;
}

class ResourceViewImpl : public IResourceView {
public:
  void OnDraw() override {
    if (m_rootPath.empty()) {
      ImGui::TextDisabled("No root path set");
      return;
    }

    float totalW = ImGui::GetContentRegionAvail().x;

    // Top row: Filter dropdown + Name filter
    const char* filterLabels[] = {"All", ".level", ".mesh", ".material", ".texture", ".json"};
    const char* filterExts[] = {"", ".level", ".mesh", ".material", ".texture", ".json"};
    int filterIdx = 0;
    for (int i = 0; i < 6; ++i) {
      if (m_filterExt == filterExts[i]) { filterIdx = i; break; }
    }
    ImGui::SetNextItemWidth(120.f);
    if (ImGui::Combo("##TypeFilter", &filterIdx, filterLabels, 6)) {
      m_filterExt = filterExts[filterIdx];
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##NameFilter", m_nameFilterBuf, sizeof(m_nameFilterBuf));

    float leftW = (totalW > 10.f) ? totalW * 0.25f : 80.f;
    float centerW = (totalW > 10.f) ? totalW * 0.45f : 80.f;
    float rightW = (totalW > 10.f) ? totalW * 0.3f - 4 : 80.f;

    ImGui::BeginChild("ResourceLeft", ImVec2(leftW, -1), true);
    DrawLeftPanel();
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("ResourceCenter", ImVec2(centerW, -1), true);
    DrawCenterPanel();
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("ResourceRight", ImVec2(rightW, -1), true);
    DrawRightPanel();
    ImGui::EndChild();
  }

  void SetRootPath(char const* path) override { m_rootPath = path ? path : ""; }

  void SetOnOpenLevel(std::function<void(std::string const&)> fn) override { m_onOpenLevel = std::move(fn); }

private:
  void DrawLeftPanel() {
    ImGui::Text("Directory");
    ImGui::Separator();
    ImGui::BeginChild("DirTree", ImVec2(-1, -20), true);
    std::string rootName = te::core::PathGetFileName(m_rootPath);
    if (rootName.empty()) rootName = m_rootPath;
    ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
    if (m_selectedDir.empty()) rootFlags |= ImGuiTreeNodeFlags_Selected;
    if (ImGui::TreeNodeEx("##root", rootFlags, "%s", rootName.c_str())) {
      if (ImGui::IsItemClicked()) m_selectedDir = "";
      DrawDirTree(m_rootPath, "");
      ImGui::TreePop();
    } else if (ImGui::IsItemClicked()) {
      m_selectedDir = "";
    }
    ImGui::EndChild();
  }

  void DrawDirTree(std::string const& fullPath, std::string const& displayPath) {
    std::error_code ec;
    fs::directory_iterator it(fullPath, ec);
    if (ec) return;

    std::vector<std::string> dirs;
    std::vector<std::string> files;
    for (; it != fs::directory_iterator(); ++it) {
      std::string name = it->path().filename().generic_string();
      if (name.empty() || name[0] == '.') continue;
      if (it->is_directory(ec)) {
        dirs.push_back(name);
      } else {
        files.push_back(name);
      }
    }
    std::sort(dirs.begin(), dirs.end());
    std::sort(files.begin(), files.end());

    for (std::string const& d : dirs) {
      std::string childPath = te::core::PathJoin(fullPath, d);
      std::string childDisplay = displayPath.empty() ? d : displayPath + "/" + d;
      ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
      if (m_selectedDir == childPath) flags |= ImGuiTreeNodeFlags_Selected;
      if (ImGui::TreeNodeEx(("dir:" + childPath).c_str(), flags, "%s", d.c_str())) {
        if (ImGui::IsItemClicked()) m_selectedDir = childPath;
        DrawDirTree(childPath, childDisplay);
        ImGui::TreePop();
      } else if (ImGui::IsItemClicked()) {
        m_selectedDir = childPath;
      }
    }
  }

  void DrawCenterPanel() {
    std::string viewPath = m_selectedDir.empty() ? m_rootPath : m_selectedDir;
    ImGui::Text("Files: %s", viewPath.c_str());
    ImGui::Separator();

    std::vector<std::string> entries;
    std::error_code ec;
    fs::directory_iterator it(viewPath, ec);
    if (!ec) {
      for (; it != fs::directory_iterator(); ++it) {
        std::string name = it->path().filename().generic_string();
        if (name.empty() || name[0] == '.') continue;
        if (it->is_directory(ec)) continue;
        std::string ext = te::core::PathGetExtension(name);
        if (!MatchesFilter(m_filterExt.c_str(), ext)) continue;
        if (!MatchesNameFilter(name, m_nameFilterBuf)) continue;
        entries.push_back(name);
      }
    }
    std::sort(entries.begin(), entries.end());

    ImGui::BeginChild("FileList", ImVec2(-1, -1), true);
    for (std::string const& name : entries) {
      std::string fullPath = te::core::PathJoin(viewPath, name);
      std::string ext = te::core::PathGetExtension(name);
      bool selected = (m_selectedFile == fullPath);
      if (ImGui::Selectable(name.c_str(), selected)) {
        m_selectedFile = fullPath;
      }
      if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        if (ext == ".level" && m_onOpenLevel) {
          m_onOpenLevel(fullPath);
        }
      }
    }
    ImGui::EndChild();
  }

  void DrawRightPanel() {
    ImGui::Text("Preview");
    ImGui::Separator();
    if (m_selectedFile.empty()) {
      ImGui::TextDisabled("Select a file");
      return;
    }
    std::string name = te::core::PathGetFileName(m_selectedFile);
    std::string ext = te::core::PathGetExtension(m_selectedFile);
    ImGui::Text("Name: %s", name.c_str());
    ImGui::Text("Path: %s", m_selectedFile.c_str());
    ImGui::Text("Type: %s", ext.empty() ? "(unknown)" : ext.c_str());
    std::size_t size = te::core::FileGetSize(m_selectedFile);
    ImGui::Text("Size: %zu bytes", size);
  }

  std::string m_rootPath;
  std::string m_filterExt;
  char m_nameFilterBuf[256] = "";
  std::string m_selectedDir;
  std::string m_selectedFile;
  std::function<void(std::string const&)> m_onOpenLevel;
};

IResourceView* CreateResourceView() {
  return new ResourceViewImpl();
}

}  // namespace editor
}  // namespace te
