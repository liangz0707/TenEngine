/**
 * @file ImGuiBackend.h
 * @brief ImGui Win32 + D3D11 backend.
 */
#ifndef TE_EDITOR_IMGUI_BACKEND_H
#define TE_EDITOR_IMGUI_BACKEND_H

#include <string>
#include <vector>

namespace te {
namespace editor {

/** Pass application to register ImGui WndProc handler for input. */
void ImGuiBackend_RegisterWndProcHandler(void* application);
bool ImGuiBackend_Init(void* hwnd, int width, int height);
void ImGuiBackend_Shutdown();
void ImGuiBackend_NewFrame();
void ImGuiBackend_Render();
void ImGuiBackend_Resize(int width, int height);
bool ImGuiBackend_IsInitialized();

/** Return and clear paths from OS drag-drop (WM_DROPFILES). Empty on non-Windows or when none. */
std::vector<std::string> ImGuiBackend_GetAndClearDroppedPaths();

/** Main window handle (e.g. HWND on Windows) for modal dialogs. Null if not initialized. */
void* ImGuiBackend_GetWindowHandle();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_IMGUI_BACKEND_H
