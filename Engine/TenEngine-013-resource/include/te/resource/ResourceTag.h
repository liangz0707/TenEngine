/**
 * @file ResourceTag.h
 * @brief Resource tagging system for categorization and querying.
 * 
 * Provides functionality for:
 * - Tagging resources with arbitrary labels
 * - Querying resources by tags
 * - Tag hierarchies and categories
 * - Tag-based filtering and search
 */
#ifndef TE_RESOURCE_RESOURCE_TAG_H
#define TE_RESOURCE_RESOURCE_TAG_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>

namespace te {
namespace resource {

/**
 * Tag identifier type.
 */
using TagId = std::size_t;

/**
 * Invalid tag ID constant.
 */
constexpr TagId InvalidTagId = 0;

/**
 * Tag category for organizing tags.
 */
struct TagCategory {
  std::string name;           // Category name (e.g., "Type", "Environment", "Gameplay")
  std::string description;    // Category description
  bool isSystemCategory;      // System-managed category (cannot be deleted)
  std::vector<TagId> tags;    // Tags in this category
  
  TagCategory() : isSystemCategory(false) {}
};

/**
 * Tag information structure.
 */
struct TagInfo {
  TagId id;                   // Unique tag ID
  std::string name;           // Tag name (e.g., "Weapon", "Tree", "Interior")
  std::string displayName;    // Display name for UI
  std::string description;    // Tag description
  std::string category;       // Category name
  TagId parentTag;            // Parent tag ID for hierarchy (InvalidTagId if none)
  std::size_t resourceCount;  // Number of resources with this tag
  bool isSystemTag;           // System-managed tag (cannot be deleted)
  
  TagInfo() : id(InvalidTagId), parentTag(InvalidTagId), 
              resourceCount(0), isSystemTag(false) {}
};

/**
 * Resource tag manager interface.
 * Manages tags and their associations with resources.
 */
class IResourceTagManager {
 public:
  virtual ~IResourceTagManager() = default;
  
  //==========================================================================
  // Tag Management
  //==========================================================================
  
  /**
   * Create a new tag.
   * @param name Tag name
   * @param category Category name (empty for default)
   * @return Tag ID, or InvalidTagId if creation failed
   */
  virtual TagId CreateTag(std::string const& name, 
                          std::string const& category = "") = 0;
  
  /**
   * Get tag ID by name.
   * @param name Tag name
   * @return Tag ID, or InvalidTagId if not found
   */
  virtual TagId GetTagId(std::string const& name) const = 0;
  
  /**
   * Get tag info by ID.
   * @param id Tag ID
   * @param outInfo Output info structure
   * @return true if tag found
   */
  virtual bool GetTagInfo(TagId id, TagInfo& outInfo) const = 0;
  
  /**
   * Get tag info by name.
   * @param name Tag name
   * @param outInfo Output info structure
   * @return true if tag found
   */
  virtual bool GetTagInfoByName(std::string const& name, TagInfo& outInfo) const = 0;
  
  /**
   * Delete a tag.
   * @param id Tag ID
   * @return true if tag was deleted
   */
  virtual bool DeleteTag(TagId id) = 0;
  
  /**
   * Rename a tag.
   * @param id Tag ID
   * @param newName New name
   * @return true if renamed successfully
   */
  virtual bool RenameTag(TagId id, std::string const& newName) = 0;
  
  /**
   * Get all tags.
   * @param outTags Output vector of tag info
   */
  virtual void GetAllTags(std::vector<TagInfo>& outTags) const = 0;
  
  /**
   * Get tags in a category.
   * @param category Category name
   * @param outTags Output vector of tag info
   */
  virtual void GetTagsByCategory(std::string const& category, 
                                  std::vector<TagInfo>& outTags) const = 0;
  
  //==========================================================================
  // Category Management
  //==========================================================================
  
  /**
   * Create a tag category.
   * @param name Category name
   * @param description Category description
   * @return true if created
   */
  virtual bool CreateCategory(std::string const& name, 
                               std::string const& description = "") = 0;
  
  /**
   * Get all categories.
   * @param outCategories Output vector
   */
  virtual void GetCategories(std::vector<TagCategory>& outCategories) const = 0;
  
  /**
   * Delete a category.
   * Tags in the category are moved to default category.
   * @param name Category name
   * @return true if deleted
   */
  virtual bool DeleteCategory(std::string const& name) = 0;
  
  //==========================================================================
  // Tag Hierarchy
  //==========================================================================
  
  /**
   * Set parent tag for hierarchy.
   * @param tagId Child tag ID
   * @param parentId Parent tag ID (InvalidTagId to remove parent)
   * @return true if successful
   */
  virtual bool SetTagParent(TagId tagId, TagId parentId) = 0;
  
  /**
   * Get child tags of a parent.
   * @param parentId Parent tag ID
   * @param outChildren Output vector of child tag IDs
   * @param recursive Include all descendants
   */
  virtual void GetChildTags(TagId parentId, 
                            std::vector<TagId>& outChildren,
                            bool recursive = false) const = 0;
  
  //==========================================================================
  // Resource Tagging
  //==========================================================================
  
  /**
   * Add a tag to a resource.
   * @param resourceId Resource ID
   * @param tagId Tag ID
   * @return true if tag was added
   */
  virtual bool AddTagToResource(ResourceId resourceId, TagId tagId) = 0;
  
  /**
   * Add a tag to a resource by tag name.
   * @param resourceId Resource ID
   * @param tagName Tag name
   * @return true if tag was added
   */
  virtual bool AddTagToResourceByName(ResourceId resourceId, 
                                       std::string const& tagName) = 0;
  
  /**
   * Remove a tag from a resource.
   * @param resourceId Resource ID
   * @param tagId Tag ID
   * @return true if tag was removed
   */
  virtual bool RemoveTagFromResource(ResourceId resourceId, TagId tagId) = 0;
  
  /**
   * Remove all tags from a resource.
   * @param resourceId Resource ID
   * @return Number of tags removed
   */
  virtual std::size_t RemoveAllTagsFromResource(ResourceId resourceId) = 0;
  
  /**
   * Get all tags on a resource.
   * @param resourceId Resource ID
   * @param outTags Output vector of tag info
   */
  virtual void GetResourceTags(ResourceId resourceId, 
                                std::vector<TagInfo>& outTags) const = 0;
  
  /**
   * Check if resource has a tag.
   * @param resourceId Resource ID
   * @param tagId Tag ID
   * @return true if resource has the tag
   */
  virtual bool ResourceHasTag(ResourceId resourceId, TagId tagId) const = 0;
  
  /**
   * Check if resource has a tag by name.
   * @param resourceId Resource ID
   * @param tagName Tag name
   * @return true if resource has the tag
   */
  virtual bool ResourceHasTagByName(ResourceId resourceId, 
                                     std::string const& tagName) const = 0;
  
  //==========================================================================
  // Resource Queries
  //==========================================================================
  
  /**
   * Get all resources with a tag.
   * @param tagId Tag ID
   * @param outResources Output vector of resource IDs
   */
  virtual void GetResourcesWithTag(TagId tagId, 
                                    std::vector<ResourceId>& outResources) const = 0;
  
  /**
   * Get all resources with all specified tags (AND query).
   * @param tagIds Vector of tag IDs
   * @param outResources Output vector of resource IDs
   */
  virtual void GetResourcesWithAllTags(std::vector<TagId> const& tagIds,
                                        std::vector<ResourceId>& outResources) const = 0;
  
  /**
   * Get all resources with any of the specified tags (OR query).
   * @param tagIds Vector of tag IDs
   * @param outResources Output vector of resource IDs
   */
  virtual void GetResourcesWithAnyTag(std::vector<TagId> const& tagIds,
                                       std::vector<ResourceId>& outResources) const = 0;
  
  /**
   * Resource query filter function type.
   * Returns true if resource passes the filter.
   */
  using ResourceFilterFunc = std::function<bool(ResourceId)>;
  
  /**
   * Query resources with advanced filtering.
   * @param requiredTags Tags that must ALL be present (AND)
   * @param anyTags Tags where at least ONE must be present (OR)
   * @param excludedTags Tags that must NOT be present
   * @param filter Optional custom filter function
   * @param outResources Output vector of resource IDs
   */
  virtual void QueryResources(std::vector<TagId> const& requiredTags,
                              std::vector<TagId> const& anyTags,
                              std::vector<TagId> const& excludedTags,
                              ResourceFilterFunc filter,
                              std::vector<ResourceId>& outResources) const = 0;
  
  //==========================================================================
  // Batch Operations
  //==========================================================================
  
  /**
   * Add tags to multiple resources.
   * @param resourceIds Resource IDs
   * @param tagId Tag ID to add
   * @return Number of resources that received the tag
   */
  virtual std::size_t AddTagToResources(std::vector<ResourceId> const& resourceIds,
                                         TagId tagId) = 0;
  
  /**
   * Remove tag from multiple resources.
   * @param resourceIds Resource IDs
   * @param tagId Tag ID to remove
   * @return Number of resources that had the tag removed
   */
  virtual std::size_t RemoveTagFromResources(std::vector<ResourceId> const& resourceIds,
                                              TagId tagId) = 0;
  
  //==========================================================================
  // Serialization
  //==========================================================================
  
  /**
   * Save tags to file.
   * @param filePath Output file path
   * @return true on success
   */
  virtual bool SaveToFile(char const* filePath) = 0;
  
  /**
   * Load tags from file.
   * @param filePath Input file path
   * @param merge If true, merge with existing tags; if false, replace
   * @return true on success
   */
  virtual bool LoadFromFile(char const* filePath, bool merge = false) = 0;
};

/**
 * Get the global resource tag manager.
 */
IResourceTagManager* GetResourceTagManager();

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_TAG_H
