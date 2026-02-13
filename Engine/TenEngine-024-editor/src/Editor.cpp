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
    if (ext == ".level") {
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

    if (m_phase == Phase::Launcher) {
      DrawLauncherUI();
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
        OnNewScene();
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
        if (m_isLoadingLevel) ImGui::BeginDisabled();
        if (ImGui::Button("Open Selected")) {
          OpenLevel(levels[static_cast<size_t>(m_selectedLevelIndex)]);
        }
        if (m_isLoadingLevel) ImGui::EndDisabled();
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
          if (ImGui::MenuItem("New Scene")) OnNewScene();
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

      float w = ImGui::GetIO().DisplaySize.x;
      float bottomH = 150.f;
      float mainH = ImGui::GetIO().DisplaySize.y - 60.f - bottomH;
      if (mainH < 1.f) mainH = 1.f;
      if (w < 1.f) w = 1.f;
      if (bottomH < 1.f) bottomH = 1.f;

      ImGui::BeginChild("Left", ImVec2(w * 0.2f, mainH), true);
      ImGui::Text("Scene Tree");
      if (m_sceneView) m_sceneView->OnDraw();
      ImGui::EndChild();

      ImGui::SameLine();
      ImGui::BeginChild("Center", ImVec2(w * 0.5f - 4, mainH), true);
      ImGui::Text("Viewport");
      ImGui::EndChild();

      ImGui::SameLine();
      ImGui::BeginChild("Right", ImVec2(w * 0.3f - 20, mainH), true);
      ImGui::Text("Property Panel");
      if (m_sceneView && m_propertyPanel) {
        std::vector<te::entity::EntityId> sel;
        m_sceneView->GetSelection(sel);
        m_propertyPanel->SetSelection(sel);
      }
      if (m_propertyPanel) m_propertyPanel->OnDraw();
      ImGui::EndChild();

      ImGui::BeginChild("Bottom", ImVec2(w, bottomH), true);
      ImGui::Text("Resource Browser");
      if (m_resourceView) m_resourceView->OnDraw();
      ImGui::EndChild();

      if (m_showOpenModal) DrawOpenLevelModal();
    }
    ImGui::End();
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
                           ? te::core::PathJoin(g_editorCtx.projectRootPath ? g_editorCtx.projectRootPath : ".", "assets/levels/untitled.level")
                           : m_currentLevelPath;
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
    te::resource::IResourceManager* mgr = g_editorCtx.resourceManager ? g_editorCtx.resourceManager
                                                                     : te::resource::GetResourceManager();
    if (!mgr) return;
    if (m_isLoadingLevel) return;

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
            c->editor->OnLevelLoaded(r, result, c->path);
          }
          delete c;
        },
        ctx);
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
      m_resourceView->SetRootPath(ctx.projectRootPath);
      m_resourceView->SetOnOpenLevel([this](std::string const& path) { OpenLevel(path); });
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
  std::string m_currentLevelPath;
  bool m_isLoadingLevel = false;

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
