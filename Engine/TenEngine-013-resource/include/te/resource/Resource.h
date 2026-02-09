/**
 * @file Resource.h
 * @brief IResource base class (contract: specs/_contracts/013-resource-ABI.md).
 * 
 * IResource is the base class for all loadable resources.
 * Provides Load, LoadAsync, Save, Import methods with default implementations (template method pattern)
 * and protected helper methods for file loading, GUID management, serialization coordination,
 * and dependency resolution.
 */
#ifndef TE_RESOURCE_RESOURCE_H
#define TE_RESOURCE_RESOURCE_H

#include <te/resource/ResourceTypes.h>
#include <te/resource/ResourceId.h>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <cstddef>

namespace te {
namespace resource {

// Forward declarations
class IResourceManager;
class IResource;  // Forward declaration for LoadCompleteCallback

// LoadCompleteCallback type (IResource is forward declared, full definition not needed here)
using LoadCompleteCallback = void (*)(IResource* resource, LoadResult result, void* user_data);

/**
 * Base class for all loadable resources.
 * 
 * All resource types (Mesh, Texture, Material, Model, etc.) inherit from IResource
 * and can override Load, LoadAsync, Save, Import methods or use the default implementations.
 * 
 * Template Method Pattern:
 * - Base class provides complete default implementations for Load/Save/Import
 * - Subclasses can override entire methods or just specific steps (OnLoadComplete, OnPrepareSave, etc.)
 * - Import requires subclasses to implement OnConvertSourceFile and OnCreateAssetDesc
 */
class IResource {
 public:
  virtual ~IResource() = default;

  // Pure virtual functions (must be implemented by subclasses)
  
  /** Get resource type. */
  virtual ResourceType GetResourceType() const = 0;

  /** Get resource GUID/ResourceId. */
  virtual ResourceId GetResourceId() const = 0;

  /** Decrement refcount; when zero, resource may be reclaimed. Call once per acquisition. */
  virtual void Release() = 0;

  // Virtual functions with default implementations (template method pattern)

  /**
   * Load resource synchronously from path.
   * 
   * Default implementation:
   * 1. LoadAssetDesc (read .desc file)
   * 2. LoadDataFile (read .data file)
   * 3. LoadDependencies (load dependencies, synchronous mode)
   * 4. OnLoadComplete() (subclass can override for resource-specific initialization)
   * 
   * @param path Resource file path (AssetDesc file path, e.g., "resource.mesh")
   * @param manager ResourceManager for dependency loading and cache access
   * @return true on success, false on failure
   */
  virtual bool Load(char const* path, IResourceManager* manager);

  /**
   * Load resource asynchronously from path.
   * 
   * Default implementation: Uses 001-Core's IThreadPool to execute Load logic in background thread.
   * 
   * @param path Resource file path (AssetDesc file path)
   * @param manager ResourceManager for dependency loading and cache access
   * @param on_done Completion callback
   * @param user_data User data for callback
   * @return true if async load started successfully, false on failure
   */
  virtual bool LoadAsync(char const* path, IResourceManager* manager,
                         LoadCompleteCallback on_done, void* user_data);

  /**
   * Save resource to path.
   * 
   * Default implementation:
   * 1. OnPrepareSave() (subclass can override, prepare save data)
   * 2. GenerateGUID (if needed)
   * 3. SaveAssetDesc (serialize and save AssetDesc)
   * 4. SaveDataFile (save data file)
   * 
   * @param path Output file path (AssetDesc file path, e.g., "resource.mesh")
   * @param manager ResourceManager for GUID mapping and dependency tracking
   * @return true on success, false on failure
   */
  virtual bool Save(char const* path, IResourceManager* manager);

  /**
   * Import resource from source file.
   * 
   * Default implementation:
   * 1. DetectFormat (detect source file format)
   * 2. OnConvertSourceFile() (subclass must implement, convert source file)
   * 3. OnCreateAssetDesc() (subclass must implement, generate AssetDesc)
   * 4. GenerateGUID
   * 5. SaveAssetDesc
   * 6. SaveDataFile
   * 
   * @param sourcePath Source file path (e.g., PNG, OBJ, FBX)
   * @param manager ResourceManager for dependency tracking and GUID mapping
   * @return true on success, false on failure
   */
  virtual bool Import(char const* sourcePath, IResourceManager* manager);

  /**
   * Forward to implementation; 013 does not call 008-RHI or create DResource.
   * Subclasses call 008-RHI (or via 028/011/012) to create GPU resources.
   */
  virtual void EnsureDeviceResources() {}
  
  virtual void EnsureDeviceResourcesAsync(void (*on_done)(void*), void* user_data) {
    (void)on_done;
    (void)user_data;
  }

 protected:
  // Protected helper methods (tools for subclasses)

  /**
   * Template method: Read AssetDesc file and deserialize via 002-Object.
   * Called by Load() implementation.
   * 
   * @tparam T AssetDesc type (e.g., MeshAssetDesc)
   * @param path AssetDesc file path (e.g., "resource.mesh")
   * @return std::unique_ptr<T> on success, nullptr on failure
   */
  template<typename T>
  std::unique_ptr<T> LoadAssetDesc(char const* path);

  /**
   * Template method: Serialize AssetDesc via 002-Object and write to file.
   * Called by Save() implementation.
   * 
   * @tparam T AssetDesc type
   * @param path Output file path (e.g., "resource.mesh")
   * @param desc AssetDesc pointer
   * @return true on success
   */
  template<typename T>
  bool SaveAssetDesc(char const* path, T const* desc);

  /**
   * Read data file (binary data, separate from AssetDesc).
   * Called by Load() implementation.
   * 
   * @param path Data file path (e.g., "resource.mesh.data")
   * @param outData Output buffer (allocated via 001-Core, caller must free)
   * @param outSize Output size
   * @return true on success
   */
  bool LoadDataFile(char const* path, void** outData, std::size_t* outSize);

  /**
   * Write data file (binary data, separate from AssetDesc).
   * Called by Save() implementation.
   * 
   * @param path Output file path (e.g., "resource.mesh.data")
   * @param data Data buffer
   * @param size Data size
   * @return true on success
   */
  bool SaveDataFile(char const* path, void const* data, std::size_t size);

  /**
   * Load single dependency resource (synchronous, recursive).
   * Called by LoadDependencies or directly.
   * 
   * @param guid Dependency GUID
   * @param manager ResourceManager for loading
   * @return IResource* of dependency, or nullptr on failure
   */
  IResource* LoadDependency(ResourceId guid, IResourceManager* manager);

  /**
   * Template method: Load dependencies from AssetDesc.
   * Extracts dependency list using getDeps function object and loads them.
   * Automatically selects sync/async mode based on context (sync in Load, async in LoadAsync).
   * 
   * @tparam T AssetDesc type
   * @tparam GetDepsFn Function object type: std::vector<ResourceId> (*)(T const*)
   * @param desc AssetDesc pointer
   * @param getDeps Function object to extract dependency GUID list from AssetDesc
   * @param manager ResourceManager for loading
   * @return true on success
   */
  template<typename T, typename GetDepsFn>
  bool LoadDependencies(T const* desc, GetDepsFn getDeps, IResourceManager* manager);

  /**
   * Generate GUID for resource.
   * Called by Save()/Import() implementation.
   * 
   * @return Generated ResourceId/GUID
   */
  ResourceId GenerateGUID();

  /**
   * Detect source file format (by file extension).
   * Called by Import() implementation.
   * 
   * @param sourcePath Source file path
   * @return File extension (e.g., ".png", ".obj")
   */
  std::string DetectFormat(char const* sourcePath);

  /**
   * Get AssetDesc file path from resource path.
   * Adds appropriate extension based on resource type.
   * 
   * @param path Resource path (without extension)
   * @return AssetDesc file path (e.g., "resource.mesh")
   */
  std::string GetDescPath(char const* path) const;

  /**
   * Get data file path from resource path.
   * Adds .data extension.
   * 
   * @param path Resource path or AssetDesc path
   * @return Data file path (e.g., "resource.mesh.data")
   */
  std::string GetDataPath(char const* path) const;

  // Protected virtual functions (subclasses can override)

  /**
   * Called after Load completes successfully.
   * Subclasses can override to perform resource-specific initialization.
   * Default implementation is empty.
   */
  virtual void OnLoadComplete() {}

  /**
   * Called before Save to prepare data.
   * Subclasses can override to prepare save data.
   * Default implementation is empty.
   */
  virtual void OnPrepareSave() {}

  /**
   * Convert source file to engine format.
   * Subclasses MUST implement this pure virtual function.
   * 
   * @param sourcePath Source file path
   * @param outData Output buffer (allocated by subclass, caller manages)
   * @param outSize Output size
   * @return true on success
   */
  virtual bool OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) = 0;

  /**
   * Create AssetDesc instance.
   * Subclasses MUST implement this pure virtual function.
   * 
   * @return AssetDesc pointer (allocated via 002-Object, caller manages)
   */
  virtual void* OnCreateAssetDesc() = 0;

 private:
  // Internal state
  bool m_isLoadingAsync = false;  // Track if currently loading asynchronously
};

}  // namespace resource
}  // namespace te

// Template implementations must be in header for template instantiation
#include <te/resource/Resource.inl>

#endif  // TE_RESOURCE_RESOURCE_H
