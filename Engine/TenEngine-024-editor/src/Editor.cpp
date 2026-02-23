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
// New components
#include <te/editor/Gizmo.h>
#include <te/editor/EditorCamera.h>
#include <te/editor/SelectionManager.h>
#include <te/editor/SnapSettings.h>
#include <te/editor/MainMenu.h>
#include <te/editor/Toolbar.h>
#include <te/editor/StatusBar.h>
#include <te/editor/ConsolePanel.h>
#include <te/editor/EditorPreferences.h>
#include <te/editor/ProfilerPanel.h>
#include <te/editor/StatisticsPanel.h>
#include <te/editor/LayoutManager.h>

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
#include <te/entity/Entity.h>
#include <string>
#include <vector>
#include <exception>
#include <filesystem>

namespace te {
namespace editor {

// ============================================================================
// Play Mode Snapshot Structures
// ============================================================================

/**
 * @brief Snapshot of an entity's transform for play mode state restoration.
 */
struct EntityTransformSnapshot {
  te::entity::EntityId entityId;
  te::scene::Transform localTransform;
};

/**
 * @brief Complete snapshot for play mode state management.
 *
 * Saves all entity transforms and camera state when entering play mode,
 * allowing full restoration when exiting play mode.
 */
struct PlayModeSnapshot {
  std::vector<EntityTransformSnapshot> entityTransforms;
  te::core::Vector3 cameraPosition;
  te::core::Vector3 cameraRotationEuler;  // Yaw, Pitch, Roll in radians
  float cameraOrbitDistance;
  bool hasCameraState;
};

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
    // Existing components
    m_undoSystem = CreateUndoSystem(50);
    m_sceneView = CreateSceneView();
    m_resourceView = CreateResourceView();
    m_propertyPanel = CreatePropertyPanel(m_undoSystem);
    m_renderViewport = CreateRenderViewport();
    m_renderingSettingsPanel = CreateRenderingSettingsPanel();
    
    // New components
    m_gizmo = CreateGizmo();
    m_editorCamera = CreateEditorCamera();
    m_selectionManager = CreateSelectionManager();
    m_snapSettings = CreateSnapSettings();
    m_mainMenu = CreateMainMenu();
    m_toolbar = CreateToolbar();
    m_statusBar = CreateStatusBar();
    m_consolePanel = CreateConsolePanel();
    m_preferences = CreateEditorPreferences();
    m_profilerPanel = CreateProfilerPanel();
    m_statisticsPanel = CreateStatisticsPanel();
    m_layoutManager = CreateLayoutManager();
    
    // Initialize main menu
    m_mainMenu->InitializeStandardMenus();
    m_mainMenu->SetOnMenuItemClicked([this](const char* menuName, int itemId) {
      OnMenuItemClicked(menuName, itemId);
    });
    
    // Setup toolbar callbacks
    m_toolbar->SetOnPlayClicked([this]() { EnterPlayMode(); });
    m_toolbar->SetOnPauseClicked([this]() { PausePlayMode(); });
    m_toolbar->SetOnStopClicked([this]() { StopPlayMode(); });
    m_toolbar->SetOnStepClicked([this]() { StepFrame(); });
    m_toolbar->SetOnTransformToolChanged([this](GizmoMode mode) {
      if (m_gizmo) m_gizmo->SetMode(mode);
    });
  }

  ~EditorImpl() override {
    // Delete new components
    delete m_layoutManager;
    delete m_statisticsPanel;
    delete m_profilerPanel;
    delete m_preferences;
    delete m_consolePanel;
    delete m_statusBar;
    delete m_toolbar;
    delete m_mainMenu;
    delete m_snapSettings;
    delete m_selectionManager;
    delete m_editorCamera;
    delete m_gizmo;
    
    // Delete existing components
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

    // Update frame time for profiler
    auto now = std::chrono::high_resolution_clock::now();
    float frameTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
    m_lastFrameTime = now;
    m_frameTimeMs = frameTime * 1000.0f;
    m_fps = (frameTime > 0.0f) ? (1.0f / frameTime) : 0.0f;

    // Run game update when in play mode
    if (m_playModeState == PlayModeState::Playing && m_phase == Phase::MainEditor) {
      OnGameUpdate(frameTime);
    }

#if TE_PLATFORM_WINDOWS
    if (!ImGuiBackend_IsInitialized()) {
      ImGuiBackend_RegisterWndProcHandler(g_editorCtx.application);
      if (!ImGuiBackend_Init(hwnd, 1280, 720)) return;
    }
    ImGuiBackend_NewFrame();

    std::vector<std::string> dropped = ImGuiBackend_GetAndClearDroppedPaths();
    if (!dropped.empty() && m_resourceView)
      m_resourceView->ImportFiles(dropped);

    // Process editor camera input
    if (m_editorCamera && m_phase == Phase::MainEditor) {
      m_editorCamera->OnInput(frameTime);
    }

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
      
      ImGui::Separator();
      ImGui::Text("Levels in assets/levels/");

      std::vector<std::string> levels = ScanLevelFiles(g_editorCtx.projectRootPath);
      m_levelPaths = levels;

      ImGui::BeginChild("LevelList", ImVec2(0, -30), true);
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      const float tileW = 96.f;
      const float tileH = 90.f;
      const float iconSz = 64.f;
      float availW = ImGui::GetContentRegionAvail().x;
      int cols = (availW > tileW) ? static_cast<int>(availW / tileW) : 1;
      
      for (size_t i = 0; i < levels.size(); ++i) {
        if (i > 0 && (i % static_cast<size_t>(cols)) != 0)
          ImGui::SameLine();
        std::string const& path = levels[i];
        std::string name = te::core::PathGetFileName(path);
        if (name.size() > 12u) name = name.substr(0, 9) + "...";
        bool selected = (m_selectedLevelIndex == static_cast<int>(i));
        ImGui::PushID(static_cast<int>(i));
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##tile", ImVec2(tileW, tileH));
        bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked(0)) m_selectedLevelIndex = static_cast<int>(i);
        if (hovered && ImGui::IsMouseDoubleClicked(0) && canOpen && !m_isLoadingLevel) OpenLevel(path);
        ImVec2 iconMin(pos.x + (tileW - iconSz) * 0.5f, pos.y + 2.f);
        ImVec2 iconMax(iconMin.x + iconSz, iconMin.y + iconSz);
        ImU32 iconCol = selected ? IM_COL32(90, 90, 140, 255) : (hovered ? IM_COL32(70, 70, 100, 255) : IM_COL32(55, 55, 70, 255));
        drawList->AddRectFilled(iconMin, iconMax, iconCol, 6.f);
        drawList->AddRect(iconMin, iconMax, IM_COL32(120, 120, 150, 200), 6.f);
        const char* cname = name.c_str();
        ImVec2 textSize = ImGui::CalcTextSize(cname, nullptr, false, tileW - 4.f);
        ImVec2 textPos(pos.x + (tileW - textSize.x) * 0.5f, iconMax.y + 4.f);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), cname);
        ImGui::PopID();
      }
      ImGui::EndChild();

      if (m_isLoadingLevel) {
        ImGui::Text("Loading...");
      }
      if (canOpen && m_selectedLevelIndex >= 0 && m_selectedLevelIndex < static_cast<int>(levels.size())) {
        if (ImGui::Button("Open Selected")) {
          OpenLevel(levels[static_cast<size_t>(m_selectedLevelIndex)]);
        }
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
      // Main Menu
      if (ImGui::BeginMenuBar()) {
        m_mainMenu->OnDraw();
        ImGui::EndMenuBar();
      }

      // Toolbar
      if (m_toolbar) {
        m_toolbar->OnDraw();
      }

      ImGui::Separator();

      // Layout calculation
      float w = ImGui::GetWindowSize().x;
      float totalH = ImGui::GetWindowSize().y - 100.f;  // Account for menu+toolbar+status
      if (totalH < 1.f) totalH = 1.f;
      if (w < 1.f) w = 1.f;

      const float splitterW = 4.f;
      const float minLeft = 80.f, minCenter = 100.f, minRight = 80.f, minBottom = 60.f;

      if (m_mainLeftRatio <= 0.f || m_mainCenterRatio <= 0.f) {
        m_mainLeftRatio = 0.2f;
        m_mainCenterRatio = 0.5f;
      }
      if (m_mainBottomRatio <= 0.f) m_mainBottomRatio = 0.25f;

      float leftW = w * m_mainLeftRatio;
      float centerW = w * m_mainCenterRatio;
      float rightW = w - leftW - centerW - splitterW * 2.f;
      
      float mainH = totalH * (1.f - m_mainBottomRatio) - splitterW;
      float bottomH = totalH * m_mainBottomRatio;

      // Left Panel - Scene Tree
      ImGui::BeginChild("Left", ImVec2(leftW, mainH), true);
      ImGui::Text("Scene Tree");
      if (m_sceneView) m_sceneView->OnDraw();
      ImGui::EndChild();

      ImGui::SameLine();
      ImGui::InvisibleButton("SplitV1", ImVec2(splitterW, mainH));
      ImGui::SameLine();

      // Center Panel - Viewport
      ImGui::BeginChild("Center", ImVec2(centerW, mainH), true);
      ImGui::Text("Viewport");
      // Draw gizmo if we have selection
      if (m_gizmo && m_selectionManager) {
        auto const& sel = m_selectionManager->GetSelection();
        if (!sel.empty()) {
          // m_gizmo->OnDraw();  // Would need entity adapter
        }
      }
      ImGui::EndChild();

      ImGui::SameLine();
      ImGui::InvisibleButton("SplitV2", ImVec2(splitterW, mainH));
      ImGui::SameLine();

      // Right Panel - Properties
      ImGui::BeginChild("Right", ImVec2(rightW, mainH), true);
      ImGui::Text("Property Panel");
      if (m_sceneView && m_propertyPanel) {
        std::vector<te::entity::EntityId> sel;
        m_sceneView->GetSelection(sel);
        m_propertyPanel->SetSelection(sel);
      }
      if (m_propertyPanel) m_propertyPanel->OnDraw();
      ImGui::EndChild();

      // Bottom Panel Splitter
      ImGui::InvisibleButton("SplitH", ImVec2(w, splitterW));
      
      // Bottom Panel - Resources + Console
      ImGui::BeginChild("Bottom", ImVec2(w, bottomH), true);
      
      // Tab bar for bottom panels
      if (ImGui::BeginTabBar("BottomTabs")) {
        if (ImGui::BeginTabItem("Resources")) {
          if (m_resourceView) m_resourceView->OnDraw();
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Console")) {
          if (m_consolePanel) m_consolePanel->OnDraw();
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Profiler")) {
          if (m_profilerPanel) m_profilerPanel->OnDraw();
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Statistics")) {
          if (m_statisticsPanel) m_statisticsPanel->OnDraw();
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }
      ImGui::EndChild();

      // Modals
      if (m_showOpenModal) DrawOpenLevelModal();
      if (m_showNewSceneModal) DrawNewSceneModal();
      DrawDeleteLevelModal();
    }
    ImGui::End();
    
    // Status Bar (separate window at bottom)
    if (m_statusBar) {
      m_statusBar->SetLevelName(m_currentLevelPath.empty() ? "No Level" : te::core::PathGetFileName(m_currentLevelPath).c_str());
      m_statusBar->SetFPS(m_fps);
      m_statusBar->SetFrameTime(m_frameTimeMs);
      if (m_selectionManager) {
        m_statusBar->SetSelectionCount(static_cast<int>(m_selectionManager->GetSelectionCount()));
      }
      m_statusBar->OnDraw();
    }
  }

  void OnMenuItemClicked(const char* menuName, int itemId) {
    if (std::string(menuName) == "File") {
      switch (itemId) {
        case IMainMenu::ID_NEW_SCENE:
          m_showNewSceneModal = true;
          m_newSceneNameBuf[0] = '\0';
          break;
        case IMainMenu::ID_OPEN_SCENE:
          m_showOpenModal = true;
          break;
        case IMainMenu::ID_SAVE:
          OnSave();
          break;
        case IMainMenu::ID_EXIT:
          if (g_editorCtx.application) g_editorCtx.application->RequestExit(0);
          break;
      }
    } else if (std::string(menuName) == "Edit") {
      switch (itemId) {
        case IMainMenu::ID_UNDO:
          if (m_undoSystem) m_undoSystem->Undo();
          break;
        case IMainMenu::ID_REDO:
          if (m_undoSystem) m_undoSystem->Redo();
          break;
      }
    }
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
        std::string const& path = levels[i];
        std::string name = te::core::PathGetFileName(path);
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::Selectable(name.c_str())) {
          OpenLevel(path);
          m_showOpenModal = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::PopID();
      }
      ImGui::EndChild();
      if (ImGui::Button("Cancel")) {
        m_showOpenModal = false;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  void DeleteLevelFile(std::string const& path) {
    if (path.empty()) return;
    if (path == m_currentLevelPath) {
      if (m_levelHandle.IsValid()) {
        te::world::WorldManager::GetInstance().UnloadLevel(m_levelHandle);
        m_levelHandle = te::world::LevelHandle();
        if (m_sceneView) m_sceneView->SetLevelHandle(nullptr);
      }
      m_currentLevelPath.clear();
    }
    std::error_code ec;
    std::filesystem::remove(std::filesystem::u8path(path), ec);
    if (ec) {
      te::core::Log(te::core::LogLevel::Error, ("Editor: failed to delete level file: " + path).c_str());
    }
  }

  void DrawDeleteLevelModal() {
    if (!m_showDeleteLevelModal) return;
    if (!ImGui::IsPopupOpen("Delete Level")) ImGui::OpenPopup("Delete Level");
    if (ImGui::BeginPopupModal("Delete Level", &m_showDeleteLevelModal, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Delete level \"%s\"?", te::core::PathGetFileName(m_deleteLevelPath).c_str());
      ImGui::Text("This cannot be undone.");
      if (ImGui::Button("Delete")) {
        DeleteLevelFile(m_deleteLevelPath);
        m_showDeleteLevelModal = false;
        m_showOpenModal = false;
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel")) {
        m_showDeleteLevelModal = false;
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
              te::core::Log(te::core::LogLevel::Error, (std::string("Editor: Open level exception: ") + e.what()).c_str());
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
      if (r) r->Release();
      return;
    }
    te::world::ILevelResource* lr = dynamic_cast<te::world::ILevelResource*>(r);
    if (!lr) {
      r->Release();
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

  // === IEditor Implementation ===

  void Run(EditorContext const& ctx) override {
    if (!ctx.application) return;
    if (ctx.projectRootPath && m_resourceView) {
      std::string root = std::filesystem::absolute(ctx.projectRootPath).generic_string();
      m_resourceView->SetRootPath(root.c_str());
      m_resourceView->SetOnOpenLevel([this](std::string const& path) { OpenLevel(path); });
      m_resourceView->SetOnDeleteLevel([this](std::string const& path) {
        m_showDeleteLevelModal = true;
        m_deleteLevelPath = path;
      });
      if (ctx.resourceManager) {
        m_resourceView->SetResourceManager(ctx.resourceManager);
        ctx.resourceManager->SetAssetRoot(root.c_str());
        ctx.resourceManager->LoadAllManifests();
      }
    }
    g_editorCtx = ctx;
    g_editorInstance = this;
    m_lastFrameTime = std::chrono::high_resolution_clock::now();

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

  // Existing accessors
  ISceneView* GetSceneView() override { return m_sceneView; }
  IResourceView* GetResourceView() override { return m_resourceView; }
  IPropertyPanel* GetPropertyPanel() override { return m_propertyPanel; }
  IViewport* GetRenderViewport() override { return m_renderViewport; }
  IRenderingSettingsPanel* GetRenderingSettingsPanel() override { return m_renderingSettingsPanel; }

  // New component accessors
  IGizmo* GetGizmo() override { return m_gizmo; }
  IEditorCamera* GetEditorCamera() override { return m_editorCamera; }
  ISelectionManager* GetSelectionManager() override { return m_selectionManager; }
  ISnapSettings* GetSnapSettings() override { return m_snapSettings; }
  IMainMenu* GetMainMenu() override { return m_mainMenu; }
  IToolbar* GetToolbar() override { return m_toolbar; }
  IStatusBar* GetStatusBar() override { return m_statusBar; }
  IConsolePanel* GetConsolePanel() override { return m_consolePanel; }
  IEditorPreferences* GetPreferences() override { return m_preferences; }
  IProfilerPanel* GetProfilerPanel() override { return m_profilerPanel; }
  IStatisticsPanel* GetStatisticsPanel() override { return m_statisticsPanel; }
  ILayoutManager* GetLayoutManager() override { return m_layoutManager; }

  // Play mode control
  void EnterPlayMode() override {
    if (m_playModeState == PlayModeState::Playing) return;

    // Save current state before entering play mode
    SavePlayModeSnapshot();

    m_playModeState = PlayModeState::Playing;
    if (m_toolbar) m_toolbar->SetPlayModeState(m_playModeState);
    te::core::Log(te::core::LogLevel::Info, "Editor: Entered play mode");

    // TODO: Enable game systems (physics, AI, scripts)
    // This would typically involve:
    // 1. Starting physics simulation
    // 2. Enabling script execution
    // 3. Activating AI systems
  }
  void PausePlayMode() override {
    if (m_playModeState != PlayModeState::Playing) return;

    m_playModeState = PlayModeState::Paused;
    if (m_toolbar) m_toolbar->SetPlayModeState(m_playModeState);
    te::core::Log(te::core::LogLevel::Info, "Editor: Paused play mode");

    // TODO: Pause game systems (physics, AI, scripts)
  }
  void StopPlayMode() override {
    if (m_playModeState == PlayModeState::Stopped) return;

    m_playModeState = PlayModeState::Stopped;
    if (m_toolbar) m_toolbar->SetPlayModeState(m_playModeState);
    te::core::Log(te::core::LogLevel::Info, "Editor: Stopped play mode");

    // Restore state from saved snapshot
    RestorePlayModeSnapshot();

    // TODO: Disable game systems (physics, AI, scripts)
  }
  void StepFrame() override {
    // Can only step when paused or stopped
    if (m_playModeState == PlayModeState::Playing) {
      te::core::Log(te::core::LogLevel::Warn, "Editor: Cannot step frame while playing");
      return;
    }

    // Save current state before stepping (if not already in play mode)
    bool wasStopped = (m_playModeState == PlayModeState::Stopped);
    if (wasStopped) {
      SavePlayModeSnapshot();
    }

    te::core::Log(te::core::LogLevel::Info, "Editor: Stepping one frame");

    // Temporarily set to playing state for the frame
    PlayModeState previousState = m_playModeState;
    m_playModeState = PlayModeState::Playing;

    // Execute one frame update
    float frameTime = m_frameTimeMs / 1000.0f;
    if (frameTime <= 0.0f) frameTime = 1.0f / 60.0f;  // Default to 60 FPS
    OnGameUpdate(frameTime);

    // Restore previous state (paused or stopped)
    m_playModeState = previousState;
    if (m_toolbar) m_toolbar->SetPlayModeState(m_playModeState);

    // If we were stopped, restore state immediately
    if (wasStopped) {
      RestorePlayModeSnapshot();
    }
  }
  bool IsInPlayMode() const override {
    return m_playModeState != PlayModeState::Stopped;
  }
  PlayModeState GetPlayModeState() const override {
    return m_playModeState;
  }

  // Layout management
  void SaveLayout(char const* path) override {
    if (m_layoutManager) m_layoutManager->SaveToFile(path);
  }
  void LoadLayout(char const* path) override {
    if (m_layoutManager) m_layoutManager->LoadFromFile(path);
  }
  void ResetLayout() override {
    if (m_layoutManager) m_layoutManager->ResetToDefault();
    m_mainLeftRatio = 0.2f;
    m_mainCenterRatio = 0.5f;
    m_mainBottomRatio = 0.25f;
  }

private:
  // === Play Mode Snapshot Management ===

  /**
   * @brief Save current level state before entering play mode.
   *
   * Traverses all entities in the current level and saves their transforms.
   * Also saves editor camera state.
   */
  void SavePlayModeSnapshot() {
    m_playModeSnapshot.entityTransforms.clear();
    m_playModeSnapshot.hasCameraState = false;

    if (!m_levelHandle.IsValid()) {
      te::core::Log(te::core::LogLevel::Warn, "Editor: SavePlayModeSnapshot - no valid level");
      return;
    }

    // Traverse all nodes in the level and save transforms
    te::world::WorldManager::GetInstance().Traverse(m_levelHandle,
      [this](te::scene::ISceneNode* node) {
        if (!node) return;

        // Try to cast to Entity
        te::entity::Entity* entity = dynamic_cast<te::entity::Entity*>(node);
        if (entity) {
          EntityTransformSnapshot snapshot;
          snapshot.entityId = entity->GetEntityId();
          snapshot.localTransform = entity->GetLocalTransform();
          m_playModeSnapshot.entityTransforms.push_back(snapshot);
        }
      });

    // Save editor camera state
    if (m_editorCamera) {
      m_playModeSnapshot.cameraPosition = m_editorCamera->GetPosition();
      m_playModeSnapshot.cameraRotationEuler = te::core::Vector3{
        m_editorCamera->GetYaw(),
        m_editorCamera->GetPitch(),
        0.0f  // Roll not exposed
      };
      m_playModeSnapshot.cameraOrbitDistance = m_editorCamera->GetOrbitDistance();
      m_playModeSnapshot.hasCameraState = true;
    }

    te::core::Log(te::core::LogLevel::Info,
      ("Editor: Saved play mode snapshot with " +
       std::to_string(m_playModeSnapshot.entityTransforms.size()) + " entities").c_str());
  }

  /**
   * @brief Restore level state after exiting play mode.
   *
   * Restores all entity transforms and camera state from the saved snapshot.
   */
  void RestorePlayModeSnapshot() {
    if (m_playModeSnapshot.entityTransforms.empty() && !m_playModeSnapshot.hasCameraState) {
      te::core::Log(te::core::LogLevel::Warn, "Editor: RestorePlayModeSnapshot - no snapshot to restore");
      return;
    }

    if (!m_levelHandle.IsValid()) {
      te::core::Log(te::core::LogLevel::Warn, "Editor: RestorePlayModeSnapshot - no valid level");
      return;
    }

    // Restore entity transforms
    int restoredCount = 0;
    te::world::WorldManager::GetInstance().Traverse(m_levelHandle,
      [this, &restoredCount](te::scene::ISceneNode* node) {
        if (!node) return;

        te::entity::Entity* entity = dynamic_cast<te::entity::Entity*>(node);
        if (!entity) return;

        // Find matching snapshot
        te::entity::EntityId entityId = entity->GetEntityId();
        for (auto const& snapshot : m_playModeSnapshot.entityTransforms) {
          if (snapshot.entityId == entityId) {
            entity->SetLocalTransform(snapshot.localTransform);
            restoredCount++;
            break;
          }
        }
      });

    // Restore editor camera state
    if (m_playModeSnapshot.hasCameraState && m_editorCamera) {
      m_editorCamera->SetPosition(m_playModeSnapshot.cameraPosition);
      m_editorCamera->SetRotation(
        m_playModeSnapshot.cameraRotationEuler.x,
        m_playModeSnapshot.cameraRotationEuler.y
      );
      m_editorCamera->SetOrbitDistance(m_playModeSnapshot.cameraOrbitDistance);
    }

    te::core::Log(te::core::LogLevel::Info,
      ("Editor: Restored " + std::to_string(restoredCount) + " entity transforms").c_str());

    // Clear snapshot
    m_playModeSnapshot.entityTransforms.clear();
    m_playModeSnapshot.hasCameraState = false;
  }

  /**
   * @brief Execute one frame of game update.
   * @param deltaTime Frame delta time in seconds
   */
  void OnGameUpdate(float deltaTime) {
    // Call external game update callback if registered
    if (m_onGameUpdate) {
      m_onGameUpdate(deltaTime);
    }

    // TODO: Integrate with game systems:
    // - Physics simulation step
    // - Animation update
    // - Script tick
    // - AI update
  }

  Phase m_phase = Phase::Launcher;
  te::world::LevelHandle m_levelHandle;
  int m_selectedLevelIndex = -1;
  std::vector<std::string> m_levelPaths;
  bool m_showOpenModal = false;
  bool m_showNewSceneModal = false;
  char m_newSceneNameBuf[256] = "";
  std::string m_currentLevelPath;
  bool m_isLoadingLevel = false;

  bool m_showDeleteLevelModal = false;
  std::string m_deleteLevelPath;

  float m_mainLeftRatio = 0.f;
  float m_mainCenterRatio = 0.f;
  float m_mainBottomRatio = 0.f;

  // Frame timing
  std::chrono::high_resolution_clock::time_point m_lastFrameTime;
  float m_frameTimeMs = 0.0f;
  float m_fps = 0.0f;

  // Play mode state
  PlayModeState m_playModeState = PlayModeState::Stopped;
  PlayModeSnapshot m_playModeSnapshot;  // Saved state for restoration
  std::function<void(float)> m_onGameUpdate;  // Game update callback

  // Existing components
  IUndoSystem* m_undoSystem = nullptr;
  ISceneView* m_sceneView = nullptr;
  IResourceView* m_resourceView = nullptr;
  IPropertyPanel* m_propertyPanel = nullptr;
  IViewport* m_renderViewport = nullptr;
  IRenderingSettingsPanel* m_renderingSettingsPanel = nullptr;

  // New components
  IGizmo* m_gizmo = nullptr;
  IEditorCamera* m_editorCamera = nullptr;
  ISelectionManager* m_selectionManager = nullptr;
  ISnapSettings* m_snapSettings = nullptr;
  IMainMenu* m_mainMenu = nullptr;
  IToolbar* m_toolbar = nullptr;
  IStatusBar* m_statusBar = nullptr;
  IConsolePanel* m_consolePanel = nullptr;
  IEditorPreferences* m_preferences = nullptr;
  IProfilerPanel* m_profilerPanel = nullptr;
  IStatisticsPanel* m_statisticsPanel = nullptr;
  ILayoutManager* m_layoutManager = nullptr;
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
