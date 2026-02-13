/**
 * @file ResourceViewImpl.cpp
 * @brief Resource browser (024-Editor) - asset path tree, repository-based.
 */
#include <te/editor/ResourceView.h>
#include <te/editor/FileDialog.h>
#include <te/core/platform.h>
#include <te/resource/ResourceTypes.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceId.h>
#include <te/object/Guid.h>
#include <imgui.h>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <filesystem>

namespace te {
namespace editor {

namespace fs = std::filesystem;
using resource::ResourceType;
using resource::ResourceId;
using resource::IResourceManager;

static fs::path PathUtf8(std::string const& s) { return fs::u8path(s); }

static bool MatchesNameFilter(std::string const& name, char const* filterBuf) {
  if (!filterBuf || !filterBuf[0]) return true;
  std::string lowerName = name;
  std::string lowerFilter(filterBuf);
  std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
  std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);
  return lowerName.find(lowerFilter) != std::string::npos;
}

static ResourceType ImportTypeFromExtension(std::string const& ext) {
  if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp" || ext == ".hdr")
    return ResourceType::Texture;
  if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb")
    return ResourceType::Mesh;
  return ResourceType::_Count;
}

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

/** Return only folder segments (paths that have children or are in assetFolders). Resources (leaf paths) are not included. */
static void GetDirectChildAssetFoldersOnly(std::string const& parentPath, std::vector<std::string> const& allPaths, std::set<std::string> const& assetFoldersSet, std::vector<std::string>& outFolders) {
  outFolders.clear();
  std::set<std::string> seen;
  std::string prefix = parentPath.empty() ? "" : (parentPath + "/");
  for (std::string const& p : allPaths) {
    if (p.empty()) continue;
    std::string first;
    if (prefix.empty()) {
      size_t pos = p.find('/');
      first = pos == std::string::npos ? p : p.substr(0, pos);
    } else if (p.size() > prefix.size() && p.compare(0, prefix.size(), prefix) == 0) {
      std::string rest = p.substr(prefix.size());
      size_t pos = rest.find('/');
      first = pos == std::string::npos ? rest : rest.substr(0, pos);
    }
    if (first.empty() || seen.count(first)) continue;
    std::string fullPath = parentPath.empty() ? first : (parentPath + "/" + first);
    bool isFolder = (assetFoldersSet.count(fullPath) != 0);
    if (!isFolder) {
      std::string childPrefix = fullPath + "/";
      for (std::string const& p2 : allPaths)
        if (p2.size() > childPrefix.size() && p2.compare(0, childPrefix.size(), childPrefix) == 0) {
          isFolder = true;
          break;
        }
    }
    if (isFolder) {
      seen.insert(first);
      outFolders.push_back(first);
    }
  }
  std::sort(outFolders.begin(), outFolders.end());
}

class ResourceViewImpl : public IResourceView {
public:
  void OnDraw() override {
    if (m_rootPath.empty()) {
      ImGui::TextDisabled("No root path set");
      return;
    }

    float totalW = ImGui::GetContentRegionAvail().x;
    const char* filterLabels[] = {"All", "Texture", "Mesh", "Material", "Level"};
    int filterIdx = FilterTypeToComboIndex(m_filterType);
    ImGui::SetNextItemWidth(120.f);
    if (ImGui::Combo("##TypeFilter", &filterIdx, filterLabels, 5))
      m_filterType = ComboIndexToFilterType(filterIdx);
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

    DrawModals();

    if (m_importModalTriggerFileDialog) {
      m_importModalTriggerFileDialog = false;
      DoImportAfterFileDialog();
    }
  }

  void SetRootPath(char const* path) override { m_rootPath = path ? path : ""; }
  void SetOnOpenLevel(std::function<void(std::string const&)> fn) override { m_onOpenLevel = std::move(fn); }
  void SetResourceManager(te::resource::IResourceManager* manager) override { m_resourceManager = manager; }

  void ImportFiles(std::vector<std::string> const& paths) override {
    if (!m_resourceManager || paths.empty() || m_rootPath.empty()) return;
    m_resourceManager->LoadAllManifests();
    std::string repo = m_selectedRepo.empty() ? "main" : m_selectedRepo;
    std::vector<std::string> repos;
    m_resourceManager->GetRepositoryList(repos);
    if (repos.empty()) return;
    if (repo.empty()) repo = repos[0];
    bool inList = false;
    for (std::string const& r : repos) { if (r == repo) { inList = true; break; } }
    if (!inList) repo = repos[0];
    for (std::string const& path : paths) {
      if (path.empty()) continue;
      ResourceType type = ImportTypeFromExtension(te::core::PathGetExtension(path));
      if (type == ResourceType::_Count) continue;
      m_resourceManager->ImportIntoRepository(path.c_str(), type, repo.c_str(), m_selectedAssetPath.c_str(), nullptr);
    }
    m_resourceManager->LoadAllManifests();
  }

private:
  std::string GetFirstRepoForAssetPath(std::string const& assetPath, std::vector<IResourceManager::ResourceInfo> const& infos, std::vector<std::string> const& repos) const {
    for (auto const& i : infos) {
      bool inPath = assetPath.empty()
        ? (i.assetPath.find('/') == std::string::npos)
        : (i.assetPath == assetPath || (i.assetPath.size() > assetPath.size() && i.assetPath.compare(0, assetPath.size(), assetPath) == 0 && i.assetPath[assetPath.size()] == '/'));
      if (inPath) return i.repository;
    }
    for (std::string const& repo : repos) {
      std::vector<std::string> assetFolders;
      m_resourceManager->GetAssetFoldersForRepository(repo.c_str(), assetFolders);
      for (std::string const& folder : assetFolders)
        if (folder == assetPath || (assetPath.size() < folder.size() && folder.compare(0, assetPath.size(), assetPath) == 0 && folder[assetPath.size()] == '/'))
          return repo;
    }
    return repos.empty() ? "main" : repos[0];
  }

  void DrawLeftPanel() {
    ImGui::Text("Asset Path");
    ImGui::Separator();
    ImGui::BeginChild("DirTree", ImVec2(-1, -1), true);

    if (m_resourceManager) {
      std::vector<std::string> repos;
      m_resourceManager->GetRepositoryList(repos);
      std::vector<IResourceManager::ResourceInfo> infos;
      m_resourceManager->GetResourceInfos(infos);

      std::set<std::string> allPathsSet;
      std::set<std::string> assetFoldersSet;
      for (std::string const& repo : repos) {
        std::vector<std::string> assetFolders;
        m_resourceManager->GetAssetFoldersForRepository(repo.c_str(), assetFolders);
        for (std::string const& folder : assetFolders) assetFoldersSet.insert(folder);
        for (auto const& i : infos)
          if (i.repository == repo && !i.assetPath.empty()) allPathsSet.insert(i.assetPath);
      }
      for (std::string const& folder : assetFoldersSet) allPathsSet.insert(folder);
      std::vector<std::string> pathVec(allPathsSet.begin(), allPathsSet.end());

      ImGuiPayload const* dragPayload = ImGui::GetDragDropPayload();
      bool draggingResource = (dragPayload != nullptr && dragPayload->DataType != nullptr && std::strcmp(dragPayload->DataType, "ResourceAssetPath") == 0);
      if (draggingResource) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
        (void)ImGui::Selectable("(root) <- drop here", false, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(-1, 0));
        ImGui::PopStyleColor();
        if (ImGui::BeginDragDropTarget()) {
          if (ImGui::AcceptDragDropPayload("ResourceAssetPath")) {
            ImGuiPayload const* pl = ImGui::GetDragDropPayload();
            if (pl && pl->Data && pl->DataSize >= 16) {
              ResourceId guid;
              std::memcpy(guid.data, pl->Data, 16);
              if (!guid.IsNull()) {
                m_resourceManager->UpdateAssetPath(guid, "");
                m_resourceManager->LoadAllManifests();
              }
            }
          }
          ImGui::EndDragDropTarget();
        }
      }
      ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;
      if (m_selectedAssetPath.empty()) rootFlags |= ImGuiTreeNodeFlags_Selected;
      bool rootOpen = ImGui::TreeNodeEx("##vproot", rootFlags, "(root)");
      if (ImGui::IsItemClicked()) {
        m_showLevels = false;
        m_selectedAssetPath.clear();
        m_selectedRepo = GetFirstRepoForAssetPath("", infos, repos);
        m_selectedResourceGuid = ResourceId();
        m_cachedDetailValid = false;
      }
      if (ImGui::BeginDragDropTarget()) {
        if (ImGui::AcceptDragDropPayload("ResourceAssetPath")) {
          ImGuiPayload const* pl = ImGui::GetDragDropPayload();
          if (pl && pl->Data && pl->DataSize >= 16) {
            ResourceId guid;
            std::memcpy(guid.data, pl->Data, 16);
            if (!guid.IsNull()) {
              m_resourceManager->UpdateAssetPath(guid, "");
              m_resourceManager->LoadAllManifests();
            }
          }
        }
        if (ImGui::AcceptDragDropPayload("AssetFolderPath")) {
          ImGuiPayload const* pl = ImGui::GetDragDropPayload();
          if (pl && pl->Data && pl->DataSize > 0) {
            char buf[256];
            size_t sz = (std::min)(static_cast<size_t>(pl->DataSize), sizeof(buf) - 1);
            std::memcpy(buf, pl->Data, sz);
            buf[sz] = '\0';
            if (buf[0]) {
              m_resourceManager->MoveAssetFolder(buf, "");
              m_resourceManager->LoadAllManifests();
            }
          }
        }
        ImGui::EndDragDropTarget();
      }
      if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Import...")) { m_resourceManager->LoadAllManifests(); m_selectedAssetPath.clear(); m_selectedRepo = GetFirstRepoForAssetPath("", infos, repos); m_showImportModal = true; m_importModalRepo = m_selectedRepo; m_importModalAssetPath.clear(); ImGui::CloseCurrentPopup(); }
        if (ImGui::MenuItem("New Folder")) { m_selectedRepo = GetFirstRepoForAssetPath("", infos, repos); m_showNewFolder = true; m_newFolderParent = ""; m_newFolderBuf[0] = '\0'; ImGui::CloseCurrentPopup(); }
        if (ImGui::MenuItem("Create Repository")) { m_showCreateRepo = true; m_createRepoBuf[0] = '\0'; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
      }
      if (rootOpen) ImGui::TreePop();

      std::vector<std::string> topFolders;
      GetDirectChildAssetFoldersOnly("", pathVec, assetFoldersSet, topFolders);
      for (std::string const& f : topFolders)
        DrawAssetPathNode("", f, pathVec, assetFoldersSet, infos, repos);
    }

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1)) {
      ImGui::OpenPopup("TreeBackgroundContext");
    }
    if (ImGui::BeginPopup("TreeBackgroundContext")) {
      if (ImGui::MenuItem("Create Repository")) {
        m_showCreateRepo = true;
        m_createRepoBuf[0] = '\0';
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    ImGui::Text("Levels");
    ImGuiTreeNodeFlags levelFlags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (m_showLevels) levelFlags |= ImGuiTreeNodeFlags_Selected;
    if (ImGui::TreeNodeEx("##levels", levelFlags, "Levels")) {
      if (ImGui::IsItemClicked()) { m_showLevels = true; m_selectedRepo.clear(); m_selectedAssetPath.clear(); m_selectedResourceGuid = ResourceId(); m_cachedDetailValid = false; }
      ImGui::TreePop();
    }
    ImGui::EndChild();
  }

  void DrawAssetPathNode(std::string const& parentFullPath, std::string const& segment, std::vector<std::string> const& allPaths, std::set<std::string> const& assetFoldersSet, std::vector<IResourceManager::ResourceInfo> const& infos, std::vector<std::string> const& repos) {
    std::string nodeFullPath = parentFullPath.empty() ? segment : (parentFullPath + "/" + segment);

    std::vector<std::string> childSegments;
    GetDirectChildAssetFoldersOnly(nodeFullPath, allPaths, assetFoldersSet, childSegments);
    bool hasChildren = !childSegments.empty();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;
    if (m_selectedAssetPath == nodeFullPath) flags |= ImGuiTreeNodeFlags_Selected;
    bool open = ImGui::TreeNodeEx(("ap:" + nodeFullPath).c_str(), flags, "%s", segment.c_str());
    if (ImGui::IsItemClicked()) {
      m_showLevels = false;
      m_selectedAssetPath = nodeFullPath;
      m_selectedRepo = GetFirstRepoForAssetPath(nodeFullPath, infos, repos);
      m_selectedResourceGuid = ResourceId();
      m_cachedDetailValid = false;
    }
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
      char folderPayload[256] = {};
      size_t len = (std::min)(nodeFullPath.size(), size_t(255));
      std::memcpy(folderPayload, nodeFullPath.c_str(), len);
      folderPayload[len] = '\0';
      ImGui::SetDragDropPayload("AssetFolderPath", folderPayload, len + 1);
      ImGui::Text("%s", segment.c_str());
      ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget()) {
      if (ImGui::AcceptDragDropPayload("ResourceAssetPath")) {
        ImGuiPayload const* pl = ImGui::GetDragDropPayload();
        if (pl && pl->Data && pl->DataSize >= 16) {
          ResourceId guid;
          std::memcpy(guid.data, pl->Data, 16);
          if (!guid.IsNull()) {
            m_resourceManager->UpdateAssetPath(guid, nodeFullPath.c_str());
            m_resourceManager->LoadAllManifests();
          }
        }
      }
      if (ImGui::AcceptDragDropPayload("AssetFolderPath")) {
        ImGuiPayload const* pl = ImGui::GetDragDropPayload();
        if (pl && pl->Data && pl->DataSize > 0) {
          char buf[256];
          size_t sz = (std::min)(static_cast<size_t>(pl->DataSize), sizeof(buf) - 1);
          std::memcpy(buf, pl->Data, sz);
          buf[sz] = '\0';
          if (buf[0] && std::string(buf) != nodeFullPath) {
            m_resourceManager->MoveAssetFolder(buf, nodeFullPath.c_str());
            m_resourceManager->LoadAllManifests();
          }
        }
      }
      ImGui::EndDragDropTarget();
    }
    if (ImGui::BeginPopupContextItem()) {
      if (ImGui::MenuItem("Import...")) { m_resourceManager->LoadAllManifests(); m_selectedAssetPath = nodeFullPath; m_selectedRepo = GetFirstRepoForAssetPath(nodeFullPath, infos, repos); m_showImportModal = true; m_importModalRepo = m_selectedRepo; m_importModalAssetPath = nodeFullPath; ImGui::CloseCurrentPopup(); }
      if (ImGui::MenuItem("New Folder")) { m_showNewFolder = true; m_newFolderParent = nodeFullPath; m_newFolderBuf[0] = '\0'; ImGui::CloseCurrentPopup(); }
      if (ImGui::MenuItem("Change Repository")) { m_resourceManager->LoadAllManifests(); m_changeRepoTargetAssetPath = nodeFullPath; m_changeRepoTargetIsFolder = true; m_showChangeRepo = true; m_changeRepoCombo = 0; ImGui::CloseCurrentPopup(); }
      if (ImGui::MenuItem("Move here")) { /* move asset path handled by drag */ ImGui::CloseCurrentPopup(); }
      ImGui::EndPopup();
    }
    if (open) {
      for (std::string const& ch : childSegments)
        DrawAssetPathNode(nodeFullPath, ch, allPaths, assetFoldersSet, infos, repos);
      ImGui::TreePop();
    }
  }

  void DrawCenterPanel() {
    if (m_showLevels) {
      ImGui::Text("Levels: %s", (m_rootPath + "/assets/levels").c_str());
      ImGui::Separator();
      std::string levelsDir = te::core::PathJoin(m_rootPath, "assets/levels");
      std::vector<std::string> entries;
      std::error_code ec;
      fs::directory_iterator it(PathUtf8(levelsDir), ec);
      if (!ec) {
        for (; it != fs::directory_iterator(); ++it) {
          std::string name = it->path().filename().u8string();
          if (name.size() >= 10 && name.compare(name.size() - 10, 10, ".level.xml") == 0)
            entries.push_back(name);
        }
      }
      std::sort(entries.begin(), entries.end());
      ImGui::BeginChild("FileList", ImVec2(-1, -1), true);
      for (std::string const& e : entries) {
        std::string fullPath = te::core::PathJoin(levelsDir, e);
        if (ImGui::Selectable(e.c_str(), false))
          m_selectedLevelPath = fullPath;
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && m_onOpenLevel)
          m_onOpenLevel(fullPath);
      }
      ImGui::EndChild();
      return;
    }

    ImGui::Text("Asset path: %s", m_selectedAssetPath.empty() ? "(root)" : m_selectedAssetPath.c_str());
    ImGui::Separator();

    std::vector<IResourceManager::ResourceInfo> infos;
    if (m_resourceManager) m_resourceManager->GetResourceInfos(infos);
    std::vector<IResourceManager::ResourceInfo> filtered;
    for (auto const& i : infos) {
      if (i.guid.IsNull()) continue;
      bool inFolder = false;
      if (m_selectedAssetPath.empty()) {
        size_t firstSlash = i.assetPath.find('/');
        size_t secondSlash = (firstSlash == std::string::npos) ? std::string::npos : i.assetPath.find('/', firstSlash + 1);
        inFolder = (secondSlash == std::string::npos);
      } else {
        if (i.assetPath == m_selectedAssetPath) inFolder = true;
        else if (i.assetPath.size() > m_selectedAssetPath.size() && i.assetPath[m_selectedAssetPath.size()] == '/' && i.assetPath.compare(0, m_selectedAssetPath.size(), m_selectedAssetPath) == 0 && i.assetPath.find('/', m_selectedAssetPath.size() + 1) == std::string::npos) inFolder = true;
      }
      if (!inFolder) continue;
      if (m_filterType >= 0 && static_cast<int>(i.type) != m_filterType) continue;
      if (!MatchesNameFilter(i.displayName, m_nameFilterBuf)) continue;
      filtered.push_back(i);
    }
    std::sort(filtered.begin(), filtered.end(), [](auto const& a, auto const& b) { return a.displayName < b.displayName; });

    ImGui::BeginChild("FileList", ImVec2(-1, -1), true);
    if (!m_resourceManager) {
      ImGui::TextDisabled("No resource manager. Open a project.");
    } else if (infos.empty()) {
      ImGui::TextDisabled("No resources. Import files or create a repository.");
    } else if (filtered.empty()) {
      ImGui::TextDisabled("No resources in this folder. Select (root) or a folder on the left.");
    }
    for (size_t idx = 0; idx < filtered.size(); ++idx) {
      auto const& e = filtered[idx];
      ImGui::PushID(static_cast<int>(idx));
      bool selected = (m_selectedResourceGuid == e.guid);
      if (ImGui::Selectable(e.displayName.empty() ? "(no name)" : e.displayName.c_str(), selected))
        m_selectedResourceGuid = e.guid;
      if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
        m_selectedResourceGuid = e.guid;
        m_cachedDetailInfo = e;
        m_cachedDetailValid = true;
      }
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("ResourceAssetPath", e.guid.data, 16);
        ImGui::Text("%s", e.displayName.empty() ? "(no name)" : e.displayName.c_str());
        ImGui::EndDragDropSource();
      }
      if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Change Repository")) {
          m_resourceManager->LoadAllManifests();
          m_changeRepoTargetGuid = e.guid;
          m_changeRepoTargetAssetPath.clear();
          m_changeRepoTargetIsFolder = false;
          m_showChangeRepo = true;
          m_changeRepoCombo = 0;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
      ImGui::PopID();
    }
    ImGui::EndChild();
  }

  void DrawRightPanel() {
    ImGui::Text("Details");
    ImGui::Separator();
    ImGui::BeginChild("DetailsContent", ImVec2(-1, -1), true);
    if (m_showLevels && !m_selectedLevelPath.empty()) {
      ImGui::Text("Path: %s", m_selectedLevelPath.c_str());
      ImGui::Text("Type: Level");
      ImGui::EndChild();
      return;
    }
    if (m_selectedResourceGuid.IsNull()) {
      ImGui::TextDisabled("Select a resource");
      ImGui::EndChild();
      return;
    }
    std::string selectedGuidStr = m_selectedResourceGuid.ToString();
    ImGui::Text("GUID: %s", selectedGuidStr.c_str());
    if (!m_resourceManager) {
      ImGui::TextDisabled("Repository: (no resource manager)");
      ImGui::EndChild();
      return;
    }
    std::vector<IResourceManager::ResourceInfo> infos;
    m_resourceManager->GetResourceInfos(infos);
    IResourceManager::ResourceInfo const* detail = nullptr;
    for (auto const& i : infos) {
      if (i.guid == m_selectedResourceGuid) { detail = &i; break; }
    }
    if (detail) {
      ImGui::Text("Name: %s", detail->displayName.empty() ? "(no name)" : detail->displayName.c_str());
      ImGui::Text("Asset path: %s", detail->assetPath.empty() ? "(root)" : detail->assetPath.c_str());
      char const* typeStr = "Other";
      if (detail->type == ResourceType::Texture) typeStr = "Texture";
      else if (detail->type == ResourceType::Mesh) typeStr = "Mesh";
      else if (detail->type == ResourceType::Material) typeStr = "Material";
      else if (detail->type == ResourceType::Level) typeStr = "Level";
      ImGui::Text("Type: %s", typeStr);
      ImGui::Text("Repository: %s", detail->repository.empty() ? "(default)" : detail->repository.c_str());
    } else {
      ImGui::TextDisabled("(not found in manifest)");
    }
    ImGui::EndChild();
  }

  void DoImportAfterFileDialog() {
    if (!m_resourceManager) return;
    std::string repo = m_importModalRepo.empty() ? (m_selectedRepo.empty() ? "main" : m_selectedRepo) : m_importModalRepo;
    if (repo.empty()) {
      std::vector<std::string> repos;
      m_resourceManager->GetRepositoryList(repos);
      if (!repos.empty()) repo = repos[0];
    }
    std::string vpath = m_importModalAssetPath;
    std::string initialDir = m_rootPath;
    auto savedCwd = fs::current_path();
    std::vector<std::string> paths = OpenFileDialogMulti(nullptr, nullptr, initialDir.c_str());
    fs::current_path(savedCwd);
    for (std::string const& path : paths) {
      if (path.empty()) continue;
      ResourceType type = ImportTypeFromExtension(te::core::PathGetExtension(path));
      if (type == ResourceType::_Count) continue;
      m_resourceManager->ImportIntoRepository(path.c_str(), type, repo.c_str(), vpath.c_str(), nullptr);
    }
    m_resourceManager->LoadAllManifests();
  }

  void DrawModals() {
    if (m_showCreateRepo && m_resourceManager) {
      if (!ImGui::IsPopupOpen("Create Repository"))
        ImGui::OpenPopup("Create Repository");
      if (ImGui::BeginPopupModal("Create Repository", &m_showCreateRepo, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Repository name:");
        ImGui::SetNextItemWidth(280.f);
        bool enter = ImGui::InputText("##RepoName", m_createRepoBuf, sizeof(m_createRepoBuf), ImGuiInputTextFlags_EnterReturnsTrue);
        if (m_createRepoFailed) {
          ImGui::TextDisabled("Failed. Open a project first; name must not contain / \\ : * ? \" < > |");
        }
        if (ImGui::Button("OK") || enter) {
          m_createRepoFailed = false;
          if (m_createRepoBuf[0] != '\0') {
            if (m_resourceManager->CreateRepository(m_createRepoBuf)) {
              m_resourceManager->LoadAllManifests();
              m_showCreateRepo = false;
              ImGui::CloseCurrentPopup();
            } else {
              m_createRepoFailed = true;
            }
          }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
          m_showCreateRepo = false;
          m_createRepoFailed = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }

    if (m_showNewFolder && m_resourceManager) {
      if (!ImGui::IsPopupOpen("New Folder"))
        ImGui::OpenPopup("New Folder");
      if (ImGui::BeginPopupModal("New Folder", &m_showNewFolder, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Folder name:");
        ImGui::SetNextItemWidth(280.f);
        bool enter = ImGui::InputText("##NewFolderName", m_newFolderBuf, sizeof(m_newFolderBuf), ImGuiInputTextFlags_EnterReturnsTrue);
        if (ImGui::Button("OK") || enter) {
          if (m_newFolderBuf[0] != '\0') {
            std::string vpath = m_newFolderParent.empty() ? m_newFolderBuf : (m_newFolderParent + "/" + m_newFolderBuf);
            if (m_resourceManager->AddAssetFolder(m_selectedRepo.c_str(), vpath.c_str())) {
              m_showNewFolder = false;
              ImGui::CloseCurrentPopup();
            }
          }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
          m_showNewFolder = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }

    if (m_showImportModal && m_resourceManager) {
      if (!ImGui::IsPopupOpen("Import"))
        ImGui::OpenPopup("Import");
      if (ImGui::BeginPopupModal("Import", &m_showImportModal, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::vector<std::string> repos;
        m_resourceManager->GetRepositoryList(repos);
        int repoIdx = 0;
        for (size_t i = 0; i < repos.size(); ++i) { if (repos[i] == m_importModalRepo) { repoIdx = static_cast<int>(i); break; } }
        if (m_importModalRepo.empty() && !repos.empty()) { m_importModalRepo = repos[0]; repoIdx = 0; }
        std::vector<const char*> repoPtrs;
        for (auto const& r : repos) repoPtrs.push_back(r.c_str());
        ImGui::Text("Import to repository:");
        ImGui::SetNextItemWidth(200.f);
        if (ImGui::Combo("##ImportModalRepo", &repoIdx, repoPtrs.data(), static_cast<int>(repos.size())) && repoIdx >= 0 && static_cast<size_t>(repoIdx) < repos.size())
          m_importModalRepo = repos[static_cast<size_t>(repoIdx)];
        ImGui::Text("Folder: %s", m_importModalAssetPath.empty() ? "(root)" : m_importModalAssetPath.c_str());
        if (ImGui::Button("Select Files")) {
          m_showImportModal = false;
          m_importModalTriggerFileDialog = true;
          ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
          m_showImportModal = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }

    if (m_showChangeRepo && m_resourceManager) {
      if (!ImGui::IsPopupOpen("Change Repository"))
        ImGui::OpenPopup("Change Repository");
      if (ImGui::BeginPopupModal("Change Repository", &m_showChangeRepo, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::vector<std::string> repos;
        m_resourceManager->GetRepositoryList(repos);
        std::vector<const char*> repoPtrs;
        for (auto const& r : repos) repoPtrs.push_back(r.c_str());
        int n = static_cast<int>(repos.size());
        ImGui::Text("Target repository:");
        ImGui::SetNextItemWidth(200.f);
        ImGui::Combo("##TargetRepo", &m_changeRepoCombo, repoPtrs.data(), n);
        if (ImGui::Button("OK")) {
          if (m_changeRepoCombo >= 0 && m_changeRepoCombo < n) {
            std::string targetRepo = repos[static_cast<size_t>(m_changeRepoCombo)];
            if (targetRepo == m_selectedRepo) { m_showChangeRepo = false; ImGui::CloseCurrentPopup(); ImGui::EndPopup(); return; }
            if (m_changeRepoTargetIsFolder) {
              std::vector<IResourceManager::ResourceInfo> infos;
              m_resourceManager->GetResourceInfos(infos);
              if (m_changeRepoTargetAssetPath.empty()) {
                for (auto const& i : infos)
                  if (i.repository == m_selectedRepo)
                    m_resourceManager->MoveResourceToRepository(i.guid, targetRepo.c_str());
              } else {
                std::string prefix = m_changeRepoTargetAssetPath + "/";
                for (auto const& i : infos) {
                  if (i.repository != m_selectedRepo) continue;
                  if (i.assetPath == m_changeRepoTargetAssetPath || (i.assetPath.size() > prefix.size() && i.assetPath.compare(0, prefix.size(), prefix) == 0))
                    m_resourceManager->MoveResourceToRepository(i.guid, targetRepo.c_str());
                }
              }
            } else
              m_resourceManager->MoveResourceToRepository(m_changeRepoTargetGuid, targetRepo.c_str());
            m_showChangeRepo = false;
            ImGui::CloseCurrentPopup();
          }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
          m_showChangeRepo = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }
  }

  std::string m_rootPath;
  int m_filterType = -1;
  char m_nameFilterBuf[256] = "";
  std::string m_selectedRepo;
  std::string m_selectedAssetPath;
  ResourceId m_selectedResourceGuid;
  IResourceManager::ResourceInfo m_cachedDetailInfo;
  bool m_cachedDetailValid = false;
  bool m_showLevels = false;
  std::string m_selectedLevelPath;
  std::function<void(std::string const&)> m_onOpenLevel;
  IResourceManager* m_resourceManager = nullptr;

  float m_leftRatio = 0.25f;
  float m_centerRatio = 0.45f;

  bool m_showCreateRepo = false;
  bool m_createRepoFailed = false;
  char m_createRepoBuf[256] = "";
  bool m_showNewFolder = false;
  std::string m_newFolderParent;
  char m_newFolderBuf[256] = "";
  bool m_showChangeRepo = false;
  int m_changeRepoCombo = 0;
  std::string m_changeRepoTargetAssetPath;
  bool m_changeRepoTargetIsFolder = false;
  ResourceId m_changeRepoTargetGuid;
  bool m_showImportModal = false;
  std::string m_importModalRepo;
  std::string m_importModalAssetPath;
  bool m_importModalTriggerFileDialog = false;
};

IResourceView* CreateResourceView() {
  return new ResourceViewImpl();
}

}  // namespace editor
}  // namespace te
