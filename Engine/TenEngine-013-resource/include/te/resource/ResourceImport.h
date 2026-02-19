/**
 * @file ResourceImport.h
 * @brief Enhanced import system with presets and batch operations.
 * 
 * Provides functionality for:
 * - Import presets for consistent asset processing
 * - Batch import operations
 * - Import post-processing pipeline
 * - Version migration support
 */
#ifndef TE_RESOURCE_RESOURCE_IMPORT_H
#define TE_RESOURCE_RESOURCE_IMPORT_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace te {
namespace resource {

class IResourceManager;
class IResource;

/**
 * Import result for a single resource.
 */
struct ImportResult {
  bool success = false;
  ResourceId resourceId;
  std::string outputPath;       // Generated output path
  std::string errorMessage;
  std::vector<ResourceId> generatedDependencies;  // Generated dependency IDs
  
  ImportResult() : resourceId() {}
};

/**
 * Batch import result.
 */
struct BatchImportResult {
  std::size_t totalCount = 0;
  std::size_t successCount = 0;
  std::size_t failedCount = 0;
  std::vector<ImportResult> results;
};

/**
 * Import preset base class.
 * Defines default settings for importing specific resource types.
 */
class ImportPreset {
 public:
  virtual ~ImportPreset() = default;
  
  /**
   * Get preset name.
   */
  virtual std::string GetName() const = 0;
  
  /**
   * Get preset description.
   */
  virtual std::string GetDescription() const { return ""; }
  
  /**
   * Get supported resource type.
   */
  virtual ResourceType GetResourceType() const = 0;
  
  /**
   * Get supported source file extensions.
   * @param outExtensions Output vector (e.g., {".png", ".jpg", ".tga"})
   */
  virtual void GetSupportedExtensions(std::vector<std::string>& outExtensions) const = 0;
  
  /**
   * Check if this preset can import the given file.
   * @param sourcePath Source file path
   * @return true if this preset supports the file
   */
  virtual bool CanImport(std::string const& sourcePath) const;
  
  /**
   * Serialize preset settings to string (for storage).
   */
  virtual std::string Serialize() const { return ""; }
  
  /**
   * Deserialize preset settings from string.
   */
  virtual bool Deserialize(std::string const& data) { 
    (void)data;
    return true; 
  }
  
  /**
   * Clone this preset.
   */
  virtual std::unique_ptr<ImportPreset> Clone() const = 0;
};

/**
 * Import preset for textures.
 */
class TextureImportPreset : public ImportPreset {
 public:
  std::string GetName() const override { return "Texture"; }
  ResourceType GetResourceType() const override { return ResourceType::Texture; }
  
  void GetSupportedExtensions(std::vector<std::string>& outExtensions) const override {
    outExtensions = {".png", ".jpg", ".jpeg", ".tga", ".bmp", ".hdr", ".exr"};
  }
  
  std::unique_ptr<ImportPreset> Clone() const override {
    return std::make_unique<TextureImportPreset>(*this);
  }
  
  // Texture-specific settings
  bool generateMipmaps = true;
  bool sRGB = true;
  bool compress = true;
  std::size_t maxSize = 4096;
  std::size_t preferredFormat = 0;  // Format enum
};

/**
 * Import preset for meshes.
 */
class MeshImportPreset : public ImportPreset {
 public:
  std::string GetName() const override { return "Mesh"; }
  ResourceType GetResourceType() const override { return ResourceType::Mesh; }
  
  void GetSupportedExtensions(std::vector<std::string>& outExtensions) const override {
    outExtensions = {".obj", ".fbx", ".gltf", ".glb"};
  }
  
  std::unique_ptr<ImportPreset> Clone() const override {
    return std::make_unique<MeshImportPreset>(*this);
  }
  
  // Mesh-specific settings
  bool generateNormals = true;
  bool generateTangents = true;
  bool optimizeMesh = true;
  float scale = 1.0f;
  bool splitByMaterial = false;
};

/**
 * Import configuration for a single import operation.
 */
struct ImportConfig {
  std::string sourcePath;            // Source file path
  std::string outputPath;            // Output directory (optional)
  std::string repository;            // Target repository
  std::string assetPath;             // Asset path in repository
  ResourceType type = ResourceType::Custom;
  ImportPreset* preset = nullptr;    // Import preset (optional)
  bool overwrite = false;            // Overwrite existing
  bool generateDependencies = true;  // Generate dependency resources
  void* userData = nullptr;          // User data
};

/**
 * Import callback type.
 */
using ImportCompleteCallback = void (*)(ImportResult const& result, void* user_data);

/**
 * Batch import callback type.
 */
using BatchImportCompleteCallback = void (*)(BatchImportResult const& result, void* user_data);

/**
 * Post-processor interface.
 * Called after import to perform additional processing.
 */
class IPostProcessor {
 public:
  virtual ~IPostProcessor() = default;
  
  /**
   * Get processor name.
   */
  virtual std::string GetName() const = 0;
  
  /**
   * Get processor priority (higher = runs later).
   */
  virtual int GetPriority() const { return 0; }
  
  /**
   * Check if this processor applies to the resource type.
   */
  virtual bool AppliesTo(ResourceType type) const = 0;
  
  /**
   * Process the imported resource.
   * @param result Import result
   * @param manager Resource manager
   * @param config Import configuration
   * @return true on success
   */
  virtual bool Process(ImportResult& result, 
                       IResourceManager* manager,
                       ImportConfig const& config) = 0;
};

/**
 * Import manager interface.
 */
class IImportManager {
 public:
  virtual ~IImportManager() = default;
  
  //==========================================================================
  // Preset Management
  //==========================================================================
  
  /**
   * Register an import preset.
   * @param preset Preset instance (ownership transferred)
   */
  virtual void RegisterPreset(std::unique_ptr<ImportPreset> preset) = 0;
  
  /**
   * Get preset by name.
   * @param name Preset name
   * @return Preset pointer (ownership not transferred)
   */
  virtual ImportPreset* GetPreset(std::string const& name) = 0;
  
  /**
   * Get preset for a source file.
   * @param sourcePath Source file path
   * @param type Resource type
   * @return Best matching preset, or nullptr
   */
  virtual ImportPreset* GetPresetForFile(std::string const& sourcePath,
                                          ResourceType type) = 0;
  
  /**
   * Get all presets for a resource type.
   * @param type Resource type
   * @param outPresets Output vector of preset pointers
   */
  virtual void GetPresetsForType(ResourceType type,
                                  std::vector<ImportPreset*>& outPresets) = 0;
  
  //==========================================================================
  // Post-Processor Management
  //==========================================================================
  
  /**
   * Register a post-processor.
   * @param processor Processor instance (ownership transferred)
   */
  virtual void RegisterPostProcessor(std::unique_ptr<IPostProcessor> processor) = 0;
  
  /**
   * Get all post-processors for a resource type.
   * @param type Resource type
   * @param outProcessors Output vector
   */
  virtual void GetPostProcessorsForType(ResourceType type,
                                         std::vector<IPostProcessor*>& outProcessors) = 0;
  
  //==========================================================================
  // Import Operations
  //==========================================================================
  
  /**
   * Import a single resource synchronously.
   * @param manager Resource manager
   * @param config Import configuration
   * @return Import result
   */
  virtual ImportResult ImportSync(IResourceManager* manager,
                                   ImportConfig const& config) = 0;
  
  /**
   * Import a single resource asynchronously.
   * @param manager Resource manager
   * @param config Import configuration
   * @param callback Completion callback
   * @param user_data User data
   */
  virtual void ImportAsync(IResourceManager* manager,
                           ImportConfig const& config,
                           ImportCompleteCallback callback,
                           void* user_data) = 0;
  
  /**
   * Import multiple resources synchronously.
   * @param manager Resource manager
   * @param configs Import configurations
   * @return Batch import result
   */
  virtual BatchImportResult ImportBatchSync(IResourceManager* manager,
                                             std::vector<ImportConfig> const& configs) = 0;
  
  /**
   * Import multiple resources asynchronously.
   * @param manager Resource manager
   * @param configs Import configurations
   * @param callback Completion callback
   * @param user_data User data
   */
  virtual void ImportBatchAsync(IResourceManager* manager,
                                std::vector<ImportConfig> const& configs,
                                BatchImportCompleteCallback callback,
                                void* user_data) = 0;
  
  /**
   * Import all files in a directory.
   * @param manager Resource manager
   * @param directoryPath Directory path
   * @param recursive Include subdirectories
   * @param repository Target repository
   * @param assetPath Base asset path
   * @param overwrite Overwrite existing
   * @return Batch import result
   */
  virtual BatchImportResult ImportDirectorySync(IResourceManager* manager,
                                                 std::string const& directoryPath,
                                                 bool recursive,
                                                 std::string const& repository,
                                                 std::string const& assetPath,
                                                 bool overwrite = false) = 0;
  
  //==========================================================================
  // Re-import Support
  //==========================================================================
  
  /**
   * Check if a resource can be re-imported.
   * @param manager Resource manager
   * @param resourceId Resource ID
   * @return true if re-import is possible
   */
  virtual bool CanReimport(IResourceManager* manager, ResourceId resourceId) = 0;
  
  /**
   * Re-import a resource from its source file.
   * @param manager Resource manager
   * @param resourceId Resource ID
   * @param force Force re-import even if source unchanged
   * @return Import result
   */
  virtual ImportResult ReimportSync(IResourceManager* manager,
                                     ResourceId resourceId,
                                     bool force = false) = 0;
  
  //==========================================================================
  // Utility
  //==========================================================================
  
  /**
   * Detect resource type from file extension.
   * @param filePath File path
   * @return Resource type, or Custom if unknown
   */
  virtual ResourceType DetectType(std::string const& filePath) = 0;
  
  /**
   * Get import settings stored for a resource.
   * @param resourceId Resource ID
   * @param outSettings Output settings string (serialized preset)
   * @return true if settings found
   */
  virtual bool GetStoredImportSettings(ResourceId resourceId,
                                        std::string& outSettings) = 0;
};

/**
 * Get the global import manager.
 */
IImportManager* GetImportManager();

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_IMPORT_H
