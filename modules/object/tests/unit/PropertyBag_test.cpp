/** Unit test: property get/set and FindProperty (contract: 002-object-public-api.md US3) */

#include "te/object/PropertyBag.hpp"
#include "te/object/PropertyDescriptor.hpp"
#include "te/object/TypeId.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace te::object {

/** Concrete PropertyBag for testing. */
class TestPropertyBag : public PropertyBag {
public:
    bool GetProperty(void* outValue, char const* name) const override {
        if (!outValue || !name) return false;
        auto it = store_.find(name);
        if (it == store_.end()) return false;
        std::memcpy(outValue, it->second.data(), it->second.size());
        return true;
    }
    bool SetProperty(void const* value, char const* name) override {
        if (!name) return false;
        auto it = descs_.find(name);
        if (it == descs_.end()) return false;
        size_t n = it->second.size;
        store_[name].resize(n);
        if (value && n) std::memcpy(store_[name].data(), value, n);
        return true;
    }
    PropertyDescriptor const* FindProperty(char const* name) const override {
        if (!name) return nullptr;
        auto it = descs_.find(name);
        return it != descs_.end() ? &it->second.desc : nullptr;
    }
    void AddProperty(PropertyDescriptor const& d, size_t valueSize) {
        Entry e;
        e.desc = d;
        e.size = valueSize;
        descs_[d.name] = e;
        store_[d.name].resize(valueSize);
        if (d.defaultValue && valueSize)
            std::memcpy(store_[d.name].data(), d.defaultValue, valueSize);
    }

private:
    struct Entry { PropertyDescriptor desc; size_t size; };
    std::map<std::string, Entry> descs_;
    std::map<std::string, std::vector<char>> store_;
};

}  // namespace te::object

int main() {
    using namespace te::object;

    TestPropertyBag bag;
    int defaultVal = 42;
    PropertyDescriptor pd{"x", 1u, &defaultVal};
    bag.AddProperty(pd, sizeof(int));

    assert(bag.FindProperty("x") != nullptr);
    assert(bag.FindProperty("y") == nullptr);

    int v = 100;
    assert(bag.SetProperty(&v, "x"));
    int out = 0;
    assert(bag.GetProperty(&out, "x"));
    assert(out == 100);

    std::cout << "PropertyBag_test passed.\n";
    return 0;
}
