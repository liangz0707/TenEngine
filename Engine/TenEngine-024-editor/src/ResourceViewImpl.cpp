/**
 * @file ResourceViewImpl.cpp
 * @brief Resource browser (024-Editor) - asset path tree, repository-based. Full rewrite.
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
#include <set>

namespace te {
namespace editor {

using resource::ResourceType;
using resource::ResourceId;
using resource::IResourceManager;

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------
static const char RESOURCE_GUID_PAYLOAD[] = "ResourceAssetPath";
static const char FOLDER_PATH_PAYLOAD[] = "AssetFolderPath";
static const char LEVELS_REL_DIR[] = "assets/levels";

// -----------------------------------------------------------------------------
// Helpers: Level scan (match Editor ScanLevelFiles)
// -----------------------------------------------------------------------------
static std::vector<std::string> ScanLevelFiles(char const* rootPath) {
  std::vector<std::string> out;
  if (!rootPath || !rootPath[0]) return out;
  std::string levelsDir = te::core::PathJoin(rootPath, LEVELS_REL_DIR);
  std::vector<te::core::DirEntry> entries = te::core::DirectoryEnumerate(levelsDir);
  for (std::string const& name : entries) {
    std::string ext = te::core::PathGetExtension(name);
    if (ext == ".level" || ext == ".xml") {
      out.push_back(te::core::PathJoin(levelsDir, name));
    } else if (ext == ".json" && name.find(".level.") != std::string::npos) {
      out.push_back(te::core::PathJoin(levelsDir, name));
    }
  }
  return out;
}

// -----------------------------------------------------------------------------
// Helpers: Path relation
// -----------------------------------------------------------------------------
static bool IsUnderFolder(std::string const& assetPath, std::string const& folderPath) {
  if (folderPath.empty()) {
    return assetPath.find('/') == std::string::npos;
  }
  if (assetPath == folderPath) return true;
  if (assetPath.size() <= folderPath.size()) return false;
  if (assetPath[folderPath.size()] != '/') return false;
  return assetPath.compare(0, folderPath.size(), folderPath) == 0;
}

/** Direct child folder segments under parentPath. A "folder" is either in assetFolderSet or has a child path in allAssetPaths. */
static void GetDirectChildFolderSegments(
  std::string const& parentPath,
  std::vector<std::string> const& allAssetPaths,
  std::set<std::string> const& assetFolderSet,
  std::vector<std::string>& outSegments) {
  outSegments.clear();
  std::set<std::string> seen;
  std::string prefix = parentPath.empty() ? "" : (parentPath + "/");
  for (std::string const& p : allAssetPaths) {
    if (p.empty()) continue;
    std::string segment;
    if (prefix.empty()) {
      size_t pos = p.find('/');
      segment = pos == std::string::npos ? p : p.substr(0, pos);
    } else if (p.size() > prefix.size() && p.compare(0, prefix.size(), prefix) == 0) {
      std::string rest = p.substr(prefix.size());
      size_t pos = rest.find('/');
      segment = pos == std::string::npos ? rest : rest.substr(0, pos);
    }
    if (segment.empty() || seen.count(segment)) continue;
    std::string fullPath = prefix.empty() ? segment : (parentPath + "/" + segment);
    bool isFolder = (assetFolderSet.count(fullPath) != 0);
    if (!isFolder) {
      std::string childPrefix = fullPath + "/";
      for (std::string const& p2 : allAssetPaths) {
        if (p2.size() > childPrefix.size() && p2.compare(0, childPrefix.size(), childPrefix) == 0) {
          isFolder = true;
          break;
        }
      }
    }
    if (isFolder) {
      seen.insert(segment);
      outSegments.push_back(segment);
    }
  }
  std::sort(outSegments.begin(), outSegments.end());
}

// -----------------------------------------------------------------------------
// Helpers: Resource filter
// -----------------------------------------------------------------------------
static bool MatchesNameFilter(std::string const& name, char const* filterBuf) {
  if (!filterBuf || !filterBuf[0]) return true;
  std::string lowerName = name;
  std::string lowerFilter(filterBuf);
  std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
  std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);
  return lowerName.find(lowerFilter) != std::string::npos;
}

/** Show only resources whose assetPath exactly equals folderPath (resource is in that folder, not in a subfolder). */
static std::vector<IResourceManager::ResourceInfo> FilterResourcesInFolder(
  std::vector<IResourceManager::ResourceInfo> const& infos,
  std::string const& folderPath,
  int typeFilter,
  char const* nameFilterBuf) {
  std::vector<IResourceManager::ResourceInfo> out;
  for (auto const& i : infos) {
    if (i.guid.IsNull()) continue;
    if (i.assetPath != folderPath) continue;
    if (typeFilter >= 0 && static_cast<int>(i.type) != typeFilter) continue;
    if (!MatchesNameFilter(i.displayName, nameFilterBuf)) continue;
    out.push_back(i);
  }
  std::sort(out.begin(), out.end(), [](auto const& a, auto const& b) { return a.displayName < b.displayName; });
  return out;
}

// -----------------------------------------------------------------------------
// Helpers: Import type and filter combo
// -----------------------------------------------------------------------------
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

/** Resolve which repo "owns" the given asset path for UI (selected repo when that folder is selected). */
static std::string GetRepoForAssetPath(
  std::string const& assetPath,
  std::vector<IResourceManager::ResourceInfo> const& infos,
  std::vector<std::string> const& repos,
  IResourceManager* mgr) {
  for (auto const& i : infos) {
    if (assetPath.empty()) {
      if (i.assetPath.find('/') == std::string::npos) return i.repository;
    } else {
      if (i.assetPath == assetPath || (i.assetPath.size() > assetPath.size() && i.assetPath[assetPath.size()] == '/' && i.assetPath.compare(0, assetPath.size(), assetPath) == 0))
        return i.repository;
    }
  }
  for (std::string const& repo : repos) {
    std::vector<std::string> folders;
    mgr->GetAssetFoldersForRepository(repo.c_str(), folders);
    for (std::string const& f : folders) {
      if (f == assetPath || (assetPath.size() < f.size() && f[assetPath.size()] == '/' && f.compare(0, assetPath.size(), assetPath) == 0))
        return repo;
    }
  }
  return repos.empty() ? "main" : repos[0];
}

// -----------------------------------------------------------------------------
// ResourceViewImpl
// -----------------------------------------------------------------------------
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

    if (m_pendingOpenFileDialog) {
      m_pendingOpenFileDialog = false;
      DoImportAfterFileDialog();
    }
  }

  void SetRootPath(char const* path) override { m_rootPath = path ? path : ""; }
  void SetOnOpenLevel(std::function<void(std::string const&)> fn) override { m_onOpenLevel = std::move(fn); }
  void SetOnDeleteLevel(std::function<void(std::string const&)> fn) override { m_onDeleteLevel = std::move(fn); }
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
  void GatherTreeData(
    std::vector<std::string>& reposOut,
    std::vector<IResourceManager::ResourceInfo>& infosOut,
    std::set<std::string>& allAssetPathsOut,
    std::set<std::string>& assetFoldersSetOut,
    std::vector<std::string>& pathVecOut) {
    reposOut.clear();
    infosOut.clear();
    allAssetPathsOut.clear();
    assetFoldersSetOut.clear();
    pathVecOut.clear();
    if (!m_resourceManager) return;
    m_resourceManager->GetRepositoryList(reposOut);
    m_resourceManager->GetResourceInfos(infosOut);
    for (std::string const& repo : reposOut) {
      std::vector<std::string> folders;
      m_resourceManager->GetAssetFoldersForRepository(repo.c_str(), folders);
      for (std::string const& f : folders) assetFoldersSetOut.insert(f);
    }
    for (auto const& i : infosOut)
      if (!i.assetPath.empty()) allAssetPathsOut.insert(i.assetPath);
    for (std::string const& f : assetFoldersSetOut) allAssetPathsOut.insert(f);
    pathVecOut.assign(allAssetPathsOut.begin(), allAssetPathsOut.end());
  }

  void DrawLeftPanel() {
    ImGui::Text("Asset Path");
    ImGui::Separator();
    ImGui::BeginChild("DirTree", ImVec2(-1, -1), true);

    std::vector<std::string> repos;
    std::vector<IResourceManager::ResourceInfo> infos;
    std::set<std::string> allAssetPathsSet;
    std::set<std::string> assetFoldersSet;
    std::vector<std::string> pathVec;
    GatherTreeData(repos, infos, allAssetPathsSet, assetFoldersSet, pathVec);

    if (m_resourceManager) {
      bool draggingResource = false;
      ImGuiPayload const* payload = ImGui::GetDragDropPayload();
      if (payload && payload->DataType && std::strcmp(payload->DataType, RESOURCE_GUID_PAYLOAD) == 0)
        draggingResource = true;
      if (draggingResource) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
        (void)ImGui::Selectable("(root) <- drop here", false, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(-1, 0));
        ImGui::PopStyleColor();
        if (ImGui::BeginDragDropTarget()) {
          if (ImGui::AcceptDragDropPayload(RESOURCE_GUID_PAYLOAD)) {
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
          if (ImGui::AcceptDragDropPayload(FOLDER_PATH_PAYLOAD)) {
            char buf[512];
            ImGuiPayload const* pl = ImGui::GetDragDropPayload();
            if (pl && pl->Data && pl->DataSize > 0) {
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
      }

      ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;
      if (m_selectedAssetPath.empty()) rootFlags |= ImGuiTreeNodeFlags_Selected;
      bool rootOpen = ImGui::TreeNodeEx("##root", rootFlags, "(root)");
      if (ImGui::IsItemClicked()) {
        m_showLevels = false;
        m_selectedAssetPath.clear();
        m_selectedRepo = GetRepoForAssetPath("", infos, repos, m_resourceManager);
        m_selectedResourceGuid = ResourceId();
      }
      if (ImGui::BeginDragDropTarget()) {
        if (ImGui::AcceptDragDropPayload(RESOURCE_GUID_PAYLOAD)) {
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
        if (ImGui::AcceptDragDropPayload(FOLDER_PATH_PAYLOAD)) {
          char buf[512];
          ImGuiPayload const* pl = ImGui::GetDragDropPayload();
          if (pl && pl->Data && pl->DataSize > 0) {
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
        if (ImGui::MenuItem("Import...")) {
          m_resourceManager->LoadAllManifests();
          m_selectedAssetPath.clear();
          m_selectedRepo = GetRepoForAssetPath("", infos, repos, m_resourceManager);
          m_showImportModal = true;
          m_importRepo = m_selectedRepo;
          m_importFolderPath.clear();
          ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("New Folder")) {
          m_selectedRepo = GetRepoForAssetPath("", infos, repos, m_resourceManager);
          m_showNewFolderModal = true;
          m_newFolderParentPath.clear();
          m_newFolderNameBuf[0] = '\0';
          ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Create Repository")) {
          m_showCreateRepoModal = true;
          m_createRepoNameBuf[0] = '\0';
          m_createRepoFailed = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
      if (rootOpen) {
        std::vector<std::string> topSegments;
        GetDirectChildFolderSegments("", pathVec, assetFoldersSet, topSegments);
        for (std::string const& seg : topSegments)
          DrawFolderNode("", seg, pathVec, assetFoldersSet, infos, repos);
        ImGui::TreePop();
      }
    }

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1))
      ImGui::OpenPopup("TreeBackgroundContext");
    if (ImGui::BeginPopup("TreeBackgroundContext")) {
      if (ImGui::MenuItem("Create Repository")) {
        m_showCreateRepoModal = true;
        m_createRepoNameBuf[0] = '\0';
        m_createRepoFailed = false;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    ImGui::Text("Levels");
    ImGuiTreeNodeFlags levelFlags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (m_showLevels) levelFlags |= ImGuiTreeNodeFlags_Selected;
    if (ImGui::TreeNodeEx("##levels", levelFlags, "Levels")) {
      if (ImGui::IsItemClicked()) {
        m_showLevels = true;
        m_selectedRepo.clear();
        m_selectedAssetPath.clear();
        m_selectedResourceGuid = ResourceId();
        m_selectedLevelPath.clear();
      }
      ImGui::TreePop();
    }
    ImGui::EndChild();
  }

  void DrawFolderNode(
    std::string const& parentPath,
    std::string const& segment,
    std::vector<std::string> const& pathVec,
    std::set<std::string> const& assetFoldersSet,
    std::vector<IResourceManager::ResourceInfo> const& infos,
    std::vector<std::string> const& repos) {
    std::string nodeFullPath = parentPath.empty() ? segment : (parentPath + "/" + segment);
    std::vector<std::string> childSegments;
    GetDirectChildFolderSegments(nodeFullPath, pathVec, assetFoldersSet, childSegments);
    bool hasChildren = !childSegments.empty();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;
    if (m_selectedAssetPath == nodeFullPath) flags |= ImGuiTreeNodeFlags_Selected;
    bool open = ImGui::TreeNodeEx(("ap:" + nodeFullPath).c_str(), flags, "%s", segment.c_str());
    if (ImGui::IsItemClicked()) {
      m_showLevels = false;
      m_selectedAssetPath = nodeFullPath;
      m_selectedRepo = GetRepoForAssetPath(nodeFullPath, infos, repos, m_resourceManager);
      m_selectedResourceGuid = ResourceId();
    }
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
      char pathBuf[512];
      size_t len = (std::min)(nodeFullPath.size(), size_t(511));
      std::memcpy(pathBuf, nodeFullPath.c_str(), len);
      pathBuf[len] = '\0';
      ImGui::SetDragDropPayload(FOLDER_PATH_PAYLOAD, pathBuf, len + 1);
      ImGui::Text("%s", segment.c_str());
      ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget()) {
      if (ImGui::AcceptDragDropPayload(RESOURCE_GUID_PAYLOAD)) {
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
      if (ImGui::AcceptDragDropPayload(FOLDER_PATH_PAYLOAD)) {
        char buf[512];
        ImGuiPayload const* pl = ImGui::GetDragDropPayload();
        if (pl && pl->Data && pl->DataSize > 0) {
          size_t sz = (std::min)(static_cast<size_t>(pl->DataSize), sizeof(buf) - 1);
          std::memcpy(buf, pl->Data, sz);
          buf[sz] = '\0';
          if (buf[0] && std::string(buf) != nodeFullPath) {
            std::string bufStr(buf);
            bool isDescendant = (nodeFullPath.size() > bufStr.size() + 1 &&
                                nodeFullPath.compare(0, bufStr.size(), bufStr) == 0 &&
                                nodeFullPath[bufStr.size()] == '/');
            if (!isDescendant) {
              m_resourceManager->MoveAssetFolder(buf, nodeFullPath.c_str());
              m_resourceManager->LoadAllManifests();
            }
          }
        }
      }
      ImGui::EndDragDropTarget();
    }
    if (ImGui::BeginPopupContextItem()) {
      if (ImGui::MenuItem("Import...")) {
        m_resourceManager->LoadAllManifests();
        m_selectedAssetPath = nodeFullPath;
        m_selectedRepo = GetRepoForAssetPath(nodeFullPath, infos, repos, m_resourceManager);
        m_showImportModal = true;
        m_importRepo = m_selectedRepo;
        m_importFolderPath = nodeFullPath;
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::MenuItem("New Folder")) {
        m_showNewFolderModal = true;
        m_newFolderParentPath = nodeFullPath;
        m_newFolderNameBuf[0] = '\0';
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::MenuItem("Delete Folder")) {
        m_showDeleteFolderModal = true;
        m_deleteFolderPath = nodeFullPath;
        m_deleteFolderRepo = GetRepoForAssetPath(nodeFullPath, infos, repos, m_resourceManager);
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::MenuItem("Change Repository")) {
        m_resourceManager->LoadAllManifests();
        m_changeRepoTargetPath = nodeFullPath;
        m_changeRepoIsFolder = true;
        m_showChangeRepoModal = true;
        m_changeRepoComboIdx = 0;
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::MenuItem("Move here")) { /* handled by drag */ ImGui::CloseCurrentPopup(); }
      ImGui::EndPopup();
    }
    if (open) {
      for (std::string const& ch : childSegments)
        DrawFolderNode(nodeFullPath, ch, pathVec, assetFoldersSet, infos, repos);
      ImGui::TreePop();
    }
  }

  void DrawCenterPanel() {
    if (m_showLevels) {
      ImGui::Text("Levels: %s", (m_rootPath + "/" + LEVELS_REL_DIR).c_str());
      ImGui::Separator();
      std::vector<std::string> levelPaths = ScanLevelFiles(m_rootPath.c_str());
      ImGui::BeginChild("FileList", ImVec2(-1, -1), true);
      const float tileW = 88.f;
      const float tileH = 82.f;
      const float iconSz = 56.f;
      float availW = ImGui::GetContentRegionAvail().x;
      int cols = (availW > tileW) ? static_cast<int>(availW / tileW) : 1;
      ImDrawList* dl = ImGui::GetWindowDrawList();
      for (size_t i = 0; i < levelPaths.size(); ++i) {
        if (i > 0 && (i % static_cast<size_t>(cols)) != 0) ImGui::SameLine();
        std::string const& fullPath = levelPaths[i];
        std::string name = te::core::PathGetFileName(fullPath);
        if (name.size() > 12u) name = name.substr(0, 9) + "...";
        bool selected = (m_selectedLevelPath == fullPath);
        ImGui::PushID(static_cast<int>(i));
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##lv", ImVec2(tileW, tileH));
        bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked(0)) m_selectedLevelPath = fullPath;
        if (hovered && ImGui::IsMouseDoubleClicked(0) && m_onOpenLevel) m_onOpenLevel(fullPath);
        ImVec2 iconMin(pos.x + (tileW - iconSz) * 0.5f, pos.y + 2.f);
        ImVec2 iconMax(iconMin.x + iconSz, iconMin.y + iconSz);
        ImU32 iconCol = selected ? IM_COL32(90, 90, 140, 255) : (hovered ? IM_COL32(70, 70, 100, 255) : IM_COL32(55, 55, 70, 255));
        dl->AddRectFilled(iconMin, iconMax, iconCol, 5.f);
        ImVec2 textSize = ImGui::CalcTextSize(name.c_str(), nullptr, false, tileW - 4.f);
        dl->AddText(ImVec2(pos.x + (tileW - textSize.x) * 0.5f, iconMax.y + 4.f), IM_COL32(255, 255, 255, 255), name.c_str());
        if (ImGui::BeginPopupContextItem()) {
          if (ImGui::MenuItem("Delete Level") && m_onDeleteLevel) m_onDeleteLevel(fullPath);
          ImGui::EndPopup();
        }
        ImGui::PopID();
      }
      ImGui::EndChild();
      return;
    }

    ImGui::Text("Asset path: %s", m_selectedAssetPath.empty() ? "(root)" : m_selectedAssetPath.c_str());
    ImGui::Separator();

    std::vector<std::string> repos;
    std::vector<IResourceManager::ResourceInfo> infos;
    std::set<std::string> allAssetPathsSet, assetFoldersSet;
    std::vector<std::string> pathVec;
    GatherTreeData(repos, infos, allAssetPathsSet, assetFoldersSet, pathVec);

    std::vector<std::string> childFolderSegments;
    GetDirectChildFolderSegments(m_selectedAssetPath, pathVec, assetFoldersSet, childFolderSegments);
    std::vector<IResourceManager::ResourceInfo> filtered = FilterResourcesInFolder(infos, m_selectedAssetPath, m_filterType, m_nameFilterBuf);

    ImGui::BeginChild("FileList", ImVec2(-1, -1), true);
    if (!m_resourceManager) {
      ImGui::TextDisabled("No resource manager. Open a project.");
    } else {
      const float tileW = 88.f;
      const float tileH = 82.f;
      const float iconSz = 56.f;
      float availW = ImGui::GetContentRegionAvail().x;
      int cols = (availW > tileW) ? static_cast<int>(availW / tileW) : 1;
      ImDrawList* dl = ImGui::GetWindowDrawList();
      int tileIndex = 0;
      for (std::string const& seg : childFolderSegments) {
        if (tileIndex > 0 && (tileIndex % cols) != 0) ImGui::SameLine();
        std::string fullPath = m_selectedAssetPath.empty() ? seg : (m_selectedAssetPath + "/" + seg);
        bool selected = (m_selectedAssetPath == fullPath);
        ImGui::PushID(("f:" + fullPath).c_str());
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##folder", ImVec2(tileW, tileH));
        bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked(0)) {
          m_showLevels = false;
          m_selectedAssetPath = fullPath;
          m_selectedRepo = GetRepoForAssetPath(fullPath, infos, repos, m_resourceManager);
          m_selectedResourceGuid = ResourceId();
        }
        ImVec2 iconMin(pos.x + (tileW - iconSz) * 0.5f, pos.y + 2.f);
        ImVec2 iconMax(iconMin.x + iconSz, iconMin.y + iconSz);
        ImU32 iconCol = selected ? IM_COL32(80, 120, 80, 255) : (hovered ? IM_COL32(60, 95, 60, 255) : IM_COL32(50, 75, 50, 255));
        dl->AddRectFilled(iconMin, iconMax, iconCol, 5.f);
        std::string disp = seg.size() > 12u ? seg.substr(0, 9) + "..." : seg;
        ImVec2 ts = ImGui::CalcTextSize(disp.c_str(), nullptr, false, tileW - 4.f);
        dl->AddText(ImVec2(pos.x + (tileW - ts.x) * 0.5f, iconMax.y + 4.f), IM_COL32(255, 255, 255, 255), disp.c_str());
        ImGui::PopID();
        ++tileIndex;
      }
      for (size_t idx = 0; idx < filtered.size(); ++idx) {
        auto const& e = filtered[idx];
        if (tileIndex > 0 && (tileIndex % cols) != 0) ImGui::SameLine();
        bool selected = (m_selectedResourceGuid == e.guid);
        ImGui::PushID(static_cast<int>(idx));
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##res", ImVec2(tileW, tileH));
        bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked(0)) m_selectedResourceGuid = e.guid;
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
          ImGui::SetDragDropPayload(RESOURCE_GUID_PAYLOAD, e.guid.data, 16);
          ImGui::Text("%s", e.displayName.empty() ? "(no name)" : e.displayName.c_str());
          ImGui::EndDragDropSource();
        }
        ImVec2 iconMin(pos.x + (tileW - iconSz) * 0.5f, pos.y + 2.f);
        ImVec2 iconMax(iconMin.x + iconSz, iconMin.y + iconSz);
        ImU32 iconCol = selected ? IM_COL32(90, 90, 140, 255) : (hovered ? IM_COL32(70, 70, 100, 255) : IM_COL32(55, 55, 70, 255));
        dl->AddRectFilled(iconMin, iconMax, iconCol, 5.f);
        std::string disp = e.displayName.empty() ? "(no name)" : e.displayName;
        if (disp.size() > 12u) disp = disp.substr(0, 9) + "...";
        ImVec2 ts = ImGui::CalcTextSize(disp.c_str(), nullptr, false, tileW - 4.f);
        dl->AddText(ImVec2(pos.x + (tileW - ts.x) * 0.5f, iconMax.y + 4.f), IM_COL32(255, 255, 255, 255), disp.c_str());
        if (ImGui::BeginPopupContextItem()) {
          if (ImGui::MenuItem("Change Repository")) {
            m_resourceManager->LoadAllManifests();
            m_changeRepoTargetGuid = e.guid;
            m_changeRepoTargetPath.clear();
            m_changeRepoIsFolder = false;
            m_showChangeRepoModal = true;
            m_changeRepoComboIdx = 0;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }
        ImGui::PopID();
        ++tileIndex;
      }
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
    ImGui::Text("GUID: %s", m_selectedResourceGuid.ToString().c_str());
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

  void DrawModals() {
    if (m_showCreateRepoModal && m_resourceManager) {
      if (!ImGui::IsPopupOpen("Create Repository"))
        ImGui::OpenPopup("Create Repository");
      if (ImGui::BeginPopupModal("Create Repository", &m_showCreateRepoModal, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Repository name:");
        ImGui::SetNextItemWidth(280.f);
        bool enter = ImGui::InputText("##RepoName", m_createRepoNameBuf, sizeof(m_createRepoNameBuf), ImGuiInputTextFlags_EnterReturnsTrue);
        if (m_createRepoFailed)
          ImGui::TextDisabled("Failed. Open a project first; name must not contain / \\ : * ? \" < > |");
        if (ImGui::Button("OK") || enter) {
          m_createRepoFailed = false;
          if (m_createRepoNameBuf[0] != '\0') {
            if (m_resourceManager->CreateRepository(m_createRepoNameBuf)) {
              m_resourceManager->LoadAllManifests();
              m_showCreateRepoModal = false;
              ImGui::CloseCurrentPopup();
            } else {
              m_createRepoFailed = true;
            }
          }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
          m_showCreateRepoModal = false;
          m_createRepoFailed = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }

    if (m_showNewFolderModal && m_resourceManager) {
      if (!ImGui::IsPopupOpen("New Folder"))
        ImGui::OpenPopup("New Folder");
      if (ImGui::BeginPopupModal("New Folder", &m_showNewFolderModal, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Folder name:");
        ImGui::SetNextItemWidth(280.f);
        bool enter = ImGui::InputText("##NewFolderName", m_newFolderNameBuf, sizeof(m_newFolderNameBuf), ImGuiInputTextFlags_EnterReturnsTrue);
        if (ImGui::Button("OK") || enter) {
          if (m_newFolderNameBuf[0] != '\0') {
            std::string fullPath = m_newFolderParentPath.empty() ? m_newFolderNameBuf : (m_newFolderParentPath + "/" + m_newFolderNameBuf);
            if (m_resourceManager->AddAssetFolder(m_selectedRepo.c_str(), fullPath.c_str())) {
              m_showNewFolderModal = false;
              ImGui::CloseCurrentPopup();
            }
          }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
          m_showNewFolderModal = false;
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
        for (size_t i = 0; i < repos.size(); ++i) { if (repos[i] == m_importRepo) { repoIdx = static_cast<int>(i); break; } }
        if (m_importRepo.empty() && !repos.empty()) { m_importRepo = repos[0]; repoIdx = 0; }
        std::vector<const char*> repoPtrs;
        for (auto const& r : repos) repoPtrs.push_back(r.c_str());
        ImGui::Text("Import to repository:");
        ImGui::SetNextItemWidth(200.f);
        if (ImGui::Combo("##ImportRepo", &repoIdx, repoPtrs.data(), static_cast<int>(repos.size())) && repoIdx >= 0 && static_cast<size_t>(repoIdx) < repos.size())
          m_importRepo = repos[static_cast<size_t>(repoIdx)];
        ImGui::Text("Folder: %s", m_importFolderPath.empty() ? "(root)" : m_importFolderPath.c_str());
        if (ImGui::Button("Select Files")) {
          m_showImportModal = false;
          m_pendingOpenFileDialog = true;
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

    if (m_showChangeRepoModal && m_resourceManager) {
      if (!ImGui::IsPopupOpen("Change Repository"))
        ImGui::OpenPopup("Change Repository");
      if (ImGui::BeginPopupModal("Change Repository", &m_showChangeRepoModal, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::vector<std::string> repos;
        m_resourceManager->GetRepositoryList(repos);
        std::vector<const char*> repoPtrs;
        for (auto const& r : repos) repoPtrs.push_back(r.c_str());
        int n = static_cast<int>(repos.size());
        ImGui::Text("Target repository:");
        ImGui::SetNextItemWidth(200.f);
        ImGui::Combo("##TargetRepo", &m_changeRepoComboIdx, repoPtrs.data(), n);
        if (ImGui::Button("OK")) {
          if (m_changeRepoComboIdx >= 0 && m_changeRepoComboIdx < n) {
            std::string targetRepo = repos[static_cast<size_t>(m_changeRepoComboIdx)];
            if (targetRepo == m_selectedRepo) {
              m_showChangeRepoModal = false;
              ImGui::CloseCurrentPopup();
              ImGui::EndPopup();
              return;
            }
            if (m_changeRepoIsFolder) {
              std::vector<IResourceManager::ResourceInfo> infos;
              m_resourceManager->GetResourceInfos(infos);
              std::string prefix = m_changeRepoTargetPath.empty() ? "" : (m_changeRepoTargetPath + "/");
              for (auto const& i : infos) {
                if (i.repository != m_selectedRepo) continue;
                if (m_changeRepoTargetPath.empty()) {
                  m_resourceManager->MoveResourceToRepository(i.guid, targetRepo.c_str());
                } else {
                  if (i.assetPath == m_changeRepoTargetPath || (i.assetPath.size() > prefix.size() && i.assetPath.compare(0, prefix.size(), prefix) == 0))
                    m_resourceManager->MoveResourceToRepository(i.guid, targetRepo.c_str());
                }
              }
            } else {
              m_resourceManager->MoveResourceToRepository(m_changeRepoTargetGuid, targetRepo.c_str());
            }
            m_showChangeRepoModal = false;
            m_resourceManager->LoadAllManifests();
            ImGui::CloseCurrentPopup();
          }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
          m_showChangeRepoModal = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }

    if (m_showDeleteFolderModal && m_resourceManager) {
      if (!ImGui::IsPopupOpen("Delete Folder"))
        ImGui::OpenPopup("Delete Folder");
      if (ImGui::BeginPopupModal("Delete Folder", &m_showDeleteFolderModal, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete folder \"%s\"?", m_deleteFolderPath.empty() ? "(root)" : m_deleteFolderPath.c_str());
        ImGui::Text("Resources inside will be moved to the parent folder.");
        if (ImGui::Button("Delete")) {
          if (!m_deleteFolderPath.empty() && !m_deleteFolderRepo.empty()) {
            m_resourceManager->RemoveAssetFolder(m_deleteFolderRepo.c_str(), m_deleteFolderPath.c_str());
            m_resourceManager->LoadAllManifests();
            if (m_selectedAssetPath == m_deleteFolderPath || (m_selectedAssetPath.size() > m_deleteFolderPath.size() && m_selectedAssetPath.compare(0, m_deleteFolderPath.size(), m_deleteFolderPath) == 0 && m_selectedAssetPath[m_deleteFolderPath.size()] == '/')) {
              std::string parent = te::core::PathGetDirectory(m_deleteFolderPath);
              m_selectedAssetPath = (parent == ".") ? "" : parent;
            }
          }
          m_showDeleteFolderModal = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
          m_showDeleteFolderModal = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
    }
  }

  void DoImportAfterFileDialog() {
    if (!m_resourceManager) return;
    std::string repo = m_importRepo.empty() ? (m_selectedRepo.empty() ? "main" : m_selectedRepo) : m_importRepo;
    if (repo.empty()) {
      std::vector<std::string> repos;
      m_resourceManager->GetRepositoryList(repos);
      if (!repos.empty()) repo = repos[0];
    }
    std::string folderPath = m_importFolderPath;
    std::vector<std::string> paths = OpenFileDialogMulti(nullptr, nullptr, m_rootPath.empty() ? nullptr : m_rootPath.c_str());
    for (std::string const& path : paths) {
      if (path.empty()) continue;
      ResourceType type = ImportTypeFromExtension(te::core::PathGetExtension(path));
      if (type == ResourceType::_Count) continue;
      m_resourceManager->ImportIntoRepository(path.c_str(), type, repo.c_str(), folderPath.c_str(), nullptr);
    }
    m_resourceManager->LoadAllManifests();
  }

  std::string m_rootPath;
  int m_filterType = -1;
  char m_nameFilterBuf[256] = "";
  std::string m_selectedRepo;
  std::string m_selectedAssetPath;
  ResourceId m_selectedResourceGuid;
  bool m_showLevels = false;
  std::string m_selectedLevelPath;
  std::function<void(std::string const&)> m_onOpenLevel;
  std::function<void(std::string const&)> m_onDeleteLevel;
  IResourceManager* m_resourceManager = nullptr;

  float m_leftRatio = 0.25f;
  float m_centerRatio = 0.45f;

  bool m_showCreateRepoModal = false;
  bool m_createRepoFailed = false;
  char m_createRepoNameBuf[256] = "";

  bool m_showNewFolderModal = false;
  std::string m_newFolderParentPath;
  char m_newFolderNameBuf[256] = "";

  bool m_showImportModal = false;
  std::string m_importRepo;
  std::string m_importFolderPath;
  bool m_pendingOpenFileDialog = false;

  bool m_showChangeRepoModal = false;
  int m_changeRepoComboIdx = 0;
  std::string m_changeRepoTargetPath;
  bool m_changeRepoIsFolder = false;
  ResourceId m_changeRepoTargetGuid;

  bool m_showDeleteFolderModal = false;
  std::string m_deleteFolderPath;
  std::string m_deleteFolderRepo;
};

IResourceView* CreateResourceView() {
  return new ResourceViewImpl();
}

}  // namespace editor
}  // namespace te
