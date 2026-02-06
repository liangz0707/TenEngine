/**
 * 013-Resource ABI contract test: include all public headers, link te_resource.
 * Verifies symbols from specs/_contracts/013-resource-ABI.md exist.
 */
#include "te/resource/ResourceTypes.h"
#include "te/resource/ResourceId.h"
#include "te/resource/FResource.h"
#include "te/resource/RResource.h"
#include "te/resource/DResource.h"
#include "te/resource/Resource.h"
#include "te/resource/ResourceLoader.h"
#include "te/resource/ResourceSerializer.h"
#include "te/resource/ResourceImporter.h"
#include "te/resource/ResourceManager.h"
#include "te/resource/TextureResource.h"
#include "te/resource/MeshResource.h"
#include "te/resource/MaterialResource.h"
#include "te/resource/EffectResource.h"
#include "te/resource/TerrainResource.h"
#include <cassert>

using namespace te::resource;

int main() {
    IResourceManager* mgr = GetResourceManager();
    assert(mgr != nullptr);

    ResourceId id{};
    (void)id;
    ResourceType t = ResourceType::Texture;
    (void)t;
    LoadRequestId req = mgr->RequestLoadAsync("", ResourceType::Custom, nullptr, nullptr);
    (void)req;
    (void)mgr->GetLoadStatus(0);
    (void)mgr->GetLoadProgress(0);
    mgr->CancelLoad(0);
    (void)mgr->LoadSync("", ResourceType::Custom);
    (void)mgr->GetCached(id);
    (void)mgr->ResolvePath(id);
    char path_buf[256];
    (void)mgr->ResolvePathCopy(id, path_buf, sizeof(path_buf));
    mgr->SetLoadCompleteDispatcher(nullptr);
    (void)mgr->RequestStreaming(id, 0);
    mgr->SetStreamingPriority(0, 0);
    return 0;
}
