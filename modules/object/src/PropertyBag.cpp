/** Simple property bag implementation (contract: 002-object-public-api.md) */

#include "te/object/PropertyBag.hpp"
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace te::object {

namespace {

struct PropEntry {
    PropertyDescriptor desc;
    std::vector<char> value;  // raw bytes for valueTypeId size (simplified)
};

}

class SimplePropertyBag : public PropertyBag {
public:
    bool GetProperty(void* outValue, char const* name) const override {
        if (!outValue || !name) return false;
        auto it = props_.find(name);
        if (it == props_.end()) return false;
        std::memcpy(outValue, it->second.value.data(), it->second.value.size());
        return true;
    }

    bool SetProperty(void const* value, char const* name) override {
        if (!name) return false;
        auto it = props_.find(name);
        if (it == props_.end()) return false;
        std::memcpy(it->second.value.data(), value, it->second.value.size());
        return true;
    }

    PropertyDescriptor const* FindProperty(char const* name) const override {
        if (!name) return nullptr;
        auto it = props_.find(name);
        return it != props_.end() ? &it->second.desc : nullptr;
    }

    void AddProperty(PropertyDescriptor const& desc, size_t valueSize) {
        PropEntry e;
        e.desc = desc;
        e.value.resize(valueSize);
        if (desc.defaultValue && valueSize) std::memcpy(e.value.data(), desc.defaultValue, valueSize);
        props_[desc.name] = std::move(e);
    }

private:
    std::map<std::string, PropEntry> props_;
};

}  // namespace te::object
