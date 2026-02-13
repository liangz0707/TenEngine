/**
 * @file DrawList.cpp
 * @brief Draw command list with ImGui backend (017-UICore).
 */
#include <te/uicore/DrawList.h>
#include <imgui.h>

namespace te {
namespace uicore {

class DrawCommandListImpl : public IDrawCommandList {
public:
  void DrawRect(Rect const& rect, Color const& color) override {
    ImDrawList* list = ImGui::GetWindowDrawList();
    if (!list) return;
    ImVec2 mn(rect.x, rect.y);
    ImVec2 mx(rect.x + rect.width, rect.y + rect.height);
    list->AddRectFilled(mn, mx, ImColor(color.r, color.g, color.b, color.a));
  }

  void DrawTexture(Rect const& rect, void* textureId, Rect const* uv) override {
    ImDrawList* list = ImGui::GetWindowDrawList();
    if (!list || !textureId) return;
    ImVec2 mn(rect.x, rect.y);
    ImVec2 mx(rect.x + rect.width, rect.y + rect.height);
    ImVec2 uv0(0.f, 0.f), uv1(1.f, 1.f);
    if (uv) {
      uv0.x = uv->x; uv0.y = uv->y;
      uv1.x = uv->x + uv->width; uv1.y = uv->y + uv->height;
    }
    list->AddImage(reinterpret_cast<ImTextureID>(textureId), mn, mx, uv0, uv1);
  }

  void DrawText(Rect const& rect, char const* text, void* font, Color const& color) override {
    ImDrawList* list = ImGui::GetWindowDrawList();
    if (!list || !text) return;
    ImFont* f = font ? static_cast<ImFont*>(font) : ImGui::GetFont();
    ImVec2 pos(rect.x, rect.y);
    list->AddText(f, f->FontSize, pos, ImColor(color.r, color.g, color.b, color.a), text);
  }

  void Submit() override {
    /* ImGui renders in EndFrame; no explicit submit needed. */
  }
};

IDrawCommandList* CreateDrawCommandList() {
  return new DrawCommandListImpl();
}

}  // namespace uicore
}  // namespace te
