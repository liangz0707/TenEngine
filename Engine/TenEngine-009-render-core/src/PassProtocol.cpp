/**
 * @file PassProtocol.cpp
 * @brief Implementation of pass resource declaration functions.
 *
 * Integrates with PipelineCore (019) RDG for resource dependency tracking.
 */

#include <te/rendercore/pass_protocol.hpp>

#include <vector>
#include <mutex>
#include <unordered_map>

namespace te::rendercore {

// Global resource declaration registry
// In production, this would be owned by a FrameGraph/RDG context

struct PassDeclarationRegistry {
  std::vector<PassResourceDecl> declarations;
  std::mutex mutex;

  void AddDeclaration(PassHandle pass, ResourceHandle resource, bool isRead, bool isWrite) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Check if declaration already exists
    for (auto& decl : declarations) {
      if (decl.pass.id == pass.id && decl.resource.id == resource.id) {
        decl.isRead = decl.isRead || isRead;
        decl.isWrite = decl.isWrite || isWrite;
        return;
      }
    }

    // Add new declaration
    PassResourceDecl decl;
    decl.pass = pass;
    decl.resource = resource;
    decl.isRead = isRead;
    decl.isWrite = isWrite;
    decl.lifetime = ResourceLifetime::Transient;
    declarations.push_back(decl);
  }

  void Clear() {
    std::lock_guard<std::mutex> lock(mutex);
    declarations.clear();
  }

  std::vector<PassResourceDecl> const& GetDeclarations() const {
    return declarations;
  }
};

static PassDeclarationRegistry s_registry;

// Internal implementation
void DeclareReadInternal(PassHandle pass, ResourceHandle resource) {
  s_registry.AddDeclaration(pass, resource, true, false);
}

void DeclareWriteInternal(PassHandle pass, ResourceHandle resource) {
  s_registry.AddDeclaration(pass, resource, false, true);
}

// Get all declarations for a pass
std::vector<PassResourceDecl> GetPassDeclarations(PassHandle pass) {
  std::vector<PassResourceDecl> result;
  for (auto const& decl : s_registry.GetDeclarations()) {
    if (decl.pass.id == pass.id) {
      result.push_back(decl);
    }
  }
  return result;
}

// Get all declarations for a resource
std::vector<PassResourceDecl> GetResourceDeclarations(ResourceHandle resource) {
  std::vector<PassResourceDecl> result;
  for (auto const& decl : s_registry.GetDeclarations()) {
    if (decl.resource.id == resource.id) {
      result.push_back(decl);
    }
  }
  return result;
}

// Clear all declarations (call at frame end)
void ClearPassDeclarations() {
  s_registry.Clear();
}

// Get total declaration count
size_t GetPassDeclarationCount() {
  return s_registry.GetDeclarations().size();
}

// Implementation that replaces the inline stubs
// These are not called directly due to inline nature, but provided
// for documentation and future non-inline implementation

void DeclareReadImpl(PassHandle pass, ResourceHandle resource) {
  DeclareReadInternal(pass, resource);
}

void DeclareWriteImpl(PassHandle pass, ResourceHandle resource) {
  DeclareWriteInternal(pass, resource);
}

}  // namespace te::rendercore
