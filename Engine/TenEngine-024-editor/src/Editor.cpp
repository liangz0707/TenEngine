/**
 * @file Editor.cpp
 * @brief Editor implementation (024-Editor).
 */
#include <te/editor/Editor.h>
#include <te/editor/ImGuiBackend.h>
#include <te/core/platform.h>
#include <te/core/log.h>
#include <te/editor/SceneView.h>
#include <te/editor/ResourceView.h>
#include <imgui.h>
#include <te/editor/PropertyPanel.h>
#include <te/editor/Viewport.h>
#include <te/editor/RenderingSettingsPanel.h>
#include <te/editor/UndoSystem.h>
#include <te/application/Application.h>
#include <te/application/Window.h>
#include <te/core/math.h>
#include <te/scene/SceneTypes.h>
#include <te/world/WorldManager.h>
#include <te/world/LevelAssetDesc.h>
#include <te/world/LevelResource.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceTypes.h>
#include <te/resource/Resource.h>
#include <te/entity/EntityId.h>
#include <string>
#include <vector>
#include <exception>
#include <filesystem>

namespace te {
namespace editor {

class EditorImpl;
static void EditorTickCallback(float deltaTime);

static EditorContext g_editorCtx;
static EditorImpl* g_editorInstance = nullptr;

static std::vector<std::string> ScanLevelFiles(char const* projectRoot) {
  std::vector<std::string> out;
  if (!projectRoot) return out;
  std::string levelsDir = te::core::PathJoin(projectRoot, "assets/levels");
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

class EditorImpl : public IEditor {
public:
  enum class Phase { Launcher, MainEditor };

  EditorImpl() {
    m_undoSystem = CreateUndoSystem(50);
    m_sceneView = CreateSceneView();
    m_resourceView = CreateResourceView();
    m_propertyPanel = CreatePropertyPanel(m_undoSystem);
    m_renderViewport = CreateRenderViewport();
    m_renderingSettingsPanel = CreateRenderingSettingsPanel();
  }

  ~EditorImpl() override {
    delete m_renderingSettingsPanel;
    delete m_renderViewport;
    delete m_propertyPanel;
    delete m_resourceView;
    delete m_sceneView;
    delete m_undoSystem;
  }

  void OnTick() {
    te::application::IApplication* app = g_editorCtx.application;
    te::application::WindowId mainWnd = app->GetMainWindow();
    if (mainWnd == te::application::InvalidWindowId) return;

    void* hwnd = app->GetNativeHandle(mainWnd);
    if (!hwnd) return;

#if TE_PLATFORM_WINDOWS
    if (!ImGuiBackend_IsInitialized()) {
      ImGuiBackend_RegisterWndProcHandler(g_editorCtx.application);
      if (!ImGuiBackend_Init(hwnd, 1280, 720)) return;
    }
    ImGuiBackend_NewFrame();

    std::vector<std::string> dropped = ImGuiBackend_GetAndClearDroppedPaths();
    if (!dropped.empty() && m_resourceView)
      m_resourceView->ImportFiles(dropped);

    if (m_phase == Phase::Launcher) {
      DrawLauncherUI();
      if (m_showNewSceneModal) DrawNewSceneModal();
    } else {
      DrawMainEditorUI();
    }

    ImGuiBackend_Render();
#endif
  }

  void DrawLauncherUI() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    if (ImGui::Begin("Scene Launcher", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
      ImGui::Text("TenEngine Editor - Scene Launcher");
      ImGui::Separator();

      if (ImGui::Button("New Scene")) {
        m_showNewSceneModal = true;
        m_newSceneNameBuf[0] = '\0';
      }
      ImGui::SameLine();
      bool canOpen = (g_editorCtx.resourceManager != nullptr) || (te::resource::GetResourceManager() != nullptr);
      if (!canOpen) ImGui::BeginDisabled();
      if (ImGui::Button("Open Scene")) {
        ImGui::OpenPopup("OpenLevelPopup");
      }
      if (!canOpen) ImGui::EndDisabled();
      if (canOpen) {
        ImGui::SameLine();
        ImGui::TextDisabled("(Select a level below)");
      } else {
        ImGui::SameLine();
        ImGui::TextDisabled("(No resource manager)");
      }

      ImGui::Separator();
      ImGui::Text("Levels in assets/levels/");

      std::vector<std::string> levels = ScanLevelFiles(g_editorCtx.projectRootPath);
      m_levelPaths = levels;

      ImGui::BeginChild("LevelList", ImVec2(0, -30), true);
      for (size_t i = 0; i < levels.size(); ++i) {
        std::string const& path = levels[i];
        std::string name = te::core::PathGetFileName(path);
        if (ImGui::Selectable(name.c_str(), m_selectedLevelIndex == static_cast<int>(i))) {
          m_selectedLevelIndex = static_cast<int>(i);
        }
      }
      ImGui::EndChild();

      if (m_isLoadingLevel) {
        ImGui::Text("Loading...");
      }
      if (canOpen && m_selectedLevelIndex >= 0 && m_selectedLevelIndex < static_cast<int>(levels.size())) {
        bool loading = m_isLoadingLevel;
        if (loading) ImGui::BeginDisabled();
        if (ImGui::Button("Open Selected")) {
          OpenLevel(levels[static_cast<size_t>(m_selectedLevelIndex)]);
        }
        if (loading) ImGui::EndDisabled();
      }

      if (ImGui::BeginPopup("OpenLevelPopup")) {
        for (size_t i = 0; i < levels.size(); ++i) {
          std::string name = te::core::PathGetFileName(levels[i]);
          if (ImGui::Selectable(name.c_str())) {
            OpenLevel(levels[i]);
            ImGui::CloseCurrentPopup();
          }
        }
        ImGui::EndPopup();
      }
    }
    ImGui::End();
  }

  void DrawMainEditorUI() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    if (ImGui::Begin("TenEngine Editor", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_MenuBar)) {
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          if (ImGui::MenuItem("New Scene")) { m_showNewSceneModal = true; m_newSceneNameBuf[0] = '\0'; }
          if (ImGui::MenuItem("Open Scene")) { m_showOpenModal = true; }
          ImGui::Separator();
          if (ImGui::MenuItem("Save")) { OnSave(); }
          if (ImGui::MenuItem("Exit")) {
            if (g_editorCtx.application) g_editorCtx.application->RequestExit(0);
          }
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }

      float w = ImGui::GetWindowSize().x;
      float totalH = ImGui::GetWindowSize().y - 60.f;
      if (totalH < 1.f) totalH = 1.f;
      if (w < 1.f) w = 1.f;

      const float splitterW = 4.f;
      const float minLeft = 80.f, minCenter = 100.f, minRight = 80.f, minBottom = 60.f;

      if (m_mainLeftW <= 0.f || m_mainCenterW <= 0.f) {
        m_mainLeftW = w * 0.2f;
        m_mainCenterW = w * 0.5f;
      }
      if (m_mainBottomH <= 0.f) m_mainBottomH = 150.f;

      float leftW = m_mainLeftW;
      float centerW = m_mainCenterW;
      float rightW = w - leftW - centerW - splitterW * 2.f;
      if (rightW < minRight) {
        centerW -= (minRight - rightW);
        rightW = minRight;
        if (centerW < minCenter) {
          leftW -= (minCenter - centerW);
          centerW = minCenter;
          if (leftW < minLeft) leftW = minLeft;
        }
        m_mainLeftW = leftW;
        m_mainCenterW = centerW;
      }

      float mainH = totalH - m_mainBottomH - splitterW;
      if (mainH < 60.f) {
        m_mainBottomH = totalH - 60.f - splitterW;
        if (m_mainBottomH < minBottom) m_mainBottomH = minBottom;
        mainH = totalH - m_mainBottomH - splitterW;
      }
      float bottomH = m_mainBottomH;

      ImGui::BeginChild("Left", ImVec2(leftW, mainH), true);
      ImGui::Text("Scene Tree");
      if (m_sceneView) m_sceneView->OnDraw();
      ImGui::EndChild();

      ImGui::SameLine();
      if (ImGui::InvisibleButton("SplitV1", ImVec2(splitterW, mainH))) {}
      if (ImGui::IsItemActive()) {
        float mx = ImGui::GetIO().MousePos.x;
        ImVec2 winMin = ImGui::GetWindowPos();
        float newLeft = mx - winMin.x;
        if (newLeft >= minLeft && newLeft <= w - splitterW * 2.f - minCenter - minRight)
          m_mainLeftW = newLeft;
      }
      ImGui::SameLine();
      ImGui::BeginChild("Center", ImVec2(centerW, mainH), true);
      ImGui::Text("Viewport");
      ImGui::EndChild();

      ImGui::SameLine();
      if (ImGui::InvisibleButton("SplitV2", ImVec2(splitterW, mainH))) {}
      if (ImGui::IsItemActive()) {
        float mx = ImGui::GetIO().MousePos.x;
        ImVec2 winMin = ImGui::GetWindowPos();
        float newCenterEnd = mx - winMin.x;
        float newCenterW = newCenterEnd - m_mainLeftW - splitterW;
        if (newCenterW >= minCenter && newCenterW <= w - m_mainLeftW - splitterW * 2.f - minRight)
          m_mainCenterW = newCenterW;
      }
      ImGui::SameLine();
      ImGui::BeginChild("Right", ImVec2(rightW, mainH), true);
      ImGui::Text("Property Panel");
      if (m_sceneView && m_propertyPanel) {
        std::vector<te::entity::EntityId> sel;
        m_sceneView->GetSelection(sel);
        m_propertyPanel->SetSelection(sel);
      }
      if (m_propertyPanel) m_propertyPanel->OnDraw();
      ImGui::EndChild();

      if (ImGui::InvisibleButton("SplitH", ImVec2(w, splitterW))) {}
      if (ImGui::IsItemActive()) {
        float my = ImGui::GetIO().MousePos.y;
        float winBottom = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
        float newBottomH = winBottom - my - splitterW;
        if (newBottomH >= minBottom && newBottomH <= totalH - splitterW)
          m_mainBottomH = newBottomH;
      }
      ImGui::BeginChild("Bottom", ImVec2(w, bottomH), true);
      ImGui::Text("Resource Browser");
      if (m_resourceView) m_resourceView->OnDraw();
      ImGui::EndChild();

      if (m_showOpenModal) DrawOpenLevelModal();
      if (m_showNewSceneModal) DrawNewSceneModal();
    }
    ImGui::End();
  }

  void DrawNewSceneModal() {
    if (!ImGui::IsPopupOpen("New Scene")) ImGui::OpenPopup("New Scene");
    if (ImGui::BeginPopupModal("New Scene", &m_showNewSceneModal, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Scene name:");
      ImGui::SetNextItemWidth(280.f);
      bool enter = ImGui::InputText("##NewSceneName", m_newSceneNameBuf, sizeof(m_newSceneNameBuf), ImGuiInputTextFlags_EnterReturnsTrue);
      if (ImGui::Button("OK") || enter) {
        if (m_newSceneNameBuf[0] != '\0') {
          OnNewScene();
          if (g_editorCtx.projectRootPath && g_editorCtx.projectRootPath[0] != '\0') {
            std::string name = m_newSceneNameBuf;
            if (name.size() < 10 || name.compare(name.size() - 10, 10, ".level.xml") != 0)
              name += ".level.xml";
            m_currentLevelPath = te::core::PathJoin(te::core::PathJoin(g_editorCtx.projectRootPath, "assets/levels"), name);
          }
          m_showNewSceneModal = false;
          ImGui::CloseCurrentPopup();
        }
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) {
        m_showNewSceneModal = false;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  void DrawOpenLevelModal() {
    if (!ImGui::IsPopupOpen("Open Level")) ImGui::OpenPopup("Open Level");
    if (ImGui::BeginPopupModal("Open Level", &m_showOpenModal, ImGuiWindowFlags_AlwaysAutoResize)) {
      if (m_isLoadingLevel) {
        ImGui::Text("Loading level...");
      }
      std::vector<std::string> levels = ScanLevelFiles(g_editorCtx.projectRootPath);
      ImGui::BeginChild("ModalLevelList", ImVec2(400, 300), true);
      for (size_t i = 0; i < levels.size(); ++i) {
        std::string name = te::core::PathGetFileName(levels[i]);
        if (ImGui::Selectable(name.c_str())) {
          OpenLevel(levels[i]);
          m_showOpenModal = false;
          ImGui::CloseCurrentPopup();
        }
      }
      ImGui::EndChild();
      if (ImGui::Button("Cancel")) {
        m_showOpenModal = false;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  void OnSave() {
    if (!m_levelHandle.IsValid()) return;
    te::resource::IResourceManager* mgr = g_editorCtx.resourceManager ? g_editorCtx.resourceManager
                                                                     : te::resource::GetResourceManager();
    if (!mgr) return;
    std::string path = m_currentLevelPath.empty()
                           ? te::core::PathJoin(g_editorCtx.projectRootPath ? g_editorCtx.projectRootPath : ".", "assets/levels/untitled.level.xml")
                           : m_currentLevelPath;
    {
      bool ends_level = (path.size() >= 6 && path.compare(path.size() - 6, 6, ".level") == 0);
      bool ends_level_xml = (path.size() >= 10 && path.compare(path.size() - 10, 10, ".level.xml") == 0);
      if (ends_level && !ends_level_xml) path += ".xml";
    }
    if (te::world::WorldManager::GetInstance().SaveLevel(m_levelHandle, path.c_str())) {
      m_currentLevelPath = path;
    }
  }

  void OnNewScene() {
    te::core::AABB bounds;
    bounds.min = {-1000.f, -1000.f, -1000.f};
    bounds.max = {1000.f, 1000.f, 1000.f};
    te::world::LevelAssetDesc desc;
    m_levelHandle = te::world::WorldManager::GetInstance().CreateLevelFromDesc(
        te::scene::SpatialIndexType::None, bounds, desc);
    if (m_levelHandle.IsValid()) {
      m_phase = Phase::MainEditor;
      if (m_sceneView) m_sceneView->SetLevelHandle(m_levelHandle.value);
    }
  }

  void OpenLevel(std::string const& path) {
    if (path.empty()) return;
    te::resource::IResourceManager* mgr = g_editorCtx.resourceManager ? g_editorCtx.resourceManager
                                                                     : te::resource::GetResourceManager();
    if (!mgr) return;
    if (m_isLoadingLevel) return;

    (void)te::world::WorldManager::GetInstance();
    m_isLoadingLevel = true;

    struct OpenLevelCallbackContext {
      EditorImpl* editor;
      std::string path;
    };
    OpenLevelCallbackContext* ctx = new OpenLevelCallbackContext{this, path};

    mgr->RequestLoadAsync(
        path.c_str(),
        te::resource::ResourceType::Level,
        [](te::resource::IResource* r, te::resource::LoadResult result, void* user_data) {
          auto* c = static_cast<OpenLevelCallbackContext*>(user_data);
          if (c->editor) {
            try {
              c->editor->OnLevelLoaded(r, result, c->path);
            } catch (std::exception const& e) {
              std::string msg = std::string("Editor: Open level exception: ") + e.what();
              te::core::Log(te::core::LogLevel::Error, msg.c_str());
              c->editor->OnLevelLoadFailed();
            } catch (...) {
              te::core::Log(te::core::LogLevel::Error, "Editor: Open level unknown exception");
              c->editor->OnLevelLoadFailed();
            }
          }
          delete c;
        },
        ctx);
  }

  void OnLevelLoadFailed() {
    m_isLoadingLevel = false;
  }

  void OnLevelLoaded(te::resource::IResource* r, te::resource::LoadResult result, std::string const& path) {
    m_isLoadingLevel = false;
    if (result != te::resource::LoadResult::Ok || !r) {
      if (result == te::resource::LoadResult::Error) {
        te::core::Log(te::core::LogLevel::Error, "Editor: failed to load level");
      }
      if (r) r->Release();
      return;
    }
    te::world::ILevelResource* lr = dynamic_cast<te::world::ILevelResource*>(r);
    if (!lr) {
      r->Release();
      te::core::Log(te::core::LogLevel::Error, "Editor: level resource is not ILevelResource");
      return;
    }
    te::world::LevelAssetDesc desc = lr->GetLevelAssetDesc();
    r->Release();

    te::core::AABB bounds;
    bounds.min = {-1000.f, -1000.f, -1000.f};
    bounds.max = {1000.f, 1000.f, 1000.f};
    m_levelHandle = te::world::WorldManager::GetInstance().CreateLevelFromDesc(
        te::scene::SpatialIndexType::None, bounds, desc);
    if (m_levelHandle.IsValid()) {
      m_phase = Phase::MainEditor;
      m_currentLevelPath = path;
      if (m_sceneView) m_sceneView->SetLevelHandle(m_levelHandle.value);
    }
  }

  void Run(EditorContext const& ctx) override {
    if (!ctx.application) return;
    if (ctx.projectRootPath && m_resourceView) {
      std::string root = std::filesystem::absolute(ctx.projectRootPath).generic_string();
      m_resourceView->SetRootPath(root.c_str());
      m_resourceView->SetOnOpenLevel([this](std::string const& path) { OpenLevel(path); });
      if (ctx.resourceManager) {
        m_resourceView->SetResourceManager(ctx.resourceManager);
        ctx.resourceManager->SetAssetRoot(root.c_str());
        ctx.resourceManager->LoadAllManifests();
      }
    }
    g_editorCtx = ctx;
    g_editorInstance = this;

    te::application::RunParams runParams;
    runParams.windowTitle = "TenEngine Editor";
    runParams.windowWidth = 1280;
    runParams.windowHeight = 720;
    runParams.runMode = te::application::RunMode::Editor;
    runParams.tickCallback = EditorTickCallback;
    ctx.application->Run(runParams);

    g_editorInstance = nullptr;
#if TE_PLATFORM_WINDOWS
    ImGuiBackend_Shutdown();
#endif
  }

  ISceneView* GetSceneView() override { return m_sceneView; }
  IResourceView* GetResourceView() override { return m_resourceView; }
  IPropertyPanel* GetPropertyPanel() override { return m_propertyPanel; }
  IViewport* GetRenderViewport() override { return m_renderViewport; }
  IRenderingSettingsPanel* GetRenderingSettingsPanel() override { return m_renderingSettingsPanel; }

private:
  Phase m_phase = Phase::Launcher;
  te::world::LevelHandle m_levelHandle;
  int m_selectedLevelIndex = -1;
  std::vector<std::string> m_levelPaths;
  bool m_showOpenModal = false;
  bool m_showNewSceneModal = false;
  char m_newSceneNameBuf[256] = "";
  std::string m_currentLevelPath;
  bool m_isLoadingLevel = false;

  float m_mainLeftW = 0.f;
  float m_mainCenterW = 0.f;
  float m_mainBottomH = 0.f;

  IUndoSystem* m_undoSystem = nullptr;
  ISceneView* m_sceneView = nullptr;
  IResourceView* m_resourceView = nullptr;
  IPropertyPanel* m_propertyPanel = nullptr;
  IViewport* m_renderViewport = nullptr;
  IRenderingSettingsPanel* m_renderingSettingsPanel = nullptr;
};

static void EditorTickCallback(float deltaTime) {
  (void)deltaTime;
  if (!g_editorInstance || !g_editorCtx.application) return;
  g_editorInstance->OnTick();
}

IEditor* CreateEditor(EditorContext const& ctx) {
  (void)ctx;
  return new EditorImpl();
}

}  // namespace editor
}  // namespace te
