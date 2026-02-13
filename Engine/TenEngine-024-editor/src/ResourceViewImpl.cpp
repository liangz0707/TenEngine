/**
 * @file ResourceViewImpl.cpp
 * @brief Resource browser (024-Editor) - three-column layout.
 */
#include <te/editor/ResourceView.h>
#include <te/editor/FileDialog.h>
#include <te/core/platform.h>
#include <te/resource/ResourceTypes.h>
#include <te/resource/ResourceManager.h>
#include <imgui.h>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <filesystem>

namespace te {
namespace editor {

namespace fs = std::filesystem;
using resource::ResourceType;

/** One row in the resource list: one logical resource (e.g. one texture = .texture + .texdata). */
struct ResourceEntry {
  std::string displayName;   // base name or primary filename for display
  std::string primaryPath;   // path to primary file (e.g. .texture, .mesh)
  ResourceType type = ResourceType::Custom;
};

/** Primary extensions that define "one resource". Others (e.g. .texdata) are auxiliary. */
static ResourceType GetResourceTypeFromExtension(std::string const& ext) {
  if (ext == ".texture") return ResourceType::Texture;
  if (ext == ".mesh") return ResourceType::Mesh;
  if (ext == ".material") return ResourceType::Material;
  if (ext == ".level.xml" || ext == ".level") return ResourceType::Level;
  return ResourceType::Custom;
}

static bool IsPrimaryResourceExtension(std::string const& ext) {
  return GetResourceTypeFromExtension(ext) != ResourceType::Custom;
}

/** Get resource extension for display/filter; .level.xml is treated as one extension. */
static std::string GetResourceExtension(std::string const& name) {
  if (name.size() >= 10 && name.compare(name.size() - 10, 10, ".level.xml") == 0)
    return ".level.xml";
  return te::core::PathGetExtension(name);
}

/** Collect one entry per resource; for Level prefer .level.xml over .level when both exist. */
static void CollectPrimaryResources(std::string const& dirPath, std::vector<ResourceEntry>& out) {
  std::error_code ec;
  fs::directory_iterator it(dirPath, ec);
  if (ec) return;
  std::map<std::string, std::string> levelPaths;  // base -> path (prefer .level.xml)
  for (; it != fs::directory_iterator(); ++it) {
    if (it->is_directory(ec)) continue;
    std::string name = it->path().filename().generic_string();
    if (name.empty() || name[0] == '.') continue;
    std::string ext = GetResourceExtension(name);
    if (!IsPrimaryResourceExtension(ext)) continue;
    std::string fullPath = te::core::PathJoin(dirPath, name);
    std::string base = name.substr(0, name.size() - ext.size());
    if (ext == ".level.xml") {
      levelPaths[base] = fullPath;
    } else if (ext == ".level") {
      if (levelPaths.find(base) == levelPaths.end())
        levelPaths[base] = fullPath;
    } else {
      ResourceEntry e;
      e.displayName = base.empty() ? name : base;
      e.primaryPath = fullPath;
      e.type = GetResourceTypeFromExtension(ext);
      out.push_back(e);
    }
  }
  for (auto const& p : levelPaths) {
    ResourceEntry e;
    e.displayName = p.first;
    e.primaryPath = p.second;
    e.type = ResourceType::Level;
    out.push_back(e);
  }
}

static bool MatchesNameFilter(std::string const& name, char const* filterBuf) {
  if (!filterBuf || !filterBuf[0]) return true;
  std::string lowerName = name;
  std::string lowerFilter(filterBuf);
  std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
  std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);
  return lowerName.find(lowerFilter) != std::string::npos;
}

/** Infer import ResourceType from source file extension. Returns _Count if not importable. */
static ResourceType ImportTypeFromExtension(std::string const& ext) {
  if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp" || ext == ".hdr")
    return ResourceType::Texture;
  if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb")
    return ResourceType::Mesh;
  return ResourceType::_Count;
}

/** Combo index 0 = All, 1 = Texture, 2 = Mesh, 3 = Material, 4 = Level. */
static int FilterTypeToComboIndex(int filterType) {
  if (filterType < 0) return 0;
  if (filterType == static_cast<int>(ResourceType::Level)) return 4;
  if (filterType >= 0 && filterType <= 2) return filterType + 1;
  return 0;
}
static int ComboIndexToFilterType(int comboIdx) {
  if (comboIdx <= 0) return -1;
  if (comboIdx == 4) return static_cast<int>(ResourceType::Level);
  if (comboIdx >= 1 && comboIdx <= 3) return comboIdx - 1;
  return -1;
}

class ResourceViewImpl : public IResourceView {
public:
  void OnDraw() override {
    if (m_rootPath.empty()) {
      ImGui::TextDisabled("No root path set");
      return;
    }

    float totalW = ImGui::GetContentRegionAvail().x;

    // Top row: Filter by resource type + Name filter
    const char* filterLabels[] = {"All", "Texture", "Mesh", "Material", "Level"};
    const int filterCount = 5;
    int filterIdx = FilterTypeToComboIndex(m_filterType);
    ImGui::SetNextItemWidth(120.f);
    if (ImGui::Combo("##TypeFilter", &filterIdx, filterLabels, filterCount)) {
      m_filterType = ComboIndexToFilterType(filterIdx);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##NameFilter", m_nameFilterBuf, sizeof(m_nameFilterBuf));

    const float splitterW = 4.f;
    const float minLeft = 60.f, minCenter = 80.f, minRight = 60.f;
    if (m_leftRatio <= 0.f || m_centerRatio <= 0.f) {
      m_leftRatio = 0.25f;
      m_centerRatio = 0.45f;
    }
    float leftW = totalW * m_leftRatio;
    float centerW = totalW * m_centerRatio;
    float rightW = totalW - leftW - centerW - splitterW * 2.f;
    if (rightW < minRight) {
      centerW = totalW - leftW - minRight - splitterW * 2.f;
      if (centerW < minCenter) {
        leftW = totalW - minCenter - minRight - splitterW * 2.f;
        centerW = minCenter;
      }
      rightW = totalW - leftW - centerW - splitterW * 2.f;
      m_leftRatio = leftW / totalW;
      m_centerRatio = centerW / totalW;
    }

    ImGui::BeginChild("ResourceLeft", ImVec2(leftW, -1), true);
    DrawLeftPanel();
    ImGui::EndChild();

    ImGui::SameLine();
    if (ImGui::InvisibleButton("ResSplit1", ImVec2(splitterW, -1))) {}
    if (ImGui::IsItemActive()) {
      float mx = ImGui::GetIO().MousePos.x;
      ImVec2 winMin = ImGui::GetWindowPos();
      float newLeft = (mx - winMin.x) - ImGui::GetStyle().WindowPadding.x;
      if (totalW > 0.f && newLeft >= minLeft && newLeft <= totalW - splitterW * 2.f - minCenter - minRight)
        m_leftRatio = newLeft / totalW;
    }
    ImGui::SameLine();
    ImGui::BeginChild("ResourceCenter", ImVec2(centerW, -1), true);
    DrawCenterPanel();
    ImGui::EndChild();

    ImGui::SameLine();
    if (ImGui::InvisibleButton("ResSplit2", ImVec2(splitterW, -1))) {}
    if (ImGui::IsItemActive()) {
      float mx = ImGui::GetIO().MousePos.x;
      float pad = ImGui::GetStyle().WindowPadding.x;
      float contentStart = ImGui::GetWindowPos().x + pad;
      float newCenterW = mx - contentStart - leftW - splitterW;
      if (totalW > 0.f && newCenterW >= minCenter && newCenterW <= totalW - leftW - splitterW * 2.f - minRight)
        m_centerRatio = newCenterW / totalW;
    }
    ImGui::SameLine();
    ImGui::BeginChild("ResourceRight", ImVec2(rightW, -1), true);
    DrawRightPanel();
    ImGui::EndChild();
  }

  void SetRootPath(char const* path) override { m_rootPath = path ? path : ""; }

  void SetOnOpenLevel(std::function<void(std::string const&)> fn) override { m_onOpenLevel = std::move(fn); }

  void SetResourceManager(te::resource::IResourceManager* manager) override { m_resourceManager = manager; }

  void ImportFiles(std::vector<std::string> const& paths) override {
    if (!m_resourceManager || paths.empty()) return;
    std::string targetDir = m_selectedDir.empty() ? m_rootPath : m_selectedDir;
    for (std::string const& path : paths) {
      if (path.empty()) continue;
      std::string ext = te::core::PathGetExtension(path);
      ResourceType type = ImportTypeFromExtension(ext);
      if (type == ResourceType::_Count) continue;
      std::string destPath = path;
      std::string sourceDir = te::core::PathGetDirectory(path);
      if (te::core::PathNormalize(sourceDir) != te::core::PathNormalize(targetDir)) {
        destPath = te::core::PathJoin(targetDir, te::core::PathGetFileName(path));
        std::error_code ec;
        fs::copy_file(path, destPath, fs::copy_options::overwrite_existing, ec);
        if (ec) continue;
      }
      m_resourceManager->Import(destPath.c_str(), type, nullptr);
    }
  }

private:
  void DoNewFolder(std::string const& parentPath) {
    std::string base = te::core::PathJoin(parentPath, "NewFolder");
    std::string path = base;
    int n = 0;
    std::error_code ec;
    while (fs::exists(path, ec)) {
      ++n;
      path = base + std::to_string(n);
    }
    fs::create_directories(path, ec);
  }

  bool IsDescendant(std::string const& parent, std::string const& child) {
    if (child.size() < parent.size()) return false;
    std::string p = fs::path(parent).lexically_normal().generic_string();
    std::string c = fs::path(child).lexically_normal().generic_string();
    if (p.size() >= c.size()) return false;
    return c.compare(0, p.size(), p) == 0 && (c[p.size()] == '/' || p.empty());
  }

  void DoImport() {
    if (!m_resourceManager) return;
    std::string targetDir = m_selectedDir.empty() ? m_rootPath : m_selectedDir;
    std::vector<std::string> paths = OpenFileDialogMulti(nullptr, nullptr);
    for (std::string const& path : paths) {
      if (path.empty()) continue;
      std::string ext = te::core::PathGetExtension(path);
      ResourceType type = ImportTypeFromExtension(ext);
      if (type == ResourceType::_Count) continue;
      std::string destPath = path;
      std::string sourceDir = te::core::PathGetDirectory(path);
      if (te::core::PathNormalize(sourceDir) != te::core::PathNormalize(targetDir)) {
        destPath = te::core::PathJoin(targetDir, te::core::PathGetFileName(path));
        std::error_code ec;
        fs::copy_file(path, destPath, fs::copy_options::overwrite_existing, ec);
        if (ec) continue;
      }
      m_resourceManager->Import(destPath.c_str(), type, nullptr);
    }
  }

  void DoDeleteDir(std::string const& dirPath) {
    if (dirPath.empty()) return;
    std::string canon = fs::path(dirPath).lexically_normal().generic_string();
    if (canon == fs::path(m_rootPath).lexically_normal().generic_string()) return;
    std::error_code ec;
    bool empty = true;
    for (fs::directory_iterator it(dirPath, ec); it != fs::directory_iterator(); ++it) {
      empty = false;
      break;
    }
    if (!empty) {
      m_pendingDeleteDir = dirPath;
      ImGui::OpenPopup("Delete Folder");
      return;
    }
    fs::remove_all(dirPath, ec);
    if (m_selectedDir == dirPath || IsDescendant(dirPath, m_selectedDir))
      m_selectedDir.clear();
  }

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
      if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("New Folder")) {
          DoNewFolder(m_rootPath);
          ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Import...")) {
          DoImport();
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
      DrawDirTree(m_rootPath, "");
      ImGui::TreePop();
    } else if (ImGui::IsItemClicked()) {
      m_selectedDir = "";
    }
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1)) {
      ImGui::OpenPopup("DirTreeContextMenu");
    }
    if (ImGui::BeginPopup("DirTreeContextMenu")) {
      std::string parent = m_selectedDir.empty() ? m_rootPath : m_selectedDir;
      if (ImGui::MenuItem("New Folder")) {
        DoNewFolder(parent);
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::MenuItem("Import...")) {
        DoImport();
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
    if (!m_pendingDeleteDir.empty() && !ImGui::IsPopupOpen("Delete Folder"))
      ImGui::OpenPopup("Delete Folder");
    if (ImGui::BeginPopupModal("Delete Folder", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Delete non-empty folder?\n%s", m_pendingDeleteDir.c_str());
      if (ImGui::Button("Delete")) {
        fs::remove_all(m_pendingDeleteDir);
        if (m_selectedDir == m_pendingDeleteDir || IsDescendant(m_pendingDeleteDir, m_selectedDir))
          m_selectedDir.clear();
        m_pendingDeleteDir.clear();
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) {
        m_pendingDeleteDir.clear();
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
    ImGui::EndChild();
  }

  void DrawDirTree(std::string const& fullPath, std::string const& displayPath) {
    std::error_code ec;
    fs::directory_iterator it(fullPath, ec);
    if (ec) return;

    std::vector<std::string> dirs;
    for (; it != fs::directory_iterator(); ++it) {
      std::string name = it->path().filename().generic_string();
      if (name.empty() || name[0] == '.') continue;
      if (it->is_directory(ec))
        dirs.push_back(name);
    }
    std::sort(dirs.begin(), dirs.end());

    for (std::string const& d : dirs) {
      std::string childPath = te::core::PathJoin(fullPath, d);
      std::string childDisplay = displayPath.empty() ? d : displayPath + "/" + d;
      ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
      if (m_selectedDir == childPath) flags |= ImGuiTreeNodeFlags_Selected;
      bool open = ImGui::TreeNodeEx(("dir:" + childPath).c_str(), flags, "%s", d.c_str());

      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("ResourceDir", childPath.c_str(),
                                  static_cast<int>(childPath.size() + 1));
        ImGui::Text("%s", d.c_str());
        ImGui::EndDragDropSource();
      }
      if (ImGui::BeginDragDropTarget()) {
        if (ImGui::AcceptDragDropPayload("ResourceDir")) {
          const char* payload = static_cast<const char*>(ImGui::GetDragDropPayload()->Data);
          std::string sourcePath(payload);
          if (sourcePath != childPath && !IsDescendant(sourcePath, childPath)) {
            std::string destPath = te::core::PathJoin(childPath, te::core::PathGetFileName(sourcePath));
            if (destPath != sourcePath) {
              std::error_code renec;
              fs::rename(sourcePath, destPath, renec);
            }
          }
        }
        ImGui::EndDragDropTarget();
      }
      if (ImGui::IsItemClicked()) m_selectedDir = childPath;
      if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("New Folder")) {
          DoNewFolder(childPath);
          ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Import...")) {
          DoImport();
          ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Delete")) {
          DoDeleteDir(childPath);
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }

      if (open) {
        DrawDirTree(childPath, childDisplay);
        ImGui::TreePop();
      }
    }
  }

  void DrawCenterPanel() {
    std::string viewPath = m_selectedDir.empty() ? m_rootPath : m_selectedDir;
    ImGui::Text("Resources: %s", viewPath.c_str());
    ImGui::Separator();

    std::vector<ResourceEntry> entries;
    CollectPrimaryResources(viewPath, entries);
    if (m_filterType >= 0) {
      ResourceType ft = static_cast<ResourceType>(m_filterType);
      entries.erase(std::remove_if(entries.begin(), entries.end(),
                                   [ft](ResourceEntry const& e) { return e.type != ft; }),
                    entries.end());
    }
    entries.erase(std::remove_if(entries.begin(), entries.end(),
                                 [this](ResourceEntry const& e) {
                                   return !MatchesNameFilter(e.displayName, m_nameFilterBuf);
                                 }),
                  entries.end());
    std::sort(entries.begin(), entries.end(),
              [](ResourceEntry const& a, ResourceEntry const& b) {
                return a.displayName < b.displayName;
              });

    ImGui::BeginChild("FileList", ImVec2(-1, -1), true);
    for (ResourceEntry const& e : entries) {
      bool selected = (m_selectedFile == e.primaryPath);
      if (ImGui::Selectable(e.displayName.c_str(), selected)) {
        m_selectedFile = e.primaryPath;
      }
      if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        if (e.type == ResourceType::Level && m_onOpenLevel) {
          m_onOpenLevel(e.primaryPath);
        }
      }
    }
    ImGui::EndChild();
  }

  void DrawRightPanel() {
    ImGui::Text("Preview");
    ImGui::Separator();
    if (m_selectedFile.empty()) {
      ImGui::TextDisabled("Select a resource");
      return;
    }
    std::string name = te::core::PathGetFileName(m_selectedFile);
    std::string ext = GetResourceExtension(name);
    const char* typeStr = "Unknown";
    switch (GetResourceTypeFromExtension(ext)) {
      case ResourceType::Texture: typeStr = "Texture"; break;
      case ResourceType::Mesh: typeStr = "Mesh"; break;
      case ResourceType::Material: typeStr = "Material"; break;
      case ResourceType::Level: typeStr = "Level"; break;
      default: break;
    }
    ImGui::Text("Name: %s", name.c_str());
    ImGui::Text("Path: %s", m_selectedFile.c_str());
    ImGui::Text("Type: %s", typeStr);
    std::size_t size = te::core::FileGetSize(m_selectedFile);
    ImGui::Text("Size: %zu bytes", size);
  }

  std::string m_rootPath;
  int m_filterType = -1;  // -1 = All, else ResourceType
  char m_nameFilterBuf[256] = "";
  std::string m_selectedDir;
  std::string m_selectedFile;
  std::function<void(std::string const&)> m_onOpenLevel;
  te::resource::IResourceManager* m_resourceManager = nullptr;

  float m_leftRatio = 0.25f;
  float m_centerRatio = 0.45f;
  std::string m_pendingDeleteDir;
};

IResourceView* CreateResourceView() {
  return new ResourceViewImpl();
}

}  // namespace editor
}  // namespace te
