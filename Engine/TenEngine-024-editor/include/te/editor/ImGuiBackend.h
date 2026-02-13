/**
 * @file ImGuiBackend.h
 * @brief ImGui Win32 + D3D11 backend.
 */
#ifndef TE_EDITOR_IMGUI_BACKEND_H
#define TE_EDITOR_IMGUI_BACKEND_H

namespace te {
namespace editor {

bool ImGuiBackend_Init(void* hwnd, int width, int height);
void ImGuiBackend_Shutdown();
void ImGuiBackend_NewFrame();
void ImGuiBackend_Render();
void ImGuiBackend_Resize(int width, int height);
bool ImGuiBackend_IsInitialized();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_IMGUI_BACKEND_H
