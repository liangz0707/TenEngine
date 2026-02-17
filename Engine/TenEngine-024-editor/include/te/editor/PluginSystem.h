/**
 * @file PluginSystem.h
 * @brief Editor plugin system interface.
 */
#ifndef TE_EDITOR_PLUGIN_SYSTEM_H
#define TE_EDITOR_PLUGIN_SYSTEM_H

#include <te/editor/EditorTypes.h>
#include <functional>
#include <vector>
#include <cstdint>

namespace te {
namespace editor {

/**
 * @brief Plugin metadata.
 */
struct PluginInfo {
  char id[64] = "";              ///< Unique plugin ID
  char name[128] = "";           ///< Display name
  char version[32] = "";         ///< Version string
  char author[128] = "";         ///< Author name
  char description[512] = "";    ///< Description
  char path[256] = "";           ///< Plugin path
  
  bool enabled = true;
  bool loaded = false;
  bool hasError = false;
  char errorText[512] = "";
};

/**
 * @brief Plugin hook points.
 */
enum class PluginHook {
  OnEditorInit,           ///< Editor initialization
  OnEditorShutdown,       ///< Editor shutdown
  OnSceneLoaded,          ///< Scene loaded
  OnSceneSaved,           ///< Scene saved
  OnEntitySelected,       ///< Entity selected
  OnEntityCreated,        ///< Entity created
  OnEntityDeleted,        ///< Entity deleted
  OnComponentAdded,       ///< Component added
  OnComponentRemoved,     ///< Component removed
  OnPlayModeEnter,        ///< Enter play mode
  OnPlayModeExit,         ///< Exit play mode
  OnViewportRender,       ///< Viewport render
  OnUpdate,               ///< Editor update tick
  Count
};

/**
 * @brief Plugin menu item.
 */
struct PluginMenuItem {
  char path[256] = "";           ///< Menu path (e.g., "Tools/MyPlugin/Action1")
  char label[128] = "";          ///< Display label
  char shortcut[32] = "";        ///< Keyboard shortcut
  bool enabled = true;
  bool checked = false;
  std::function<void()> callback;
};

/**
 * @brief Plugin panel definition.
 */
struct PluginPanel {
  char id[64] = "";
  char name[128] = "";
  bool visible = true;
  bool canClose = true;
  bool canFloat = true;
  std::function<void()> onDraw;
};

/**
 * @brief Plugin interface.
 */
class IPlugin {
public:
  virtual ~IPlugin() = default;
  
  /**
   * @brief Get plugin info.
   */
  virtual PluginInfo const& GetInfo() const = 0;
  
  /**
   * @brief Initialize the plugin.
   * @return true if successful
   */
  virtual bool Initialize() = 0;
  
  /**
   * @brief Shutdown the plugin.
   */
  virtual void Shutdown() = 0;
  
  /**
   * @brief Get menu items provided by this plugin.
   */
  virtual std::vector<PluginMenuItem> GetMenuItems() = 0;
  
  /**
   * @brief Get panels provided by this plugin.
   */
  virtual std::vector<PluginPanel> GetPanels() = 0;
  
  /**
   * @brief Handle hook callback.
   */
  virtual void OnHook(PluginHook hook, void* userData) = 0;
};

/**
 * @brief Plugin system interface.
 * 
 * Provides dynamic plugin loading and management.
 */
class IPluginSystem {
public:
  virtual ~IPluginSystem() = default;
  
  // === Discovery ===
  
  /**
   * @brief Scan directory for plugins.
   * @param directory Directory to scan
   * @return Number of plugins found
   */
  virtual size_t ScanForPlugins(char const* directory) = 0;
  
  /**
   * @brief Get all discovered plugins.
   */
  virtual std::vector<PluginInfo const*> GetDiscoveredPlugins() const = 0;
  
  // === Loading ===
  
  /**
   * @brief Load a plugin by ID.
   * @param pluginId Plugin ID
   * @return true if loaded successfully
   */
  virtual bool LoadPlugin(char const* pluginId) = 0;
  
  /**
   * @brief Unload a plugin.
   */
  virtual bool UnloadPlugin(char const* pluginId) = 0;
  
  /**
   * @brief Load all discovered plugins.
   */
  virtual size_t LoadAllPlugins() = 0;
  
  /**
   * @brief Unload all loaded plugins.
   */
  virtual void UnloadAllPlugins() = 0;
  
  // === Query ===
  
  /**
   * @brief Check if plugin is loaded.
   */
  virtual bool IsPluginLoaded(char const* pluginId) const = 0;
  
  /**
   * @brief Get plugin info.
   */
  virtual PluginInfo const* GetPluginInfo(char const* pluginId) const = 0;
  
  /**
   * @brief Get loaded plugin interface.
   */
  virtual IPlugin* GetPlugin(char const* pluginId) = 0;
  
  /**
   * @brief Get all loaded plugins.
   */
  virtual std::vector<IPlugin*> GetLoadedPlugins() = 0;
  
  // === Enable/Disable ===
  
  /**
   * @brief Enable/disable a plugin.
   */
  virtual void SetPluginEnabled(char const* pluginId, bool enabled) = 0;
  
  /**
   * @brief Check if plugin is enabled.
   */
  virtual bool IsPluginEnabled(char const* pluginId) const = 0;
  
  // === Hooks ===
  
  /**
   * @brief Register a hook callback.
   * @param hook Hook point
   * @param callback Callback function
   * @return Callback ID for unregistering
   */
  virtual int RegisterHook(PluginHook hook, std::function<void(void*)> callback) = 0;
  
  /**
   * @brief Unregister a hook callback.
   */
  virtual void UnregisterHook(PluginHook hook, int callbackId) = 0;
  
  /**
   * @brief Trigger a hook.
   */
  virtual void TriggerHook(PluginHook hook, void* userData = nullptr) = 0;
  
  // === Menu Items ===
  
  /**
   * @brief Get all plugin menu items.
   */
  virtual std::vector<PluginMenuItem> GetAllPluginMenuItems() = 0;
  
  // === Panels ===
  
  /**
   * @brief Get all plugin panels.
   */
  virtual std::vector<PluginPanel> GetAllPluginPanels() = 0;
  
  // === Settings ===
  
  /**
   * @brief Save plugin settings.
   */
  virtual bool SavePluginSettings(char const* pluginId) = 0;
  
  /**
   * @brief Load plugin settings.
   */
  virtual bool LoadPluginSettings(char const* pluginId) = 0;
  
  // === Hot Reload ===
  
  /**
   * @brief Reload a plugin (hot reload).
   */
  virtual bool ReloadPlugin(char const* pluginId) = 0;
};

/**
 * @brief Factory function to create plugin system.
 */
IPluginSystem* CreatePluginSystem();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_PLUGIN_SYSTEM_H
