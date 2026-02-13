/**
 * @file Resource.cpp
 * @brief IResource implementation (contract: specs/_contracts/013-resource-ABI.md).
 * 
 * Implements default Load, LoadAsync, Save, Import methods using template method pattern.
 * Provides protected helper methods for file loading, GUID management, serialization coordination,
 * and dependency resolution.
 */

#include <te/resource/Resource.h>
#include <te/resource/ResourceManager.h>
#include <te/object/Guid.h>
#include <te/object/Serializer.h>
#include <te/core/platform.h>
#include <te/core/alloc.h>
#include <te/core/thread.h>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstddef>

namespace te {
namespace resource {

bool IResource::Load(char const* path, IResourceManager* manager) {
    if (!path || !manager) {
        return false;
    }

    // Template method pattern: default implementation
    // Note: LoadAssetDesc<T> requires AssetDesc type which is unknown in base class.
    // Subclasses should override Load() and call LoadAssetDesc<T> with their specific type,
    // along with LoadDataFile, LoadDependencies<T>, and OnLoadComplete.
    
    // Default implementation: validate inputs and call OnLoadComplete hook.
    // Subclasses should override to provide complete implementation.
    OnLoadComplete();
    return true;
}

bool IResource::LoadAsync(char const* path, IResourceManager* manager,
                         LoadCompleteCallback on_done, void* user_data) {
    if (!path || !manager) {
        if (on_done) {
            on_done(nullptr, LoadResult::Error, user_data);
        }
        return false;
    }

    te::core::IThreadPool* pool = te::core::GetThreadPool();
    te::core::ITaskExecutor* ioExecutor = pool ? pool->GetIOExecutor() : nullptr;
    if (!pool || !ioExecutor) {
        if (on_done) {
            on_done(nullptr, LoadResult::Error, user_data);
        }
        return false;
    }

    m_isLoadingAsync = true;

    struct LoadAsyncContext {
        IResource* self;
        std::string path;
        IResourceManager* manager;
        LoadCompleteCallback on_done;
        void* user_data;
        bool* isLoadingAsync;
    };
    auto context = std::make_shared<LoadAsyncContext>();
    context->self = this;
    context->path = path;
    context->manager = manager;
    context->on_done = on_done;
    context->user_data = user_data;
    context->isLoadingAsync = &m_isLoadingAsync;

    struct LoadAsyncHolder {
        std::shared_ptr<LoadAsyncContext> ctx;
        static void TaskCallback(void* user_data) {
            auto* h = static_cast<LoadAsyncHolder*>(user_data);
            if (!h->ctx) {
                delete h;
                return;
            }
            LoadAsyncContext* ctx = h->ctx.get();
            bool success = ctx->self->Load(ctx->path.c_str(), ctx->manager);

            if (ctx->on_done) {
                te::core::IThreadPool* p = te::core::GetThreadPool();
                if (p) {
                    struct CallbackContext {
                        LoadCompleteCallback callback;
                        IResource* result;
                        LoadResult load_result;
                        void* user_data;
                    };
                    auto cbCtx = std::make_shared<CallbackContext>();
                    cbCtx->callback = ctx->on_done;
                    cbCtx->result = ctx->self;
                    cbCtx->load_result = success ? LoadResult::Ok : LoadResult::Error;
                    cbCtx->user_data = ctx->user_data;

                    struct CallbackHolder {
                        std::shared_ptr<CallbackContext> ctx;
                        static void Wrapper(void* ud) {
                            auto* cbH = static_cast<CallbackHolder*>(ud);
                            if (cbH->ctx) {
                                cbH->ctx->callback(cbH->ctx->result, cbH->ctx->load_result, cbH->ctx->user_data);
                            }
                            delete cbH;
                        }
                    };
                    auto* cbHolder = new CallbackHolder{std::move(cbCtx)};
                    p->SubmitTask(CallbackHolder::Wrapper, cbHolder);
                }
            }

            *(ctx->isLoadingAsync) = false;
            delete h;
        }
    };
    auto* holder = new LoadAsyncHolder{std::move(context)};
    ioExecutor->SubmitTask(LoadAsyncHolder::TaskCallback, holder);

    return true;
}

bool IResource::Save(char const* path, IResourceManager* manager) {
    if (!path || !manager) {
        return false;
    }

    // Template method pattern: default implementation
    // Note: SaveAssetDesc<T> requires AssetDesc type which is unknown in base class.
    // Subclasses should override Save() and call SaveAssetDesc<T> with their specific type,
    // along with SaveDataFile and other save operations.
    
    // Default implementation: call OnPrepareSave hook.
    // Subclasses should override to provide complete implementation.
    OnPrepareSave();
    return true;
}

bool IResource::Import(char const* sourcePath, IResourceManager* manager) {
    if (!sourcePath || !manager) {
        return false;
    }

    // Template method pattern: default implementation
    // Step 1: DetectFormat
    std::string format = DetectFormat(sourcePath);
    if (format.empty()) {
        return false;
    }

    // Step 2: OnConvertSourceFile (pure virtual, subclass must implement)
    void* outData = nullptr;
    std::size_t outSize = 0;
    if (!OnConvertSourceFile(sourcePath, &outData, &outSize)) {
        return false;
    }

    // Step 3: OnCreateAssetDesc (pure virtual, subclass must implement)
    void* assetDesc = OnCreateAssetDesc();
    if (!assetDesc) {
        if (outData) {
            te::core::Free(outData);
        }
        return false;
    }

    // Step 4: GenerateGUID
    ResourceId guid = GenerateGUID();

    // Step 5: SaveAssetDesc (subclass-specific, requires type)
    // Step 6: SaveDataFile
    if (outData && outSize > 0) {
        // Generate output path from source path
        std::string dataPath = GetDataPath(sourcePath);
        if (!SaveDataFile(dataPath.c_str(), outData, outSize)) {
            te::core::Free(outData);
            te::core::Free(assetDesc);
            return false;
        }
    }

    // Cleanup
    if (outData) {
        te::core::Free(outData);
    }
    // Note: assetDesc memory management depends on how OnCreateAssetDesc allocates it
    // It should be managed by the subclass or freed here if allocated via te::core::Alloc

    return true;
}

bool IResource::LoadDataFile(char const* path, void** outData, std::size_t* outSize) {
    if (!path || !outData || !outSize) {
        return false;
    }

    // Read file using 001-Core
    auto fileData = te::core::FileRead(path);
    if (!fileData || fileData->empty()) {
        return false;
    }

    // Allocate buffer (caller must free via te::core::Free)
    void* data = te::core::Alloc(fileData->size(), alignof(std::max_align_t));
    if (!data) {
        return false;
    }

    std::memcpy(data, fileData->data(), fileData->size());
    *outData = data;
    *outSize = fileData->size();

    return true;
}

bool IResource::SaveDataFile(char const* path, void const* data, std::size_t size) {
    if (!path || !data || size == 0) {
        return false;
    }

    // Write file using 001-Core
    std::vector<std::uint8_t> buffer(static_cast<std::uint8_t const*>(data),
                                     static_cast<std::uint8_t const*>(data) + size);
    return te::core::FileWrite(path, buffer);
}

IResource* IResource::LoadDependency(ResourceId guid, IResourceManager* manager) {
    if (guid.IsNull() || !manager) {
        return nullptr;
    }

    // Check cache first
    IResource* cached = manager->GetCached(guid);
    if (cached) {
        return cached;
    }

    // Note: LoadDependency cannot determine ResourceType from GUID alone.
    // This method is a helper for single dependency loading when type is known.
    // Subclasses should use LoadDependencies<T> template method instead,
    // which extracts dependencies from AssetDesc and loads them with proper type information.
    // 
    // TODO: Implement LoadDependency when ResourceType can be determined from GUID
    // or when a type registry is available to map GUID to ResourceType.
    return nullptr;
}


ResourceId IResource::GenerateGUID() {
    return ResourceId::Generate();
}

std::string IResource::DetectFormat(char const* sourcePath) {
    if (!sourcePath) {
        return std::string();
    }

    return te::core::PathGetExtension(sourcePath);
}

std::string IResource::GetDescPath(char const* path) const {
    if (!path) {
        return std::string();
    }

    // Add appropriate extension based on resource type
    // Subclasses can override to provide resource-specific extension mapping
    // Default: return path as-is (subclass should provide full path with extension)
    return std::string(path);
}

std::string IResource::GetDataPath(char const* path) const {
    if (!path) {
        return std::string();
    }

    // Add .data extension
    return std::string(path) + ".data";
}

}  // namespace resource
}  // namespace te
