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

// Note: PluginInfo, PluginHook, PluginMenuItem, and PluginPanel are defined in EditorTypes.h

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
