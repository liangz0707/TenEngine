// 013-Resource: IResource implementation (minimal ref-counted base for loader-created resources)
#include "te/resource/Resource.h"
#include "te/resource/ResourceTypes.h"
#include <atomic>

namespace te {
namespace resource {

namespace {

class ResourceImpl : public IResource {
public:
    ResourceImpl(ResourceType type) : type_(type), ref_count_(1) {}
    ResourceType GetResourceType() const override { return type_; }
    void Release() override {
        if (ref_count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
            delete this;
    }
    void AddRef() { ref_count_.fetch_add(1, std::memory_order_relaxed); }

private:
    ResourceType type_;
    std::atomic<int> ref_count_;
};

} // namespace

// Factory for loaders; returns new ResourceImpl with ref count 1.
IResource* CreateResource(ResourceType type) {
    return new ResourceImpl(type);
}

} // namespace resource
} // namespace te
