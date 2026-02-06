/**
 * Unit test: PropertyBag GetProperty, SetProperty, FindProperty.
 * Uses a minimal concrete PropertyBag implementation for testing.
 */

#include <te/object/PropertyBag.hpp>
#include <te/object/PropertyDescriptor.hpp>
#include <te/object/TypeDescriptor.hpp>
#include <te/core/engine.h>
#include <cassert>
#include <cstring>

namespace {

static te::object::PropertyDescriptor s_props[2] = {
    {"x", te::object::kInvalidTypeId, nullptr},
    {"y", te::object::kInvalidTypeId, nullptr},
};

struct TwoInts {
  int x = 0;
  int y = 0;
};

class TestPropertyBag : public te::object::PropertyBag {
 public:
  explicit TestPropertyBag(TwoInts* data) : data_(data) {}
  bool GetProperty(void* outValue, char const* name) const override {
    if (!data_ || !outValue || !name) return false;
    if (strcmp(name, "x") == 0) {
      *static_cast<int*>(outValue) = data_->x;
      return true;
    }
    if (strcmp(name, "y") == 0) {
      *static_cast<int*>(outValue) = data_->y;
      return true;
    }
    return false;
  }
  bool SetProperty(void const* value, char const* name) override {
    if (!data_ || !value || !name) return false;
    if (strcmp(name, "x") == 0) {
      data_->x = *static_cast<int const*>(value);
      return true;
    }
    if (strcmp(name, "y") == 0) {
      data_->y = *static_cast<int const*>(value);
      return true;
    }
    return false;
  }
  te::object::PropertyDescriptor const* FindProperty(char const* name) const override {
    if (!name) return nullptr;
    for (auto& p : s_props)
      if (p.name && strcmp(p.name, name) == 0) return &p;
    return nullptr;
  }

 private:
  TwoInts* data_;
};

}  // namespace

int main() {
  if (!te::core::Init(nullptr)) return 1;

  TwoInts obj{10, 20};
  TestPropertyBag bag(&obj);

  int v = 0;
  assert(bag.GetProperty(&v, "x") && v == 10);
  assert(bag.GetProperty(&v, "y") && v == 20);
  assert(!bag.GetProperty(&v, "z"));

  int set_val = 99;
  assert(bag.SetProperty(&set_val, "x"));
  assert(bag.GetProperty(&v, "x") && v == 99);
  assert(obj.x == 99);

  assert(bag.FindProperty("x") != nullptr && bag.FindProperty("x")->name && strcmp(bag.FindProperty("x")->name, "x") == 0);
  assert(bag.FindProperty("missing") == nullptr);

  te::core::Shutdown();
  return 0;
}
