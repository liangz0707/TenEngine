/**
 * @file ResourceMetadata.h
 * @brief Resource metadata for editor display and management.
 * 
 * Provides metadata structures for resources including:
 * - Display name and description
 * - Thumbnail/preview support
 * - Tags and categories
 * - Last modified timestamp
 * - Custom properties
 */
#ifndef TE_RESOURCE_RESOURCE_METADATA_H
#define TE_RESOURCE_RESOURCE_METADATA_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <string>
#include <vector>
#include <cstdint>
#include <ctime>

namespace te {
namespace resource {

/**
 * Thumbnail data structure.
 * Stores a small preview image for the resource browser.
 */
struct ThumbnailData {
  std::vector<std::uint8_t> pixels;  // RGBA pixel data
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  bool isValid = false;
  
  void Clear() {
    pixels.clear();
    width = 0;
    height = 0;
    isValid = false;
  }
  
  std::size_t GetMemoryUsage() const {
    return pixels.size();
  }
};

/**
 * Resource metadata structure.
 * Contains all display-related information for a resource.
 */
struct ResourceMetadata {
  // Basic info
  ResourceId guid;
  std::string displayName;        // User-friendly name
  std::string description;        // Optional description
  ResourceType type = ResourceType::Custom;
  
  // Path info
  std::string assetPath;          // Virtual path in asset browser
  std::string repository;         // Repository name
  std::string sourcePath;         // Original source file path (if imported)
  
  // Timestamps
  std::time_t createdTime = 0;
  std::time_t modifiedTime = 0;
  std::time_t importedTime = 0;
  
  // Tags and categories
  std::vector<std::string> tags;
  std::vector<std::string> categories;
  
  // Thumbnail
  ThumbnailData thumbnail;
  
  // Custom properties (key-value pairs)
  std::vector<std::pair<std::string, std::string>> customProperties;
  
  // File info
  std::size_t fileSize = 0;       // Total file size on disk
  std::size_t memorySize = 0;     // Estimated memory usage when loaded
  
  // Import info
  std::string importerName;       // Name of importer used
  std::string importSettings;     // JSON-encoded import settings
  
  // State flags
  bool isDirty = false;           // Has unsaved changes
  bool isLoaded = false;          // Currently loaded in memory
  bool hasError = false;          // Has loading/import error
  std::string errorMessage;       // Error message if hasError
  
  /**
   * Get a custom property value.
   * @param key Property key
   * @param outValue Output value
   * @return true if property exists
   */
  bool GetCustomProperty(std::string const& key, std::string& outValue) const {
    for (auto const& prop : customProperties) {
      if (prop.first == key) {
        outValue = prop.second;
        return true;
      }
    }
    return false;
  }
  
  /**
   * Set a custom property value.
   * @param key Property key
   * @param value Property value
   */
  void SetCustomProperty(std::string const& key, std::string const& value) {
    for (auto& prop : customProperties) {
      if (prop.first == key) {
        prop.second = value;
        return;
      }
    }
    customProperties.push_back({key, value});
  }
  
  /**
   * Check if resource has a specific tag.
   * @param tag Tag to check
   * @return true if tag exists
   */
  bool HasTag(std::string const& tag) const {
    for (auto const& t : tags) {
      if (t == tag) return true;
    }
    return false;
  }
  
  /**
   * Add a tag if not already present.
   * @param tag Tag to add
   */
  void AddTag(std::string const& tag) {
    if (!HasTag(tag)) {
      tags.push_back(tag);
    }
  }
  
  /**
   * Remove a tag.
   * @param tag Tag to remove
   */
  void RemoveTag(std::string const& tag) {
    tags.erase(std::remove(tags.begin(), tags.end(), tag), tags.end());
  }
  
  /**
   * Get total memory usage including thumbnail.
   */
  std::size_t GetTotalMemoryUsage() const {
    return memorySize + thumbnail.GetMemoryUsage();
  }
};

/**
 * Interface for thumbnail generators.
 * Implementations create thumbnails for specific resource types.
 */
class IThumbnailGenerator {
 public:
  virtual ~IThumbnailGenerator() = default;
  
  /**
   * Check if this generator supports the given resource type.
   * @param type Resource type
   * @return true if supported
   */
  virtual bool SupportsType(ResourceType type) const = 0;
  
  /**
   * Generate thumbnail for a resource.
   * @param resourceId Resource ID
   * @param resourcePath Path to resource file
   * @param maxWidth Maximum thumbnail width
   * @param maxHeight Maximum thumbnail height
   * @param outThumbnail Output thumbnail data
   * @return true on success
   */
  virtual bool GenerateThumbnail(ResourceId resourceId,
                                  char const* resourcePath,
                                  std::uint32_t maxWidth,
                                  std::uint32_t maxHeight,
                                  ThumbnailData& outThumbnail) = 0;
  
  /**
   * Get generator priority (higher = preferred).
   */
  virtual int GetPriority() const { return 0; }
};

/**
 * Interface for resource previewers.
 * Implementations provide custom preview rendering for the editor.
 */
class IResourcePreviewer {
 public:
  virtual ~IResourcePreviewer() = default;
  
  /**
   * Check if this previewer supports the given resource type.
   * @param type Resource type
   * @return true if supported
   */
  virtual bool SupportsType(ResourceType type) const = 0;
  
  /**
   * Check if preview is available for a resource.
   * @param resourceId Resource ID
   * @return true if preview can be rendered
   */
  virtual bool IsPreviewAvailable(ResourceId resourceId) const = 0;
  
  /**
   * Get preview priority (higher = preferred).
   */
  virtual int GetPriority() const { return 0; }
};

/**
 * Thumbnail generator registry.
 * Manages thumbnail generators for different resource types.
 */
class ThumbnailGeneratorRegistry {
 public:
  static ThumbnailGeneratorRegistry& GetInstance();
  
  /**
   * Register a thumbnail generator.
   * @param generator Generator instance (ownership not transferred)
   * @param priority Registration priority
   */
  void RegisterGenerator(IThumbnailGenerator* generator, int priority = 0);
  
  /**
   * Get the best generator for a resource type.
   * @param type Resource type
   * @return Best matching generator, or nullptr
   */
  IThumbnailGenerator* GetGenerator(ResourceType type) const;
  
  /**
   * Generate thumbnail using the best available generator.
   * @param resourceId Resource ID
   * @param resourcePath Resource path
   * @param maxWidth Maximum width
   * @param maxHeight Maximum height
   * @param outThumbnail Output thumbnail
   * @return true on success
   */
  bool GenerateThumbnail(ResourceId resourceId,
                         char const* resourcePath,
                         std::uint32_t maxWidth,
                         std::uint32_t maxHeight,
                         ThumbnailData& outThumbnail);
  
 private:
  ThumbnailGeneratorRegistry() = default;
  ~ThumbnailGeneratorRegistry() = default;
  
  struct GeneratorEntry {
    IThumbnailGenerator* generator = nullptr;
    int priority = 0;
  };
  
  std::vector<GeneratorEntry> generators_;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_METADATA_H
