/**
 * @file EntityAdapter.h
 * @brief IEntity adapter for editor (ABI IViewport::PickInViewport returns IEntity*).
 */
#ifndef TE_EDITOR_ENTITY_ADAPTER_H
#define TE_EDITOR_ENTITY_ADAPTER_H

namespace te {
namespace entity {
class Entity;
}  // namespace entity

namespace editor {

class IEntity {
public:
  virtual ~IEntity() = default;
  virtual te::entity::Entity* GetEntity() = 0;
  virtual te::entity::Entity const* GetEntity() const = 0;
};

IEntity* CreateEntityAdapter(te::entity::Entity* entity);

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_ENTITY_ADAPTER_H
