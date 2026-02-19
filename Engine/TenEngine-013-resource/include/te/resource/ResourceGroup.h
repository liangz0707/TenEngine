/**
 * @file ResourceGroup.h
 * @brief Resource group management for batch operations.
 * 
 * Provides functionality for:
 * - Grouping resources together
 * - Batch load/unload operations
 * - Dependency-aware group management
 * - Scene/level resource organization
 */
#ifndef TE_RESOURCE_RESOURCE_GROUP_H
#define TE_RESOURCE_RESOURCE_GROUP_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include <functional>

namespace te {
namespace resource {

class IResourceManager;

/**
 * Opaque handle for resource group operations.
 */
using ResourceGroupId = void*;

/**
 * Group load complete callback.
 * Called when all resources in a group have completed loading.
 */
using GroupLoadCompleteCallback = void (*)(ResourceGroupId group, 
                                            std::size_t successCount,
                                            std::size_t failedCount,
                                            void* user_data);

/**
 * Resource group information.
 */
struct ResourceGroupInfo {
  std::string name;                     // Group name
  std::string description;              // Optional description
  std::size_t resourceCount = 0;        // Number of resources in group
  std::size_t loadedCount = 0;          // Number of currently loaded resources
  std::size_t totalSize = 0;            // Total estimated memory size
  bool isLoaded = false;                // Are all resources loaded
  bool isLoading = false;               // Is currently loading
};

/**
 * Resource group class.
 * Manages a collection of resources that can be loaded/unloaded together.
 */
class ResourceGroup {
 public:
  ResourceGroup(std::string name);
  ~ResourceGroup();
  
  // Non-copyable
  ResourceGroup(ResourceGroup const&) = delete;
  ResourceGroup& operator=(ResourceGroup const&) = delete;
  
  //==========================================================================
  // Group Management
  //==========================================================================
  
  /**
   * Get group name.
   */
  std::string const& GetName() const { return name_; }
  
  /**
   * Get group description.
   */
  std::string const& GetDescription() const { return description_; }
  
  /**
   * Set group description.
   */
  void SetDescription(std::string const& desc) { description_ = desc; }
  
  /**
   * Get number of resources in group.
   */
  std::size_t GetResourceCount() const { return resources_.size(); }
  
  /**
   * Get group info structure.
   */
  ResourceGroupInfo GetInfo() const;
  
  //==========================================================================
  // Resource Management
  //==========================================================================
  
  /**
   * Add a resource to the group.
   * @param id Resource ID
   * @return true if resource was added (false if already in group)
   */
  bool AddResource(ResourceId id);
  
  /**
   * Remove a resource from the group.
   * @param id Resource ID
   * @return true if resource was removed
   */
  bool RemoveResource(ResourceId id);
  
  /**
   * Check if resource is in the group.
   * @param id Resource ID
   * @return true if resource is in group
   */
  bool ContainsResource(ResourceId id) const;
  
  /**
   * Get all resource IDs in the group.
   * @param outResources Output vector
   */
  void GetResources(std::vector<ResourceId>& outResources) const;
  
  /**
   * Clear all resources from the group.
   * Does not unload resources.
   */
  void Clear();
  
  //==========================================================================
  // Batch Operations
  //==========================================================================
  
  /**
   * Load all resources in the group asynchronously.
   * @param manager Resource manager
   * @param priority Load priority
   * @param callback Completion callback
   * @param user_data User data for callback
   * @return true if load started
   */
  bool LoadAllAsync(IResourceManager* manager,
                    LoadPriority priority,
                    GroupLoadCompleteCallback callback,
                    void* user_data);
  
  /**
   * Unload all resources in the group.
   * @param manager Resource manager
   * @param force If true, unload even if referenced elsewhere
   * @return Number of resources unloaded
   */
  std::size_t UnloadAll(IResourceManager* manager, bool force = false);
  
  /**
   * Check if all resources are loaded.
   * @param manager Resource manager
   * @return true if all resources are loaded and ready
   */
  bool IsFullyLoaded(IResourceManager* manager) const;
  
  /**
   * Get loading progress.
   * @param manager Resource manager
   * @return Progress value 0.0 to 1.0
   */
  float GetLoadProgress(IResourceManager* manager) const;
  
  //==========================================================================
  // Dependency Management
  //==========================================================================
  
  /**
   * Add all dependencies of resources in this group.
   * @param manager Resource manager
   * @param recursive If true, also add transitive dependencies
   * @return Number of resources added
   */
  std::size_t AddAllDependencies(IResourceManager* manager, bool recursive = true);
  
  //==========================================================================
  // Memory Management
  //==========================================================================
  
  /**
   * Get estimated total memory usage.
   * @param manager Resource manager
   * @return Total memory usage in bytes
   */
  std::size_t GetTotalMemoryUsage(IResourceManager* manager) const;
  
 private:
  std::string name_;
  std::string description_;
  std::unordered_set<ResourceId> resources_;
  
  // Async load tracking
  std::size_t pendingLoads_ = 0;
  std::size_t loadSuccessCount_ = 0;
  std::size_t loadFailedCount_ = 0;
  GroupLoadCompleteCallback loadCallback_ = nullptr;
  void* loadUserData_ = nullptr;
};

/**
 * Resource group manager interface.
 * Manages named resource groups.
 */
class IResourceGroupManager {
 public:
  virtual ~IResourceGroupManager() = default;
  
  /**
   * Create a new resource group.
   * @param name Group name
   * @return Pointer to group, or nullptr if name already exists
   */
  virtual ResourceGroup* CreateGroup(std::string const& name) = 0;
  
  /**
   * Get an existing group by name.
   * @param name Group name
   * @return Pointer to group, or nullptr if not found
   */
  virtual ResourceGroup* GetGroup(std::string const& name) = 0;
  
  /**
   * Destroy a group.
   * @param name Group name
   * @param unloadResources If true, also unload group's resources
   * @return true if group was destroyed
   */
  virtual bool DestroyGroup(std::string const& name, bool unloadResources = false) = 0;
  
  /**
   * Get all group names.
   * @param outNames Output vector
   */
  virtual void GetAllGroupNames(std::vector<std::string>& outNames) const = 0;
  
  /**
   * Load all resources in a group by name.
   * @param groupName Group name
   * @param priority Load priority
   * @param callback Completion callback
   * @param user_data User data
   * @return true if load started
   */
  virtual bool LoadGroupAsync(std::string const& groupName,
                              LoadPriority priority,
                              GroupLoadCompleteCallback callback,
                              void* user_data) = 0;
  
  /**
   * Unload all resources in a group by name.
   * @param groupName Group name
   * @param force Force unload even if referenced
   * @return Number of resources unloaded
   */
  virtual std::size_t UnloadGroup(std::string const& groupName, bool force = false) = 0;
};

/**
 * Get the global resource group manager.
 */
IResourceGroupManager* GetResourceGroupManager();

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_GROUP_H
