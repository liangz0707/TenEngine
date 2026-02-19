/**
 * @file RemoteResource.h
 * @brief Remote resource and DLC system for downloadable content.
 * 
 * Provides functionality for:
 * - Remote resource providers (CDN, servers)
 * - Download management
 * - Chunk/Patch system
 * - Offline caching
 */
#ifndef TE_RESOURCE_REMOTE_RESOURCE_H
#define TE_RESOURCE_REMOTE_RESOURCE_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <chrono>

namespace te {
namespace resource {

class IResourceManager;

/**
 * Download state enumeration.
 */
enum class DownloadState {
  NotDownloaded,    // Not downloaded yet
  Queued,           // Queued for download
  Downloading,      // Currently downloading
  Paused,           // Download paused
  Completed,        // Download completed
  Failed,           // Download failed
  Verifying,        // Verifying downloaded content
  Outdated          // Local copy is outdated
};

/**
 * Download result enumeration.
 */
enum class DownloadResult {
  Success,
  Failed,
  Cancelled,
  NetworkError,
  DiskError,
  VerificationFailed,
  AlreadyExists,
  Outdated
};

/**
 * Download progress info.
 */
struct DownloadProgress {
  std::string resourceId;       // Resource identifier (GUID or path)
  std::string url;              // Download URL
  DownloadState state = DownloadState::NotDownloaded;
  std::size_t totalBytes = 0;   // Total bytes to download
  std::size_t downloadedBytes = 0;  // Bytes downloaded
  float progress = 0.0f;        // 0.0 to 1.0
  std::size_t bytesPerSecond = 0;  // Download speed
  std::chrono::seconds estimatedTimeRemaining;
  std::string errorMessage;
};

/**
 * Download complete callback.
 */
using DownloadCompleteCallback = void (*)(std::string const& resourceId,
                                          DownloadResult result,
                                          std::string const& localPath,
                                          void* user_data);

/**
 * Download progress callback.
 */
using DownloadProgressCallback = void (*)(std::string const& resourceId,
                                          DownloadProgress const& progress,
                                          void* user_data);

/**
 * Remote resource provider interface.
 */
class IRemoteResourceProvider {
 public:
  virtual ~IRemoteResourceProvider() = default;
  
  /**
   * Get provider name.
   */
  virtual std::string GetName() const = 0;
  
  /**
   * Get base URL for resources.
   */
  virtual std::string GetBaseUrl() const = 0;
  
  /**
   * Check if provider is available.
   */
  virtual bool IsAvailable() const = 0;
  
  /**
   * Get download URL for a resource.
   * @param resourceId Resource ID
   * @param outUrl Output URL
   * @return true if URL is available
   */
  virtual bool GetResourceUrl(std::string const& resourceId, std::string& outUrl) = 0;
  
  /**
   * Get resource info.
   * @param resourceId Resource ID
   * @param outSize Output size in bytes
   * @param outHash Output hash (optional)
   * @param outVersion Output version (optional)
   * @return true if info is available
   */
  virtual bool GetResourceInfo(std::string const& resourceId,
                               std::size_t& outSize,
                               std::string& outHash,
                               std::string& outVersion) = 0;
  
  /**
   * Check if resource exists remotely.
   */
  virtual bool ResourceExists(std::string const& resourceId) = 0;
  
  /**
   * Get list of available resources.
   */
  virtual void GetAvailableResources(std::vector<std::string>& outResources) = 0;
};

/**
 * Chunk info structure.
 */
struct ChunkInfo {
  std::string chunkId;          // Chunk identifier
  std::string name;             // Display name
  std::string description;      // Description
  std::size_t totalSize = 0;    // Total size in bytes
  std::size_t downloadSize = 0; // Download size (compressed)
  std::string version;          // Version string
  std::string hash;             // Content hash
  std::vector<std::string> dependencies;  // Dependency chunk IDs
  std::vector<std::string> resources;     // Resources in this chunk
  bool isRequired = false;      // Required for base game
  bool isInstalled = false;     // Currently installed
  bool isDownloaded = false;    // Downloaded (may not be installed)
};

/**
 * DLC (Downloadable Content) info.
 */
struct DLCInfo {
  std::string dlcId;            // DLC identifier
  std::string name;             // Display name
  std::string description;      // Description
  std::string version;          // Version string
  std::vector<std::string> chunks;  // Chunks in this DLC
  std::size_t totalSize = 0;    // Total size
  bool isOwned = false;         // User owns this DLC
  bool isInstalled = false;     // Currently installed
  bool isDownloading = false;   // Currently downloading
  float downloadProgress = 0.0f; // Download progress
};

/**
 * Download manager interface.
 */
class IDownloadManager {
 public:
  virtual ~IDownloadManager() = default;
  
  //==========================================================================
  // Provider Management
  //==========================================================================
  
  /**
   * Register a remote resource provider.
   */
  virtual void RegisterProvider(std::unique_ptr<IRemoteResourceProvider> provider) = 0;
  
  /**
   * Get provider by name.
   */
  virtual IRemoteResourceProvider* GetProvider(std::string const& name) = 0;
  
  //==========================================================================
  // Download Operations
  //==========================================================================
  
  /**
   * Queue a resource for download.
   * @param resourceId Resource ID to download
   * @param localPath Local path to save to
   * @param priority Download priority
   * @param onComplete Completion callback
   * @param onProgress Progress callback (optional)
   * @param userData User data
   * @return Download handle (or nullptr on error)
   */
  virtual void* QueueDownload(std::string const& resourceId,
                              std::string const& localPath,
                              LoadPriority priority,
                              DownloadCompleteCallback onComplete,
                              DownloadProgressCallback onProgress = nullptr,
                              void* userData = nullptr) = 0;
  
  /**
   * Cancel a download.
   */
  virtual void CancelDownload(void* downloadHandle) = 0;
  
  /**
   * Pause a download.
   */
  virtual void PauseDownload(void* downloadHandle) = 0;
  
  /**
   * Resume a paused download.
   */
  virtual void ResumeDownload(void* downloadHandle) = 0;
  
  /**
   * Get download progress.
   */
  virtual bool GetDownloadProgress(void* downloadHandle, DownloadProgress& outProgress) = 0;
  
  /**
   * Get all active downloads.
   */
  virtual void GetActiveDownloads(std::vector<DownloadProgress>& outDownloads) = 0;
  
  //==========================================================================
  // Batch Operations
  //==========================================================================
  
  /**
   * Queue multiple resources for download.
   * @param resourceIds Resource IDs to download
   * @param localDir Local directory to save to
   * @param priority Download priority
   * @param onComplete Completion callback (called when all complete)
   * @param onProgress Progress callback (optional)
   * @param userData User data
   * @return Download handle for batch
   */
  virtual void* QueueBatchDownload(std::vector<std::string> const& resourceIds,
                                   std::string const& localDir,
                                   LoadPriority priority,
                                   DownloadCompleteCallback onComplete,
                                   DownloadProgressCallback onProgress = nullptr,
                                   void* userData = nullptr) = 0;
  
  //==========================================================================
  // Utility
  //==========================================================================
  
  /**
   * Get estimated download size for a resource.
   */
  virtual std::size_t GetDownloadSize(std::string const& resourceId) = 0;
  
  /**
   * Check if a resource is downloaded locally.
   */
  virtual bool IsDownloaded(std::string const& resourceId) = 0;
  
  /**
   * Get local path for a downloaded resource.
   */
  virtual bool GetLocalPath(std::string const& resourceId, std::string& outPath) = 0;
  
  /**
   * Verify a downloaded resource.
   */
  virtual bool VerifyResource(std::string const& resourceId) = 0;
  
  /**
   * Delete a downloaded resource.
   */
  virtual bool DeleteDownloadedResource(std::string const& resourceId) = 0;
};

/**
 * Chunk manager interface.
 */
class IChunkManager {
 public:
  virtual ~IChunkManager() = default;
  
  //==========================================================================
  // Chunk Management
  //==========================================================================
  
  /**
   * Get all available chunks.
   */
  virtual void GetAvailableChunks(std::vector<ChunkInfo>& outChunks) = 0;
  
  /**
   * Get installed chunks.
   */
  virtual void GetInstalledChunks(std::vector<ChunkInfo>& outChunks) = 0;
  
  /**
   * Get chunk info by ID.
   */
  virtual bool GetChunkInfo(std::string const& chunkId, ChunkInfo& outInfo) = 0;
  
  /**
   * Check if a chunk is installed.
   */
  virtual bool IsChunkInstalled(std::string const& chunkId) = 0;
  
  //==========================================================================
  // Chunk Operations
  //==========================================================================
  
  /**
   * Download and install a chunk.
   * @param chunkId Chunk ID
   * @param onComplete Completion callback
   * @param onProgress Progress callback
   * @param userData User data
   * @return Operation handle
   */
  virtual void* InstallChunk(std::string const& chunkId,
                             DownloadCompleteCallback onComplete,
                             DownloadProgressCallback onProgress = nullptr,
                             void* userData = nullptr) = 0;
  
  /**
   * Uninstall a chunk.
   */
  virtual bool UninstallChunk(std::string const& chunkId) = 0;
  
  /**
   * Update a chunk to latest version.
   */
  virtual void* UpdateChunk(std::string const& chunkId,
                            DownloadCompleteCallback onComplete,
                            DownloadProgressCallback onProgress = nullptr,
                            void* userData = nullptr) = 0;
  
  /**
   * Check for chunk updates.
   */
  virtual void CheckForUpdates(std::vector<std::string>& outUpdatedChunks) = 0;
  
  //==========================================================================
  // DLC Management
  //==========================================================================
  
  /**
   * Get all available DLCs.
   */
  virtual void GetAvailableDLCs(std::vector<DLCInfo>& outDLCs) = 0;
  
  /**
   * Get owned DLCs.
   */
  virtual void GetOwnedDLCs(std::vector<DLCInfo>& outDLCs) = 0;
  
  /**
   * Get DLC info by ID.
   */
  virtual bool GetDLCInfo(std::string const& dlcId, DLCInfo& outInfo) = 0;
  
  /**
   * Check if DLC is owned.
   */
  virtual bool IsDLCOwned(std::string const& dlcId) = 0;
  
  /**
   * Check if DLC is installed.
   */
  virtual bool IsDLCInstalled(std::string const& dlcId) = 0;
  
  /**
   * Install a DLC (all chunks).
   */
  virtual void* InstallDLC(std::string const& dlcId,
                           DownloadCompleteCallback onComplete,
                           DownloadProgressCallback onProgress = nullptr,
                           void* userData = nullptr) = 0;
  
  /**
   * Uninstall a DLC.
   */
  virtual bool UninstallDLC(std::string const& dlcId) = 0;
};

/**
 * Get the global download manager.
 */
IDownloadManager* GetDownloadManager();

/**
 * Get the global chunk manager.
 */
IChunkManager* GetChunkManager();

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_REMOTE_RESOURCE_H
