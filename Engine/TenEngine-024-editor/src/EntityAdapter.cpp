/**
 * @file EntityAdapter.cpp
 * @brief Entity adapter (024-Editor).
 */
#include <te/editor/EntityAdapter.h>
#include <te/entity/Entity.h>

namespace te {
namespace editor {

class EntityAdapterImpl : public IEntity {
public:
  explicit EntityAdapterImpl(te::entity::Entity* e) : m_entity(e) {}
  te::entity::Entity* GetEntity() override { return m_entity; }
  te::entity::Entity const* GetEntity() const override { return m_entity; }
private:
  te::entity::Entity* m_entity;
};

IEntity* CreateEntityAdapter(te::entity::Entity* entity) {
  return entity ? new EntityAdapterImpl(entity) : nullptr;
}

}  // namespace editor
}  // namespace te
