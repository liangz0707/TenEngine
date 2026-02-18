/**
 * @file RenderElement.hpp
 * @brief 009-RenderCore: Concrete implementation of IRenderElement.
 *
 * SimpleRenderElement holds references to IRenderMesh and IRenderMaterial.
 */

#pragma once

#include <te/rendercore/IRenderElement.hpp>
#include <te/rendercore/IRenderMesh.hpp>
#include <te/rendercore/IRenderMaterial.hpp>

namespace te {
namespace rendercore {

/**
 * @brief Simple implementation of IRenderElement.
 *
 * Holds pointers to mesh and material. The caller is responsible for
 * ensuring the mesh and material remain valid during the element's lifetime.
 */
class SimpleRenderElement : public IRenderElement {
public:
    SimpleRenderElement() = default;
    SimpleRenderElement(IRenderMesh* mesh, IRenderMaterial* material)
        : m_mesh(mesh), m_material(material) {}

    ~SimpleRenderElement() override = default;

    // === IRenderElement interface ===

    IRenderMesh* GetMesh() override { return m_mesh; }
    IRenderMesh const* GetMesh() const override { return m_mesh; }

    IRenderMaterial* GetMaterial() override { return m_material; }
    IRenderMaterial const* GetMaterial() const override { return m_material; }

    // === Setters ===

    void SetMesh(IRenderMesh* mesh) { m_mesh = mesh; }
    void SetMaterial(IRenderMaterial* material) { m_material = material; }

private:
    IRenderMesh* m_mesh{nullptr};
    IRenderMaterial* m_material{nullptr};
};

/**
 * @brief Owning implementation of IRenderElement.
 *
 * Owns the mesh and material through smart pointers.
 * Used when the element should manage the lifetime of its resources.
 */
class OwningRenderElement : public IRenderElement {
public:
    OwningRenderElement() = default;

    ~OwningRenderElement() override = default;

    // === IRenderElement interface ===

    IRenderMesh* GetMesh() override { return m_mesh.get(); }
    IRenderMesh const* GetMesh() const override { return m_mesh.get(); }

    IRenderMaterial* GetMaterial() override { return m_material.get(); }
    IRenderMaterial const* GetMaterial() const override { return m_material.get(); }

    // === Setters ===

    void SetMesh(std::unique_ptr<IRenderMesh> mesh) { m_mesh = std::move(mesh); }
    void SetMaterial(std::unique_ptr<IRenderMaterial> material) { m_material = std::move(material); }

    // Take ownership of raw pointers (caller transfers ownership)
    void TakeMesh(IRenderMesh* mesh) { m_mesh.reset(mesh); }
    void TakeMaterial(IRenderMaterial* material) { m_material.reset(material); }

private:
    std::unique_ptr<IRenderMesh> m_mesh;
    std::unique_ptr<IRenderMaterial> m_material;
};

// === Factory functions ===

/**
 * @brief Create a simple render element with mesh and material pointers.
 * @param mesh The mesh (caller retains ownership)
 * @param material The material (caller retains ownership)
 * @return New SimpleRenderElement instance
 */
inline IRenderElement* CreateRenderElement(IRenderMesh* mesh, IRenderMaterial* material) {
    return new SimpleRenderElement(mesh, material);
}

/**
 * @brief Destroy a render element created by CreateRenderElement.
 * @param element The element to destroy
 */
inline void DestroyRenderElement(IRenderElement* element) {
    delete element;
}

}  // namespace rendercore
}  // namespace te
